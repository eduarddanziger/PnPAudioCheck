// ReSharper disable CppClangTidyModernizeUseNodiscard
#pragma once

#ifdef AC_EXPORTS
#define AC_EXPORT_IMPORT_DECL __declspec(dllexport)
#else
#define AC_EXPORT_IMPORT_DECL __declspec(dllimport)
#endif

#include <memory>

#include "ClassDefHelper.h"

class DeviceCollectionInterface;
class DeviceCollectionObserver;
class DeviceInterface;
class DeviceCollectionObserverInterface;

enum class AC_EXPORT_IMPORT_DECL DeviceCollectionEvent : uint8_t {
    None = 0,
    Discovered,
    Detached,
    VolumeChanged
};

enum class AC_EXPORT_IMPORT_DECL DeviceFlowEnum : uint8_t {
    None = 0,
    Render,
    Capture,
    RenderAndCapture
};

class AC_EXPORT_IMPORT_DECL AudioControl {
public:
    static std::unique_ptr<DeviceCollectionInterface> CreateDeviceCollection(
        const std::wstring & nameFilter, bool bothHeadsetAndMicro = false);

    DISALLOW_COPY_MOVE(AudioControl);
    AudioControl() = delete;
    ~AudioControl() = delete;
};

class AC_EXPORT_IMPORT_DECL DeviceCollectionInterface {
public:
    virtual size_t GetSize() const = 0;
    virtual std::unique_ptr<DeviceInterface> CreateItem(size_t deviceNumber) const = 0;

    virtual void Subscribe(DeviceCollectionObserverInterface & observer) = 0;
    virtual void Unsubscribe(DeviceCollectionObserverInterface & observer) = 0;

    virtual void ResetContent() = 0;

    AS_INTERFACE(DeviceCollectionInterface);
    DISALLOW_COPY_MOVE(DeviceCollectionInterface);
};

class AC_EXPORT_IMPORT_DECL DeviceCollectionObserverInterface {
public:
    virtual void OnCollectionChanged(DeviceCollectionEvent event, const std::wstring & devicePnpId) = 0;
    virtual void OnTrace(const std::wstring & line) = 0;
    virtual void OnTraceDebug(const std::wstring & line) = 0;

    AS_INTERFACE(DeviceCollectionObserverInterface);
    DISALLOW_COPY_MOVE(DeviceCollectionObserverInterface);
};


class AC_EXPORT_IMPORT_DECL DeviceInterface {
public:
    virtual std::wstring GetName() const = 0;
    virtual std::wstring GetPnpId() const = 0;
    virtual DeviceFlowEnum GetFlow() const = 0;
    virtual uint16_t GetCurrentRenderVolume() const = 0;
    virtual uint16_t GetCurrentCaptureVolume() const = 0;

    AS_INTERFACE(DeviceInterface);
    DISALLOW_COPY_MOVE(DeviceInterface);
};
