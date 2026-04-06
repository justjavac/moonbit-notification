#ifndef JUSTJAVAC_NOTIFICATION_NATIVE_H
#define JUSTJAVAC_NOTIFICATION_NATIVE_H

#include <stddef.h>
#include <stdint.h>

#include "moonbit.h"

#define DESKTOP_NOTIFICATION_APP_NAME "Lepus"
#define DESKTOP_NOTIFICATION_PATH_BUFFER_SIZE 4096

int desktop_notification_dry_run_enabled(void);

#if defined(__APPLE__) || defined(__linux__)
int desktop_notification_run_process(char *const argv[]);
#endif

#ifdef __linux__
int desktop_notification_find_program(
    const char *program, char *buffer, size_t buffer_size);
#endif

#endif
