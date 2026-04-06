#include "notification_native.h"

#include <stdlib.h>
#include <string.h>

#if defined(__APPLE__) || defined(__linux__)
#include <spawn.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
extern char **environ;
#endif

#ifdef __linux__
#include <stdio.h>
#include <unistd.h>
#endif

static int32_t desktop_notification_force_dry_run = 0;

/* Returns nonzero when the string is neither NULL nor empty. */
static int desktop_notification_has_text(const char *text) {
  return text != NULL && text[0] != '\0';
}

/*
 * Returns whether the native layer should short-circuit delivery attempts.
 *
 * Dry-run mode can be forced by tests or enabled through the
 * MOONBIT_NOTIFICATION_DRY_RUN environment variable.
 */
int desktop_notification_dry_run_enabled(void) {
  const char *value = getenv("MOONBIT_NOTIFICATION_DRY_RUN");
  if (desktop_notification_force_dry_run) {
    return 1;
  }
  if (value == NULL) {
    return 0;
  }
  return strcmp(value, "1") == 0 || strcmp(value, "true") == 0 ||
         strcmp(value, "TRUE") == 0;
}

/*
 * Enables or disables dry-run mode for test environments.
 *
 * When enabled, platform backends skip the actual OS call and report success
 * after validation.
 */
MOONBIT_FFI_EXPORT void desktop_notification_set_dry_run(int32_t enabled) {
  desktop_notification_force_dry_run = enabled != 0;
}

/* Returns nonzero when both notification text fields are valid. */
int desktop_notification_payload_is_valid(const char *title, const char *body) {
  return desktop_notification_has_text(title) &&
         desktop_notification_has_text(body);
}

/*
 * Returns the compiled platform backend identifier for the current build.
 *
 * The MoonBit layer uses this identifier to choose a stable, platform-specific
 * support error message.
 */
MOONBIT_FFI_EXPORT int32_t desktop_notification_backend_kind(void) {
#if defined(_WIN32)
  return DESKTOP_NOTIFICATION_BACKEND_WINDOWS;
#elif defined(__APPLE__)
  return DESKTOP_NOTIFICATION_BACKEND_MACOS;
#elif defined(__linux__)
  return DESKTOP_NOTIFICATION_BACKEND_LINUX;
#else
  return DESKTOP_NOTIFICATION_BACKEND_UNSUPPORTED;
#endif
}

/*
 * Returns whether the active platform backend can attempt notification
 * delivery.
 *
 * Windows always reports support, while macOS and Linux delegate to runtime
 * checks for backend-specific executables.
 */
MOONBIT_FFI_EXPORT int32_t desktop_notification_is_supported(void) {
#if defined(_WIN32)
  return 1;
#elif defined(__APPLE__)
  return desktop_notification_macos_is_supported();
#elif defined(__linux__)
  return desktop_notification_linux_is_supported();
#else
  return 0;
#endif
}

/*
 * Dispatches the notification to the backend selected for the current target.
 *
 * The dispatcher keeps backend selection in one place so the MoonBit layer can
 * call a single FFI symbol regardless of platform.
 */
MOONBIT_FFI_EXPORT int32_t desktop_notification_show(
    int64_t window_handle, moonbit_bytes_t title, moonbit_bytes_t body,
    int32_t level) {
#if defined(_WIN32)
  return desktop_notification_windows_show(window_handle, title, body, level);
#elif defined(__APPLE__)
  return desktop_notification_macos_show(window_handle, title, body, level);
#elif defined(__linux__)
  return desktop_notification_linux_show(window_handle, title, body, level);
#else
  (void)window_handle;
  (void)title;
  (void)body;
  (void)level;
  return 0;
#endif
}

#if defined(__APPLE__) || defined(__linux__)
/* Spawns a child process and reports whether it exits with status code 0. */
int desktop_notification_run_process(char *const argv[]) {
  pid_t pid = 0;
  int status = 0;

  if (posix_spawn(&pid, argv[0], NULL, NULL, argv, environ) != 0) {
    return 0;
  }
  if (waitpid(pid, &status, 0) < 0) {
    return 0;
  }
  return WIFEXITED(status) && WEXITSTATUS(status) == 0;
}
#endif

#ifdef __linux__
/* Writes `<directory>/<program>` into the buffer when it fits. */
static int desktop_notification_copy_path_segment(
    char *buffer, size_t buffer_size, const char *directory,
    size_t directory_length, const char *program) {
  size_t program_length = strlen(program);
  size_t required_length = directory_length + 1 + program_length + 1;

  if (directory_length == 0 || required_length > buffer_size) {
    return 0;
  }

  memcpy(buffer, directory, directory_length);
  buffer[directory_length] = '/';
  memcpy(buffer + directory_length + 1, program, program_length + 1);
  return 1;
}

/* Tests one directory candidate and optionally copies the resolved path out. */
static int desktop_notification_try_program_in_directory(
    const char *program, const char *directory, size_t directory_length,
    char *buffer, size_t buffer_size) {
  char candidate[DESKTOP_NOTIFICATION_PATH_BUFFER_SIZE];
  size_t candidate_length = 0;

  if (!desktop_notification_copy_path_segment(candidate, sizeof(candidate),
                                              directory, directory_length,
                                              program) ||
      access(candidate, X_OK) != 0) {
    return 0;
  }

  if (buffer != NULL) {
    candidate_length = strlen(candidate);
    if (buffer_size <= candidate_length) {
      return 0;
    }
    memcpy(buffer, candidate, candidate_length + 1);
  }

  return 1;
}

/* Searches PATH and fallback directories for an executable program name. */
int desktop_notification_find_program(const char *program, char *buffer,
                                      size_t buffer_size) {
  const char *path_env = getenv("PATH");
  const char *fallback_dirs[] = {"/usr/bin", "/bin", "/usr/local/bin", NULL};

  if (!desktop_notification_has_text(program)) {
    return 0;
  }

  if (path_env != NULL) {
    const char *segment = path_env;

    while (*segment != '\0') {
      const char *separator = strchr(segment, ':');
      size_t segment_length =
          separator == NULL ? strlen(segment) : (size_t)(separator - segment);

      if (segment_length > 0 &&
          desktop_notification_try_program_in_directory(
              program, segment, segment_length, buffer, buffer_size)) {
        return 1;
      }

      if (separator == NULL) {
        break;
      }

      segment = separator + 1;
    }
  }

  for (size_t index = 0; fallback_dirs[index] != NULL; index += 1) {
    if (desktop_notification_try_program_in_directory(
            program, fallback_dirs[index], strlen(fallback_dirs[index]), buffer,
            buffer_size)) {
      return 1;
    }
  }

  return 0;
}
#endif
