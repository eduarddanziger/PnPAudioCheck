namespace AudioClient;

public class AudioDeviceService
{
    private readonly ulong _serviceHandle;
    public AudioDeviceService(AcEventDelegate eventDelegate, AcLogDelegate logItDelegate, string filter)
    {
#pragma warning disable CA1806
        AudioControllerInterop.AcInitialize(out _serviceHandle, filter, eventDelegate, logItDelegate);
#pragma warning restore CA1806
    }

    public AudioDeviceInfo GetAudioDevice()
    {
#pragma warning disable CA1806
        AudioControllerInterop.AcGetAttached(_serviceHandle, out var device);
#pragma warning restore CA1806
        return new AudioDeviceInfo
            { PnPUuid = device.Guid, DeviceName = device.Name, VolumeLevel = device.Volume }; ;
    }


}