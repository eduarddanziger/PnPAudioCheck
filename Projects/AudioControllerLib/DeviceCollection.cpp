// ReSharper disable CppClangTidyClangDiagnosticLanguageExtensionToken
#include "stdafx.h"

#include "DeviceCollection.h"
#include "Device.h"

#include <iostream>
#include <cstddef>
#include <mmdeviceapi.h>
#include <endpointvolume.h>
#include <Functiondiscoverykeys_devpkey.h>
#include <ranges>
#include <sstream>
#include <string>
#include <valarray>
#include <magic_enum_iostream.hpp>

#include "DefToString.h"
#include "generate-uuid.h"
#include "Utilities.h"
#include "CaseInsensitiveSubstr.h"

using namespace std::literals::string_literals;

inline DeviceFlowEnum ConvertFromLowLevelFlow(const EDataFlow flow)
{
    switch (flow)
    {
    case eRender:
        return DeviceFlowEnum::Render;
    case eCapture:
        return DeviceFlowEnum::Capture;
    case eAll:
        return DeviceFlowEnum::RenderAndCapture;
    case EDataFlow_enum_count:
    default: // NOLINT(clang-diagnostic-covered-switch-default)
        return DeviceFlowEnum::None;
    }
}


ed::audio::DeviceCollection::~DeviceCollection()
{
    UnregisterAllEndpointsVolumes();
    SAFE_RELEASE(enumerator_)
}

// ReSharper disable once CppParameterNeverUsed
ed::audio::DeviceCollection::DeviceCollection(std::wstring nameFilter, bool bothHeadsetAndMicro)
    : MultipleNotificationClient()
      , nameFilter_(std::move(nameFilter))
      , bothHeadsetAndMicro_(bothHeadsetAndMicro)
{
    auto hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), nullptr, CLSCTX_ALL, IID_PPV_ARGS(&enumerator_));
    assert(SUCCEEDED(hr));

    ResetNotification(enumerator_);
}

void ed::audio::DeviceCollection::ResetContent()
{
    RecreateActiveDeviceList();
}

size_t ed::audio::DeviceCollection::GetSize() const
{
    return pnpToDeviceMap_.size();
}

std::unique_ptr<DeviceInterface> ed::audio::DeviceCollection::CreateItem(size_t deviceNumber) const
{
    if (deviceNumber >= pnpToDeviceMap_.size())
    {
        throw std::runtime_error("Device number is too big");
    }
    size_t i = 0;
    for (const auto & recordVal : pnpToDeviceMap_ | std::views::values)
    {
        if (i++ == deviceNumber)
        {
            return std::make_unique<Device>(recordVal);
        }
    }
    throw std::runtime_error("Device number not found");
}

void ed::audio::DeviceCollection::Subscribe(DeviceCollectionObserverInterface & observer)
{
    observers_.insert(&observer);
}

void ed::audio::DeviceCollection::Unsubscribe(DeviceCollectionObserverInterface & observer)
{
    observers_.erase(&observer);
}


bool ed::audio::DeviceCollection::TryCreateDeviceAndGetVolumeEndpoint(
    ULONG i,
    CComPtr<IMMDevice> deviceEndpointSmartPtr,
    Device & device,
    std::wstring & deviceId,
    EndPointVolumeSmartPtr & outVolumeEndpoint
) const {
    HRESULT hr;
    // Get device id
    {
        LPWSTR deviceIdPtr = nullptr;
        hr = deviceEndpointSmartPtr->GetId(&deviceIdPtr);
        if (FAILED(hr)) {
            return false;
        }
        deviceId = deviceIdPtr;
        LOG_INFO(L"Id of the current point device " << i << L" is \"" << deviceId << L"\".");
        CoTaskMemFree(deviceIdPtr);
    }
    // Get flow direction via IMMEndpoint
    auto flow = DeviceFlowEnum::None;
    {
        EDataFlow lowLevelFlow;
        IMMEndpoint * pEndpoint = nullptr;
        hr = deviceEndpointSmartPtr->QueryInterface(__uuidof(IMMEndpoint), reinterpret_cast<void**>(&pEndpoint));
        if (FAILED(hr)) {
            return false;
        }
        hr = pEndpoint->GetDataFlow(&lowLevelFlow);
        SAFE_RELEASE(pEndpoint);
        if (FAILED(hr)) {
            return false;
        }
        flow = ConvertFromLowLevelFlow(lowLevelFlow);
        LOG_INFO(L"The end point device " << i << L", id \"" << deviceId << L"\", has a data flow \"" << GetFlowAsString(flow) << L"\".");
    }
    // Read device PnP Class id property
    std::wstring pnpGuid;
    std::wstring name;
    {
        IPropertyStore* pProps = nullptr;
        hr = deviceEndpointSmartPtr->OpenPropertyStore(STGM_READ, &pProps);
        if (FAILED(hr)) {
            return false;
        }
        {
            PROPVARIANT propVarForName;

            PropVariantInit(&propVarForName);

            hr = pProps->GetValue(
                PKEY_Device_FriendlyName, &propVarForName);
            assert(SUCCEEDED(hr));
            if (propVarForName.vt == VT_EMPTY)
            {
                std::wstringstream wos;
                wos << "UnknownDeviceName" << i;
                name = wos.str();
                LOG_INFO(L"End point device " << i << L", id \"" << deviceId << L"\", has no name, assigning: \"" << name << L"\".")
            }
            else
            {
                name = propVarForName.pwszVal;
                LOG_INFO(
                    L"The end point device " << i << L", id \"" << deviceId << L"\", has a name \"" << name << L"\".")
            }
            // ReSharper disable once CppFunctionResultShouldBeUsed
            PropVariantClear(&propVarForName);
        }

        {
            PROPVARIANT propVarForGuid;
            PropVariantInit(&propVarForGuid);

            hr = pProps->GetValue(
                PKEY_Device_ContainerId, &propVarForGuid);

            assert(SUCCEEDED(hr));
            assert(propVarForGuid.vt == VT_CLSID);
            {
                WCHAR buff[80];
                // ReSharper disable once CppTooWideScopeInitStatement
                const auto len = StringFromGUID2(
                    *propVarForGuid.puuid,
                    buff,
                    std::size(buff)
                );
                if (len == 0)
                {
                    buff[0] = L'\0';
                }
                pnpGuid = std::wstring(buff);
            }
            LOG_INFO(
                L"The end point device " << i << L", id \"" << deviceId << L"\", has a PnP id \"" << pnpGuid << L"\".")

                // ReSharper disable once CppFunctionResultShouldBeUsed
            PropVariantClear(&propVarForGuid);
        }
        SAFE_RELEASE(pProps);
    }
    // Get IAudioEndpointVolume and volume
    outVolumeEndpoint = nullptr;
    uint16_t volume = 0;
    {
        IAudioEndpointVolume* pEndpointVolume;
        hr = deviceEndpointSmartPtr->Activate(
            __uuidof(IAudioEndpointVolume),
            CLSCTX_INPROC_SERVER,
            nullptr,
            reinterpret_cast<void**>(&pEndpointVolume)
        );
        if (SUCCEEDED(hr)) {
            outVolumeEndpoint.Attach(pEndpointVolume);
        }
    }
    // Check mute and possibly correct volume
    if (outVolumeEndpoint == nullptr) {
        LOG_INFO(L"The end point device " << i << L", id \"" << deviceId << L"\", has no volume property.");
        return false;
    }
    BOOL mute;
    hr = outVolumeEndpoint->GetMute(&mute);
    if (FAILED(hr)) {
        return false;
    }
    if (mute == FALSE) {
        float currVolume = 0.0f;
        hr = outVolumeEndpoint->GetMasterVolumeLevelScalar(&currVolume);
        if (FAILED(hr)) {
            return false;
        }
        volume = static_cast<uint16_t>(lround(currVolume * 1000.0f));
        LOG_INFO(L"The end point device " << i << L", id \"" << deviceId << L"\", has a volume \"" << volume << L"\".");
    }
	uint16_t renderVolume = 0;
	uint16_t captureVolume = 0;

    switch (flow)
    {
    case DeviceFlowEnum::Capture:
        captureVolume = volume;
        break;
    case DeviceFlowEnum::Render:
        renderVolume = volume;
        break;
    default:
        break;
    }
	device = Device(pnpGuid, name, flow, renderVolume, captureVolume);
    return true;
}

void ed::audio::DeviceCollection::TraceIt(const std::wstring & line) const
{
    for (auto * obs : observers_)
    {
        obs->OnTrace(line);
    }
}

void ed::audio::DeviceCollection::TraceItDebug(const std::wstring & line) const
{
    for (auto * obs : observers_)
    {
        obs->OnTraceDebug(line);
    }
}

void ed::audio::DeviceCollection::UnregisterAllEndpointsVolumes()
{
    for (const auto & endpointVolume : devIdToEndpointVolumes_ | std::views::values)
    {
        // ReSharper disable once CppFunctionResultShouldBeUsed
        endpointVolume->UnregisterControlChangeNotify(this);
    }
}

// template<class INTERFACE>
// ULONG CountRef(INTERFACE* pInterface) noexcept
// {
//     if (pInterface)
//     {
//         pInterface->AddRef();
//         return pInterface->Release();
//     }
//
//     return 0;
// }

void ed::audio::DeviceCollection::UnregisterAndRemoveEndpointsVolumes(const std::wstring & deviceId)
{
    if
    (
        const auto foundPair = devIdToEndpointVolumes_.find(deviceId)
        ; foundPair != devIdToEndpointVolumes_.end()
    )
    {
        auto audioEndpointVolume = foundPair->second;
        // ReSharper disable once CppFunctionResultShouldBeUsed
        audioEndpointVolume->UnregisterControlChangeNotify(this);
        //        const auto ii = CountRef(static_cast<IAudioEndpointVolume*>(audioEndpointVolume));
        audioEndpointVolume.Detach();
        devIdToEndpointVolumes_.erase(foundPair);
    }
}

ed::audio::Device ed::audio::DeviceCollection::MergeDeviceWithExistingOneBasedOnPnpIdAndFlow(
    const ed::audio::Device & device) const
{
    if
    (
        const auto foundPair = pnpToDeviceMap_.find(device.GetPnpId())
        ; foundPair != pnpToDeviceMap_.end()
    )
    {
        auto volume = device.GetCurrentRenderVolume();
        auto flow = device.GetFlow();
        uint16_t renderVolume = device.GetCurrentRenderVolume();
		uint16_t captureVolume = device.GetCurrentCaptureVolume();

        const auto & foundDev = foundPair->second;
        if (foundDev.GetFlow() != device.GetFlow())
        {

            switch (flow)
            {
            case DeviceFlowEnum::Capture:
                renderVolume = foundDev.GetCurrentRenderVolume();
                break;
            case DeviceFlowEnum::Render:
                captureVolume = foundDev.GetCurrentCaptureVolume();
            default:
                break;
            }

            flow = DeviceFlowEnum::RenderAndCapture;
        }
        auto foundDevNameAsSet = Split(foundDev.GetName(), L'/');

        foundDevNameAsSet.insert(device.GetName());
        return {
			device.GetPnpId(), Merge(foundDevNameAsSet, L'/'), flow, renderVolume, captureVolume
        };
    }
    return device;
}

void ed::audio::DeviceCollection::ProcessActiveDeviceList(ProcessDeviceFunctionT processDeviceFunc)
{
	HRESULT hr;
	CComPtr<IMMDeviceCollection> deviceCollectionSmartPtr;
	{
		IMMDeviceCollection* deviceCollection = nullptr;
		hr = enumerator_->EnumAudioEndpoints(
			bothHeadsetAndMicro_ ? eAll : eRender, DEVICE_STATE_ACTIVE,
			&deviceCollection);
		if (FAILED(hr))
		{
			LOG_INFO("EnumAudioEndpoints failed")
				return;
		}
		LOG_INFO(L"Audio devices enumerated.\n")
			deviceCollectionSmartPtr.Attach(deviceCollection);
	}
	UINT count = 0;
	hr = deviceCollectionSmartPtr->GetCount(&count);
	assert(SUCCEEDED(hr));
	for (ULONG i = 0; i < count; i++)
	{
		Device device;
		EndPointVolumeSmartPtr endPointVolumeSmartPtr;
		bool isDeviceCreated = false;
		std::wstring deviceId;
		{
			CComPtr<IMMDevice> endpointDeviceSmartPtr;
			{
				IMMDevice* pEndpointDevice = nullptr;
				hr = deviceCollectionSmartPtr->Item(i, &pEndpointDevice);
				if (FAILED(hr))
				{
					LOG_INFO("Collection::Item failed")
						continue;
				}
				endpointDeviceSmartPtr.Attach(pEndpointDevice);
			}
			isDeviceCreated = TryCreateDeviceAndGetVolumeEndpoint(i, endpointDeviceSmartPtr, device, deviceId, endPointVolumeSmartPtr);
		}
		if (!isDeviceCreated)
		{
			continue;
		}
		if (!IsDeviceApplicable(device))
		{
			continue;
		}
		processDeviceFunc(this, deviceId, device, endPointVolumeSmartPtr);
		LOG_INFO(L"End point " << i << L" with plug-and-play id " << device.GetPnpId() << L" processed.\n")
	}
}


void ed::audio::DeviceCollection::RecreateActiveDeviceList()
{
    LOG_INFO("Recreating audio device info list..")
    pnpToDeviceMap_.clear();

    UnregisterAllEndpointsVolumes();
    devIdToEndpointVolumes_.clear();

    ProcessActiveDeviceList(&DeviceCollection::RegisterDevice);
}

void ed::audio::DeviceCollection::RefreshVolumes()
{
    LOG_INFO("Refreshing volumes of audio devices..")

    ProcessActiveDeviceList(&DeviceCollection::UpdateDeviceVolume);
}


// ReSharper disable CppPassValueParameterByConstReference
/*static*/
void ed::audio::DeviceCollection::RegisterDevice(ed::audio::DeviceCollection* self, const std::wstring& deviceId, const Device& device, EndPointVolumeSmartPtr endpointVolume)
{
    if (endpointVolume != nullptr)
    {
        // ReSharper disable once CppFunctionResultShouldBeUsed
        endpointVolume->RegisterControlChangeNotify(self);
        self->devIdToEndpointVolumes_[deviceId] = endpointVolume;
    }

    self->pnpToDeviceMap_[device.GetPnpId()] = self->MergeDeviceWithExistingOneBasedOnPnpIdAndFlow(device);
}

void ed::audio::DeviceCollection::UpdateDeviceVolume(DeviceCollection* self, const std::wstring& deviceId, const Device& device, EndPointVolumeSmartPtr)
{
    // ReSharper restore CppPassValueParameterByConstReference
    const auto pnpGuid = device.GetPnpId();
    if
    (
        auto foundPair = self->pnpToDeviceMap_.find(pnpGuid)
        ; foundPair != self->pnpToDeviceMap_.end()
    )
    {
        auto& foundDev = foundPair->second;
        if (device.GetFlow() == DeviceFlowEnum::Render)
        {
            foundDev.SetCurrentRenderVolume(device.GetCurrentRenderVolume());
        }
        else
        {
            foundDev.SetCurrentCaptureVolume(device.GetCurrentCaptureVolume());
        }
    }
}


void ed::audio::DeviceCollection::NotifyObservers(DeviceCollectionEvent action, const std::wstring & devicePNpId) const
{
    for (auto * observer : observers_)
    {
        observer->OnCollectionChanged(action, devicePNpId);
    }
}

bool ed::audio::DeviceCollection::IsDeviceApplicable(const Device & device) const
{
    using magic_enum::iostream_operators::operator<<; // out-of-the-box stream operators for enums

    if (!bothHeadsetAndMicro_ && device.GetFlow() != DeviceFlowEnum::Render)
    {
        LOG_INFO(
            L"Got a low-level event concerning the device \"" << device.GetName() << L"\" , that is in \"" << device.
            GetFlow() << L"\" mode. Ignore the event.\n")
        return false;
    }
    LOG_INFO(
        L"Got a low-level event concerning the device \"" << device.GetName() << L"\" , that is in " << device.GetFlow()
        << L" mode.\n")

    if (!FindSubstrCaseInsensitive(device.GetName(), nameFilter_))
    {
        LOG_INFO(
            L"The device name \"" << device.GetName() << L"\" does not satisfy the substring filter \"" << nameFilter_
            << L"\". Ignoring the event.\n")
        return false;
    }

    if (nameFilter_.empty())
    {
        LOG_INFO(L"No substring filter set for the device name \"" << device.GetName() << L"\".\n")
    }
    else
    {
        LOG_INFO(
            L"The device name \"" << device.GetName() << L"\" satisfy the substring filter \"" << nameFilter_ <<
            L"\".\n")
    }

    if (device.GetPnpId() == noPlugAndPlayGuid_)
    {
        LOG_INFO(L"The device \"" << device.GetName() << L"\" has no unique plug-and-play id. Ignoring the event.\n")
        return false;
    }
    LOG_INFO(
        L"The device \"" << device.GetName() << L"\" has got a plug-and-play id " << device.GetPnpId() <<
        L". Transferring the event to subscribers.\n")
    return true;
}

HRESULT ed::audio::DeviceCollection::OnDeviceAdded(LPCWSTR deviceId)
{
    using magic_enum::iostream_operators::operator<<; // out-of-the-box stream operators for enums

    const HRESULT onDeviceAdded = MultipleNotificationClient::OnDeviceAdded(deviceId);
    if (onDeviceAdded == S_OK)
    {
        LOG_INFO(L"ADDED INFO: device id \"" << deviceId << L".")

        Device device;
        if
        (
            EndPointVolumeSmartPtr endPointVolumeSmartPtr;
            TryCreateDeviceOnId(deviceId, device, endPointVolumeSmartPtr) && IsDeviceApplicable(device)
        )
        {
            LOG_INFO(
                L"ADDED MORE INFO: device name: \"" << device.GetName() << L"\", flow: " << device.GetFlow()
                << L", plug-and-play id " << device.GetPnpId() << L".")

            const auto possiblyMergedDevice = MergeDeviceWithExistingOneBasedOnPnpIdAndFlow(device);
            LOG_INFO(
                L"ADDED MERGED: device name: \"" << possiblyMergedDevice.GetName() << L"\", flow: " <<
                possiblyMergedDevice.GetFlow() << L".")

            pnpToDeviceMap_[device.GetPnpId()] = possiblyMergedDevice;

            // ReSharper disable once CppFunctionResultShouldBeUsed
            if (endPointVolumeSmartPtr != nullptr)
            {
                endPointVolumeSmartPtr->RegisterControlChangeNotify(this);
                devIdToEndpointVolumes_[deviceId] = endPointVolumeSmartPtr;
            }

            NotifyObservers(DeviceCollectionEvent::Discovered, device.GetPnpId());
        }
        LOG_INFO(L"ADDED FINISHED: device id \"" << deviceId << L".\n")
    }
    return onDeviceAdded;
}

bool ed::audio::DeviceCollection::CheckRemovalAndUnmergeDeviceFromExistingOneBasedOnPnpIdAndFlow(
    const Device & device, Device & unmergedDev) const
{
    unmergedDev = {
		device.GetPnpId(), device.GetName(), DeviceFlowEnum::None, device.GetCurrentRenderVolume(), device.GetCurrentCaptureVolume()
    };

    if
    (
        const auto foundPair = pnpToDeviceMap_.find(device.GetPnpId())
        ; foundPair != pnpToDeviceMap_.end()
    )
    {
        const auto volume = device.GetCurrentRenderVolume();
        auto flow = device.GetFlow();
        auto name = device.GetName();

        const auto & foundDev = foundPair->second;
        if
        (
            foundDev.GetFlow() == flow
        )
        {
            return true;
        }

        if
        (
            foundDev.GetFlow() == DeviceFlowEnum::RenderAndCapture
        )
        {
            uint16_t renderVolume = foundDev.GetCurrentRenderVolume();
			uint16_t captureVolume = foundDev.GetCurrentCaptureVolume();

            switch (flow)
            {
            case DeviceFlowEnum::Capture:
                flow = DeviceFlowEnum::Render;
                captureVolume = 0;
                break;
            case DeviceFlowEnum::Render:
                flow = DeviceFlowEnum::Capture;
                renderVolume = 0;
                break;
            default:
                break;
            }

            // ReSharper disable once CppTooWideScopeInitStatement
            const auto foundDevNameAsSet = Split(foundDev.GetName(), L'/');
            for (const auto & elem : foundDevNameAsSet)
            {
                if (elem != name)
                {
                    name = elem;
                    break;
                }
            }
			unmergedDev = { device.GetPnpId(), name, flow, renderVolume, captureVolume };
            return true;
        }
    }
    return false;
}


HRESULT ed::audio::DeviceCollection::OnDeviceRemoved(LPCWSTR deviceId)
{
    using magic_enum::iostream_operators::operator<<; // out-of-the-box stream operators for enums

    const HRESULT hr = MultipleNotificationClient::OnDeviceRemoved(deviceId);
    if (hr == S_OK)
    {
        LOG_INFO(L"REMOVED INFO: device id \"" << deviceId << L".")
        Device removedDeviceToUnmerge;
        if
        (   EndPointVolumeSmartPtr volumeEndpointSmartPtr;
            TryCreateDeviceOnId(deviceId, removedDeviceToUnmerge, volumeEndpointSmartPtr)
            && IsDeviceApplicable(removedDeviceToUnmerge)
        )
        {
            LOG_INFO(
                L"REMOVED MORE INFO: device name \"" << removedDeviceToUnmerge.GetName() << L"\", flow: " <<
                removedDeviceToUnmerge.GetFlow() << L", plug-and-play id: " << removedDeviceToUnmerge.GetPnpId() <<
                L".")

            
            if (Device possiblyUnmergedDevice; 
                CheckRemovalAndUnmergeDeviceFromExistingOneBasedOnPnpIdAndFlow(removedDeviceToUnmerge, possiblyUnmergedDevice))
            {
                if (possiblyUnmergedDevice.GetFlow() == DeviceFlowEnum::None)
                {
                    LOG_INFO(L"REMOVED UNMERGED: nothing.")
                    pnpToDeviceMap_.erase(possiblyUnmergedDevice.GetPnpId());
                }
                else
                {
                    LOG_INFO(
                        L"REMOVED UNMERGED: device name \"" << possiblyUnmergedDevice.GetName() << L"\", flow: " <<
                        possiblyUnmergedDevice.GetFlow() << L".")
                    pnpToDeviceMap_[possiblyUnmergedDevice.GetPnpId()] = possiblyUnmergedDevice;
                }
                UnregisterAndRemoveEndpointsVolumes(deviceId);
                NotifyObservers(DeviceCollectionEvent::Detached, removedDeviceToUnmerge.GetPnpId());
            }
        }
        LOG_INFO(L"REMOVED FINISHED: device id \"" << deviceId << L".\n")
    }
    return hr;
}

bool ed::audio::DeviceCollection::TryCreateDeviceOnId(
    LPCWSTR deviceId,
    Device& device,
    EndPointVolumeSmartPtr& outVolumeEndpoint
) const {
    CComPtr<IMMDevice> deviceSmartPtr;
    // Retrieve the device using the device ID
    {
        IMMDevice* devicePtr = nullptr;
        const auto hr = enumerator_->GetDevice(deviceId, &devicePtr);
        if (FAILED(hr)) {
            return false; // Return false on failure
        }
        deviceSmartPtr.Attach(devicePtr);
    }
    std::wstring devId;
    return TryCreateDeviceAndGetVolumeEndpoint(0, deviceSmartPtr, device, devId, outVolumeEndpoint);
}

std::vector<std::wstring> ed::audio::DeviceCollection::GetDevicePnPIdsWithChangedVolume(
    const TPnPIdToDeviceMap & old, const TPnPIdToDeviceMap & updated)
{
    std::vector<std::wstring> diff;
    for (const auto & [fst, snd] : old)
    {
        const auto oldPnPId = fst;
        if (auto foundPair = updated.find(oldPnPId); foundPair != updated.end())
        {
            auto oldVolume = snd.GetCurrentRenderVolume();
            auto newVolume = foundPair->second.GetCurrentRenderVolume();
            if (oldVolume != newVolume)
            {
                diff.push_back(oldPnPId);
                continue;
            }
			oldVolume = snd.GetCurrentCaptureVolume();
            newVolume = foundPair->second.GetCurrentCaptureVolume();
            if (oldVolume != newVolume)
            {
                diff.push_back(oldPnPId);
            }
        }
    }
    return diff;
}

HRESULT ed::audio::DeviceCollection::OnDeviceStateChanged(LPCWSTR deviceId, DWORD dwNewState)
{
    HRESULT hr = MultipleNotificationClient::OnDeviceStateChanged(deviceId, dwNewState);
    assert(SUCCEEDED(hr));

    switch (dwNewState)
    {
    case DEVICE_STATE_ACTIVE:
        hr = OnDeviceAdded(deviceId);
        break;
    case DEVICE_STATE_DISABLED:
    case DEVICE_STATE_NOTPRESENT:
    case DEVICE_STATE_UNPLUGGED:
        hr = OnDeviceRemoved(deviceId);
        break;
    default: ;
    }

    return hr;
}

HRESULT ed::audio::DeviceCollection::OnNotify(PAUDIO_VOLUME_NOTIFICATION_DATA pNotify)
{
    const HRESULT hResult = MultipleNotificationClient::OnNotify(pNotify);
    const auto copy = pnpToDeviceMap_;

    RefreshVolumes();

    for (
        const auto diff = GetDevicePnPIdsWithChangedVolume(copy, pnpToDeviceMap_);
        const auto & currPnPId : diff)
    {
        NotifyObservers(DeviceCollectionEvent::VolumeChanged, currPnPId);
    }

    return hResult;
}
