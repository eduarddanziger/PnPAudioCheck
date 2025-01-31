#pragma once

#include <string>
#include <mmdeviceapi.h>

#include "../AudioController/AudioControlInterface.h"


#ifndef COMMAND_CASE
#define COMMAND_CASE(cmd) case cmd: return L#cmd;  // NOLINT(cppcoreguidelines-macro-usage)
#endif

#ifndef COMMAND_CASE2
#define COMMAND_CASE2(enumClass, cmd) case enumClass::cmd: return L#cmd;  // NOLINT(cppcoreguidelines-macro-usage)
#endif


namespace ed {
template <class T>
std::wstring GetEDataFlowAsString(T v)
{
    switch (v)
    {
    COMMAND_CASE(eRender)
    COMMAND_CASE(eCapture)
    COMMAND_CASE(eAll)
    default:
        return L"Unknown data flow";
    }
}

inline std::wstring GetFlowAsString(DeviceFlowEnum v)
{
    switch (v)
    {
    COMMAND_CASE2(DeviceFlowEnum, Render)
    COMMAND_CASE2(DeviceFlowEnum, Capture)
    COMMAND_CASE2(DeviceFlowEnum, RenderAndCapture)
    case DeviceFlowEnum::None:
    default: // NOLINT(clang-diagnostic-covered-switch-default)
        return L"Unknown flow";
    }
}

inline std::wstring GetDeviceCollectionEventAsString(DeviceCollectionEvent v)
{
    switch (v)
    {
    COMMAND_CASE2(DeviceCollectionEvent, Discovered)
    COMMAND_CASE2(DeviceCollectionEvent, Detached)
    COMMAND_CASE2(DeviceCollectionEvent, VolumeChanged)
    case DeviceCollectionEvent::None:
    default: // NOLINT(clang-diagnostic-covered-switch-default)
        return L"Unknown event";
    }
}
}
