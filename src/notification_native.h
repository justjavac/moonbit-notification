#ifndef JUSTJAVAC_NOTIFICATION_NATIVE_H
#define JUSTJAVAC_NOTIFICATION_NATIVE_H

#include <stddef.h>
#include <stdint.h>

#include "moonbit.h"

#define DESKTOP_NOTIFICATION_APP_NAME "Lepus"
#define DESKTOP_NOTIFICATION_PATH_BUFFER_SIZE 4096

int desktop_notification_dry_run_enabled(void);
MOONBIT_FFI_EXPORT int32_t desktop_notification_backend_kind(void);
MOONBIT_FFI_EXPORT int32_t desktop_notification_is_supported(void);
MOONBIT_FFI_EXPORT int32_t desktop_notification_show(
    int64_t window_handle, moonbit_bytes_t title, moonbit_bytes_t body,
    int32_t level);

#ifdef _WIN32
MOONBIT_FFI_EXPORT int32_t desktop_notification_windows_show(
    int64_t window_handle, moonbit_bytes_t title, moonbit_bytes_t body,
    int32_t level);
#endif

#if defined(__APPLE__) || defined(__linux__)
int desktop_notification_run_process(char *const argv[]);
#endif

#ifdef __APPLE__
MOONBIT_FFI_EXPORT int32_t desktop_notification_macos_is_supported(void);
MOONBIT_FFI_EXPORT int32_t desktop_notification_macos_show(
    int64_t window_handle, moonbit_bytes_t title, moonbit_bytes_t body,
    int32_t level);
#endif

#ifdef __linux__
int desktop_notification_find_program(
    const char *program, char *buffer, size_t buffer_size);
MOONBIT_FFI_EXPORT int32_t desktop_notification_linux_is_supported(void);
MOONBIT_FFI_EXPORT int32_t desktop_notification_linux_show(
    int64_t window_handle, moonbit_bytes_t title, moonbit_bytes_t body,
    int32_t level);
#endif

#endif
