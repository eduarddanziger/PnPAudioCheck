namespace AudioClient;

public class AudioDeviceInfo
{
    public string PnPUuid { get; set; } = "";
    public string DeviceName { get; set; } = "";
    public float VolumeLevel { get; set; }
}