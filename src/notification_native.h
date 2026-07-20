#ifndef JUSTJAVAC_NOTIFICATION_NATIVE_H
#define JUSTJAVAC_NOTIFICATION_NATIVE_H

#include <stddef.h>
#include <stdint.h>

#include "moonbit.h"

#define DESKTOP_NOTIFICATION_APP_NAME "Lepus"
#define DESKTOP_NOTIFICATION_PATH_BUFFER_SIZE 4096
#define DESKTOP_NOTIFICATION_LEVEL_INFO 0
#define DESKTOP_NOTIFICATION_LEVEL_WARNING 1
#define DESKTOP_NOTIFICATION_LEVEL_ERROR 2
#define DESKTOP_NOTIFICATION_BACKEND_UNSUPPORTED 0
#define DESKTOP_NOTIFICATION_BACKEND_WINDOWS 1
#define DESKTOP_NOTIFICATION_BACKEND_MACOS 2
#define DESKTOP_NOTIFICATION_BACKEND_LINUX 3
#define DESKTOP_NOTIFICATION_STATUS_OK 0
#define DESKTOP_NOTIFICATION_STATUS_UNSUPPORTED 1
#define DESKTOP_NOTIFICATION_STATUS_BACKEND_UNAVAILABLE 2
#define DESKTOP_NOTIFICATION_STATUS_APP_IDENTITY_REQUIRED 3
#define DESKTOP_NOTIFICATION_STATUS_AUTHORIZATION_DENIED 4
#define DESKTOP_NOTIFICATION_STATUS_AUTHORIZATION_FAILED 5
#define DESKTOP_NOTIFICATION_STATUS_SCHEDULING_FAILED 6
#define DESKTOP_NOTIFICATION_STATUS_DELIVERY_FAILED 7
#define DESKTOP_NOTIFICATION_STATUS_APP_BACKEND_UNAVAILABLE 8

/* Returns nonzero when dry-run mode is enabled for the native layer. */
int desktop_notification_dry_run_enabled(void);

/* Returns nonzero when both title and body point to non-empty strings. */
int desktop_notification_payload_is_valid(const char *title, const char *body);

/*
 * Returns the platform backend identifier compiled into the native stub.
 *
 * The result is one of the DESKTOP_NOTIFICATION_BACKEND_* constants so the
 * MoonBit layer can map runtime support failures to stable user-facing errors.
 */
MOONBIT_FFI_EXPORT int32_t desktop_notification_backend_kind(void);

/*
 * Returns whether the active native backend can attempt notification delivery.
 *
 * Support may depend on runtime prerequisites such as `/usr/bin/osascript` or
 * the `notify-send` executable being available in the current environment.
 */
MOONBIT_FFI_EXPORT int32_t desktop_notification_is_supported(void);

/* Returns a DESKTOP_NOTIFICATION_STATUS_* value for the automatic path. */
MOONBIT_FFI_EXPORT int32_t desktop_notification_support_status(void);

/*
 * Dispatches a notification through the backend selected for the current
 * platform.
 *
 * `title` and `body` are expected to be UTF-8 strings terminated by a trailing
 * zero byte. The function returns a DESKTOP_NOTIFICATION_STATUS_* value.
 */
MOONBIT_FFI_EXPORT int32_t desktop_notification_show(
    int64_t window_handle, moonbit_bytes_t title, moonbit_bytes_t body,
    int32_t level);

/* Uses the platform's app-oriented delivery path when it has one. */
MOONBIT_FFI_EXPORT int32_t desktop_notification_show_app(
    int64_t window_handle, moonbit_bytes_t title, moonbit_bytes_t body,
    int32_t level);

/* Uses the platform's command-line delivery path when it has one. */
MOONBIT_FFI_EXPORT int32_t desktop_notification_show_cli(
    int64_t window_handle, moonbit_bytes_t title, moonbit_bytes_t body,
    int32_t level);

#ifdef _WIN32
/*
 * Sends a notification using the Windows shell notification implementation.
 *
 * The backend maps `level` to the closest stock Windows balloon icon and
 * displays the notification through `IUserNotification`.
 */
MOONBIT_FFI_EXPORT int32_t desktop_notification_windows_show(
    int64_t window_handle, moonbit_bytes_t title, moonbit_bytes_t body,
    int32_t level);
#endif

#if defined(__APPLE__) || defined(__linux__)
/* Runs a process and returns nonzero only when it exits successfully. */
int desktop_notification_run_process(char *const argv[]);
#endif

#ifdef __APPLE__
/* Returns whether automatic macOS delivery can currently be attempted. */
MOONBIT_FFI_EXPORT int32_t desktop_notification_macos_is_supported(void);

/* Returns support status for automatic macOS delivery selection. */
int32_t desktop_notification_macos_support_status(void);

/* Selects UserNotifications or osascript from the host application identity. */
MOONBIT_FFI_EXPORT int32_t desktop_notification_macos_show(
    int64_t window_handle, moonbit_bytes_t title, moonbit_bytes_t body,
    int32_t level);

/* Sends through UserNotifications for an identified macOS application. */
int32_t desktop_notification_macos_show_app(
    int64_t window_handle, moonbit_bytes_t title, moonbit_bytes_t body,
    int32_t level);

/* Sends through osascript for an unbundled macOS command-line process. */
int32_t desktop_notification_macos_show_cli(
    int64_t window_handle, moonbit_bytes_t title, moonbit_bytes_t body,
    int32_t level);
#endif

#ifdef __linux__
/* Resolves an executable name against PATH and common fallback directories. */
int desktop_notification_find_program(
    const char *program, char *buffer, size_t buffer_size);

/*
 * Returns whether the Linux backend can find a runnable `notify-send`
 * executable in the current environment.
 */
MOONBIT_FFI_EXPORT int32_t desktop_notification_linux_is_supported(void);

/*
 * Sends a notification by spawning `notify-send` with the mapped urgency flag.
 *
 * The backend ignores `window_handle`, validates that the payload is non-empty,
 * and converts the shared notification level into `low`, `normal`, or
 * `critical`.
 */
MOONBIT_FFI_EXPORT int32_t desktop_notification_linux_show(
    int64_t window_handle, moonbit_bytes_t title, moonbit_bytes_t body,
    int32_t level);
#endif

#endif
