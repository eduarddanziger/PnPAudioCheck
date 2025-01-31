using System;
using System.Runtime.InteropServices;

namespace AudioClient;

// ulong // typedef DWORD64 AcHandle;
// int // typedef INT32 AcResult;


[StructLayout(LayoutKind.Sequential, CharSet = CharSet.Unicode)]
public struct AcDescription
{
    [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 40)]
    public string Guid;

    [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 128)]
    public string Name;

    public ushort Volume;
};

public enum AcEvent : byte
{
    AcAttachedEvent = 0,
    AcDetachedEvent = 1,
    AcVolumeChangedEvent = 2
}

[UnmanagedFunctionPointer(CallingConvention.StdCall)]
public delegate void AcEventDelegate(
    byte hint
);

[UnmanagedFunctionPointer(CallingConvention.StdCall)]
public delegate void AcLogDelegate(
    [MarshalAs(UnmanagedType.Bool)] bool isError,
    [MarshalAs(UnmanagedType.LPWStr)] string infoLine
);


public static class AudioControllerInterop
{
    [DllImport("AudioController.dll", CallingConvention = CallingConvention.StdCall)]
    public static extern int AcInitialize(
        out ulong handle,
        [MarshalAs(UnmanagedType.LPWStr)] string deviceFilter,
        [MarshalAs(UnmanagedType.FunctionPtr)] AcEventDelegate eventDelegate,
        [MarshalAs(UnmanagedType.FunctionPtr)] AcLogDelegate logDelegate
    );

    [DllImport("AudioController.dll", CallingConvention = CallingConvention.StdCall)]
    public static extern int AcGetAttached(
        ulong handle,
        out AcDescription description
    );

    [DllImport("AudioController.dll", CallingConvention = CallingConvention.StdCall)]
    public static extern int AcUnInitialize(
        ulong handle
    );
}