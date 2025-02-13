#include "stdafx.h"

#include "Device.h"

ed::audio::Device::~Device() = default;

ed::audio::Device::Device()
    : Device(L"", L"", DeviceFlowEnum::None, 0)
{
}

ed::audio::Device::Device(std::wstring pnpGuid, std::wstring name, const DeviceFlowEnum flow, const uint16_t renderVolume)
    : pnpGuid_(std::move(pnpGuid))
      , name_(std::move(name))
      , flow_(flow)
      , renderVolume_(renderVolume)
{
}

ed::audio::Device::Device(const Device & toCopy)
    : pnpGuid_(toCopy.pnpGuid_)
      , name_(toCopy.name_)
      , flow_(toCopy.flow_)
      , renderVolume_(toCopy.renderVolume_)
{
}

ed::audio::Device::Device(Device && toMove) noexcept
    : pnpGuid_(std::move(toMove.pnpGuid_))
      , name_(std::move(toMove.name_))
      , flow_(std::exchange(toMove.flow_, DeviceFlowEnum::None))
      , renderVolume_(std::exchange(toMove.renderVolume_, 0))
{
}


ed::audio::Device & ed::audio::Device::operator =(const ed::audio::Device & toCopy)
{
    pnpGuid_ = toCopy.pnpGuid_;
    name_ = toCopy.name_;
    flow_ = toCopy.flow_;
    renderVolume_ = toCopy.renderVolume_;
    return *this;
}

ed::audio::Device & ed::audio::Device::operator =(ed::audio::Device && toMove) noexcept
{
    pnpGuid_ = std::move(toMove.pnpGuid_);
    name_ = std::move(toMove.name_);
    flow_ = std::exchange(toMove.flow_, DeviceFlowEnum::None);
    renderVolume_ = std::exchange(toMove.renderVolume_, 0);
    return *this;
}

std::wstring ed::audio::Device::GetName() const
{
    return name_;
}

std::wstring ed::audio::Device::GetPnpId() const
{
    return pnpGuid_;
}

DeviceFlowEnum ed::audio::Device::GetFlow() const
{
    return flow_;
}

uint16_t ed::audio::Device::GetCurrentRenderVolume() const
{
    return renderVolume_;
}

void ed::audio::Device::SetCurrentRenderVolume(uint16_t volume)
{
    renderVolume_ = volume;
}
