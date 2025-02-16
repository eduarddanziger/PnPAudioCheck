#include "stdafx.h"

#include "Device.h"

ed::audio::Device::~Device() = default;

ed::audio::Device::Device()
    : Device(L"", L"", DeviceFlowEnum::None, 0, 0)
{
}

// ReSharper disable once CppParameterMayBeConst
ed::audio::Device::Device(std::wstring pnpGuid, std::wstring name, DeviceFlowEnum flow, uint16_t renderVolume,
                          uint16_t captureVolume)
    : pnpGuid_(std::move(pnpGuid))
      , name_(std::move(name))
      , flow_(flow)
      , renderVolume_(renderVolume)
      , captureVolume_(captureVolume)
{
}

ed::audio::Device::Device(const Device & toCopy)
    : pnpGuid_(toCopy.pnpGuid_)
      , name_(toCopy.name_)
      , flow_(toCopy.flow_)
      , renderVolume_(toCopy.renderVolume_)
      , captureVolume_(toCopy.captureVolume_)
{
}

ed::audio::Device::Device(Device && toMove) noexcept
    : pnpGuid_(std::move(toMove.pnpGuid_))
      , name_(std::move(toMove.name_))
      , flow_(toMove.flow_)
      , renderVolume_(toMove.renderVolume_)
      , captureVolume_(toMove.captureVolume_)
{
}

ed::audio::Device & ed::audio::Device::operator=(const Device & toCopy)
{
    if (this != &toCopy)
    {
        pnpGuid_ = toCopy.pnpGuid_;
        name_ = toCopy.name_;
        flow_ = toCopy.flow_;
        renderVolume_ = toCopy.renderVolume_;
        captureVolume_ = toCopy.captureVolume_;
    }
    return *this;
}

ed::audio::Device & ed::audio::Device::operator=(Device && toMove) noexcept
{
    if (this != &toMove)
    {
        pnpGuid_ = std::move(toMove.pnpGuid_);
        name_ = std::move(toMove.name_);
        flow_ = toMove.flow_;
        renderVolume_ = toMove.renderVolume_;
        captureVolume_ = toMove.captureVolume_;
    }
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

uint16_t ed::audio::Device::GetCurrentCaptureVolume() const
{
    return captureVolume_;
}

void ed::audio::Device::SetCurrentRenderVolume(uint16_t volume)
{
    renderVolume_ = volume;
}

void ed::audio::Device::SetCurrentCaptureVolume(uint16_t volume)
{
    captureVolume_ = volume;
}

