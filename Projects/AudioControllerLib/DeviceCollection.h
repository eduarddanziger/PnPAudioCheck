#pragma once

#include <endpointvolume.h>
#include <set>
#include <atlbase.h>
#include <functional>

#include "../AudioController/AudioControlInterface.h"

#include "Device.h"

#include "MultipleNotificationClient.h"


namespace ed::audio {
using EndPointVolumeSmartPtr = CComPtr<IAudioEndpointVolume>;


class DeviceCollection final : public DeviceCollectionInterface, protected MultipleNotificationClient {
protected:
    using TPnPIdToDeviceMap = std::map<std::wstring, Device>;
    using ProcessDeviceFunctionT =
        std::function<void(ed::audio::DeviceCollection*, const std::wstring&, const Device&, EndPointVolumeSmartPtr)>;

public:
    DISALLOW_COPY_MOVE(DeviceCollection);
    ~DeviceCollection() override;

public:
    explicit DeviceCollection(std::wstring nameFilter, bool bothHeadsetAndMicro);

    [[nodiscard]] size_t GetSize() const override;
    [[nodiscard]] std::unique_ptr<DeviceInterface> CreateItem(size_t deviceNumber) const override;
    void Subscribe(DeviceCollectionObserverInterface & observer) override;
    void Unsubscribe(DeviceCollectionObserverInterface & observer) override;

public:
    HRESULT OnDeviceAdded(LPCWSTR deviceId) override;
    HRESULT OnDeviceRemoved(LPCWSTR deviceId) override;
    HRESULT OnDeviceStateChanged(LPCWSTR deviceId, DWORD dwNewState) override;
    HRESULT OnNotify(PAUDIO_VOLUME_NOTIFICATION_DATA pNotify) override;

private:
    void ProcessActiveDeviceList(ProcessDeviceFunctionT processDeviceFunc);
    void RecreateActiveDeviceList();
    void RefreshVolumes();
    static void RegisterDevice(DeviceCollection* self, const std::wstring& deviceId, const Device& device, EndPointVolumeSmartPtr endpointVolume);
    static void UpdateDeviceVolume(DeviceCollection* self, const std::wstring& deviceId, const Device& device, EndPointVolumeSmartPtr);


    void NotifyObservers(DeviceCollectionEvent action, const std::wstring & devicePNpId) const;
    [[nodiscard]] bool IsDeviceApplicable(const Device & device) const;
    CComPtr<IAudioEndpointVolume> TryCreateDeviceAndGetVolumeEndpoint(ULONG i, CComPtr<IMMDevice> deviceEndpointSmartPtr,
                                                                      Device & device, std::wstring & deviceId) const;

    void TraceIt(const std::wstring & line) const;
    void TraceItDebug(const std::wstring & line) const;
    void UnregisterAllEndpointsVolumes();
    void UnregisterAndRemoveEndpointsVolumes(const std::wstring & deviceId);

    [[nodiscard]] Device MergeDeviceWithExistingOneBasedOnPnpIdAndFlow(const Device & device) const;
    [[nodiscard]] bool CheckRemovalAndUnmergeDeviceFromExistingOneBasedOnPnpIdAndFlow(const Device & device, Device & unmergedDev) const;

    EndPointVolumeSmartPtr TryCreateDeviceOnId(LPCWSTR deviceId, Device & device) const;

    static std::vector<std::wstring> GetDevicePnPIdsWithChangedVolume(const TPnPIdToDeviceMap & old,
                                                                      const TPnPIdToDeviceMap & updated);

public:
    void ResetContent() override;


private:
    std::map<std::wstring, Device> pnpToDeviceMap_;
    std::set<DeviceCollectionObserverInterface*> observers_;
    IMMDeviceEnumerator * enumerator_ = nullptr;
    std::wstring nameFilter_;
    bool bothHeadsetAndMicro_;
    const std::wstring noPlugAndPlayGuid_ = L"{00000000-0000-0000-FFFF-FFFFFFFFFFFF}";

    std::map<std::wstring, CComPtr<IAudioEndpointVolume>> devIdToEndpointVolumes_;
};
}
