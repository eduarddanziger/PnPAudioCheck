/**
 * @file AudioCheckDllApi.h
 * @brief Interface for the Audio Controller, Version 1.0.
 *
 * This file contains the declarations for initializing, managing, and
 * interacting with audio devices via the audio controller DLL.
 *
 * @note This file is part of a dynamic link library and uses conditional
 *       compilation to handle import/export symbols.
 *
 */

#ifndef AUDIO_CONTROL_DLL_API_H
#define AUDIO_CONTROL_DLL_API_H

#include <Windows.h>

#ifdef AC_EXPORTS
#define AC_EXPORT_IMPORT_DECL __declspec(dllexport)
#else
#define AC_EXPORT_IMPORT_DECL __declspec(dllimport)
#endif

#ifdef __cplusplus
    extern "C" {
#endif

    /**
     * @typedef AcHandle
     * @brief Handle type used to identify a specific audio check session.
     */
    typedef DWORD64 AcHandle;

    /**
     * @typedef AcResult
     * @brief Result type returned by audio check functions.
     */
    typedef INT32 AcResult;

    /**
     * @struct AcDescription
     * @brief Describes an audio device.
     *
     * This structure holds information about an audio device, including its
     * GUID, name, and volume level.
     *
     * @var AcDescription::Guid
     *  Unique identifier for the audio device.
     *
     * @var AcDescription::Name
     *  The name of the audio device.
     *
     * @var AcDescription::Volume
     *  The volume level of the audio device.
     */
    typedef struct {
        WCHAR Guid[40];
        WCHAR Name[128];
        UINT16 Volume;
    } AcDescription;

    typedef enum {  // NOLINT(performance-enum-size)
        TAcAttachedEvent,
        TAcDetachedEvent,
        TAcVolumeChangedEvent
    } TAcEvent;

    /**
     * @typedef TAcEventCallback
     * @brief Callback type for device discovery events.
     *
     * This callback function is invoked when an audio device is either attached
     * or detached.
     *
     * @param hint Indicates whether the device is being attached (TAcAttachedEvent), detached (TAcDetachedEvent) or volume changed TAcVolumeChangedEvent
     */
    typedef void(__stdcall* TAcEventCallback)(
        _In_ UINT8 hint
        );

    /**
     * @typedef TAcLog
     * @brief Callback type for logging events.
     *
     * This callback function is used to log information or error messages during
     * the operation of the audio check DLL.
     *
     * @param isError Indicates whether the message is an error (true) or informational (false).
     * @param infoLine Pointer to the string containing the log message.
     */
    typedef void(__stdcall* TAcLog)(
        _In_  BOOL            isError,
        _In_  PCWSTR          infoLine
        );

    /**
     * @brief Initializes the audio check session.
     *
     * This function initializes the audio check session and provides a handle
     * for subsequent operations. It also allows the user to set callbacks for
     * device discovery and logging.
     *
     * @param[out] handle Pointer to the handle that will be initialized.
     * @param[in] deviceFilter Filter string for selecting specific devices.
     * @param[in, optional] eventCallback Callback function for device events.
     * @param[in, optional] logCallback Callback function for logging events.
     *
     * @return AcResult Result code indicating the success or failure of the operation.
     */
    AC_EXPORT_IMPORT_DECL
        AcResult __stdcall AcInitialize(
            _Out_ AcHandle* handle,
            _In_  PCWSTR  deviceFilter,
            _In_opt_ TAcEventCallback eventCallback,
            _In_opt_ TAcLog logCallback
        );

    /**
     * @brief Checks if an audio device is attached.
     *
     * This function retrieves the description of the audio device currently attached
     * to the specified audio check session.
     *
     * @param[in] handle The handle identifying the audio check session.
     * @param[out] description Pointer to the structure that will hold the device description.
     *
     * @return AcResult Result code indicating the success or failure of the operation.
     */
    AC_EXPORT_IMPORT_DECL
        AcResult __stdcall AcGetAttached(
            _In_ AcHandle handle,
            _Out_  AcDescription* description
        );

    /**
     * @brief Uninitializes the audio check session.
     *
     * This function uninitializes the audio check session and releases any resources
     * associated with it.
     *
     * @param[in] handle The handle identifying the audio check session.
     *
     * @return AcResult Result code indicating the success or failure of the operation.
     */
    AC_EXPORT_IMPORT_DECL
        AcResult __stdcall AcUnInitialize(
            _In_ AcHandle handle
        );

#ifdef __cplusplus
}
#endif

#endif // AUDIO_CONTROL_DLL_API_H
