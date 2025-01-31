#include "stdafx.h"

#include "AudioCheckDllApi.h"

#include "AudioControlInterface.h"

class DllObserver final : public DeviceCollectionObserverInterface {
public:
    explicit DllObserver(TAcEventCallback eventCallback, TAcLog logCallback)
        : eventCallback_(eventCallback), logCallback_(logCallback)
    {
    }
    DISALLOW_COPY_MOVE(DllObserver);
    ~DllObserver() override;

    void OnCollectionChanged(DeviceCollectionEvent event, const std::wstring& devicePnpId) override;

    void OnTrace(const std::wstring & line) override;
    void OnTraceDebug(const std::wstring & line) override;

private:
    TAcEventCallback eventCallback_;
    TAcLog logCallback_;
};

DllObserver::~DllObserver() = default;

void DllObserver::OnCollectionChanged(DeviceCollectionEvent event, const std::wstring& devicePnpId)
{
    if(eventCallback_ != nullptr)
    {
        TAcEvent acEvent;
        switch(event)
        {
        case DeviceCollectionEvent::Discovered:
            acEvent = TAcAttachedEvent;
            break;
        case DeviceCollectionEvent::VolumeChanged:
            acEvent = TAcVolumeChangedEvent;
            break;
        case DeviceCollectionEvent::Detached:
        case DeviceCollectionEvent::None:
        default:  // NOLINT(clang-diagnostic-covered-switch-default)
            acEvent = TAcDetachedEvent;
        }
        eventCallback_(acEvent);
    }
}

void DllObserver::OnTrace(const std::wstring & line)
{
    if (logCallback_ != nullptr)
    {
        logCallback_(FALSE, line.c_str());
    }
}

void DllObserver::OnTraceDebug(const std::wstring & line)
{
    if (logCallback_ != nullptr)
    {
        logCallback_(FALSE, line.c_str());
    }
}


namespace  {
    std::unique_ptr<DeviceCollectionInterface> device_collection;
    std::unique_ptr<DeviceCollectionObserverInterface> device_collection_observer;
}

AcResult AcInitialize(AcHandle* handle, PCWSTR deviceFilter, TAcEventCallback eventCallback, TAcLog logCallback)
{
    device_collection = AudioControl::CreateDeviceCollection(deviceFilter);
    device_collection_observer = std::make_unique<DllObserver>(eventCallback, logCallback);
    device_collection->Subscribe(*device_collection_observer);

    device_collection->ResetContent();

    return 0;
}

AcResult AcGetAttached(AcHandle handle, AcDescription* description)
{
    if(description == nullptr)
    {
        return 0;
    }

    if (device_collection != nullptr && device_collection->GetSize() > 0)
    {
        const auto device = device_collection->CreateItem(0);
        wcsncpy_s(description->Guid, _countof(description->Guid), device->GetPnpId().c_str(), device->GetPnpId().size());
        wcsncpy_s(description->Name, _countof(description->Name), device->GetName().c_str(), device->GetName().size());
        description->Volume = device->GetVolume();
        return 0;
    }
    description->Guid[0] = '\0';
    description->Name[0] = L'\0';
    description->Volume = 0;
    return 0;
}

AcResult AcUnInitialize(AcHandle handle)
{
    if(device_collection != nullptr)
    {
        device_collection->Unsubscribe(*device_collection_observer);
        device_collection_observer.reset();
        device_collection.reset();
    }
    return 0;
}
