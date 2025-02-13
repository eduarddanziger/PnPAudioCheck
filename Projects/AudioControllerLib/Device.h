#pragma once

#include "../AudioController/AudioControlInterface.h"

namespace ed::audio {
class Device final : public DeviceInterface {
public:
    ~Device() override;

public:
    Device();
    Device(std::wstring pnpGuid, std::wstring name, DeviceFlowEnum flow, uint16_t renderVolume);
    Device(const Device & toCopy);
    Device(Device && toMove) noexcept;
    Device & operator=(const Device & toCopy);
    Device & operator=(Device && toMove) noexcept;

public:
    [[nodiscard]] std::wstring GetName() const override;
    [[nodiscard]] std::wstring GetPnpId() const override;
    [[nodiscard]] DeviceFlowEnum GetFlow() const override;
    [[nodiscard]] uint16_t GetCurrentRenderVolume() const override;
    void SetCurrentRenderVolume(uint16_t volume);

private:
    std::wstring pnpGuid_;
    std::wstring name_;
    DeviceFlowEnum flow_;
    uint16_t renderVolume_;
};
}
