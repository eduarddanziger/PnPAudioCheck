#include "stdafx.h"
#include "AudioControlInterface.h"

#include <mmdeviceapi.h>

#include <stdexcept>

#include "DeviceCollection.h"


std::unique_ptr<DeviceCollectionInterface> AudioControl::CreateDeviceCollection(const std::wstring& nameFilter, bool bothHeadsetAndMicro)
{
    return std::make_unique<ed::audio::DeviceCollection>(nameFilter, bothHeadsetAndMicro);
}
