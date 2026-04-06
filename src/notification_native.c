#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "moonbit.h"

#ifdef _WIN32
#define COBJMACROS
#include <objbase.h>
#include <shobjidl.h>
#include <windows.h>
#ifdef _MSC_VER
#pragma comment(lib, "ole32.lib")
#pragma comment(lib, "shell32.lib")
#pragma comment(lib, "user32.lib")
#endif
#endif

#if defined(__APPLE__) || defined(__linux__)
#include <spawn.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
extern char **environ;
#endif

#define DESKTOP_NOTIFICATION_PATH_BUFFER_SIZE 4096
#define DESKTOP_NOTIFICATION_DEFAULT_TITLE "MoonBit"

static int32_t desktop_notification_force_dry_run = 0;

static int desktop_notification_dry_run_enabled(void) {
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

MOONBIT_FFI_EXPORT void desktop_notification_set_dry_run(int32_t enabled) {
  desktop_notification_force_dry_run = enabled != 0;
}

#if defined(__APPLE__) || defined(__linux__)
static int desktop_notification_run_process(char *const argv[]) {
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
static int desktop_notification_copy_path(char *buffer, size_t buffer_size,
                                          const char *directory,
                                          const char *program) {
  int written = snprintf(buffer, buffer_size, "%s/%s", directory, program);
  return written > 0 && (size_t)written < buffer_size;
}

static int desktop_notification_find_program(const char *program, char *buffer,
                                             size_t buffer_size) {
  const char *path_env = getenv("PATH");
  const char *fallback_dirs[] = {"/usr/bin", "/bin", "/usr/local/bin", NULL};
  size_t path_length = 0;

  if (path_env != NULL) {
    char *paths = NULL;
    char *cursor = NULL;
    char *segment = NULL;

    path_length = strlen(path_env);
    paths = (char *)malloc(path_length + 1);
    if (paths != NULL) {
      memcpy(paths, path_env, path_length + 1);
      cursor = paths;
      while ((segment = strtok(cursor, ":")) != NULL) {
        char candidate[DESKTOP_NOTIFICATION_PATH_BUFFER_SIZE];
        cursor = NULL;
        if (desktop_notification_copy_path(candidate, sizeof(candidate), segment,
                                           program) &&
            access(candidate, X_OK) == 0) {
          if (buffer != NULL && buffer_size > 0) {
            memcpy(buffer, candidate, strlen(candidate) + 1);
          }
          free(paths);
          return 1;
        }
      }
      free(paths);
    }
  }

  for (size_t index = 0; fallback_dirs[index] != NULL; index += 1) {
    char candidate[DESKTOP_NOTIFICATION_PATH_BUFFER_SIZE];
    if (desktop_notification_copy_path(candidate, sizeof(candidate),
                                       fallback_dirs[index], program) &&
        access(candidate, X_OK) == 0) {
      if (buffer != NULL && buffer_size > 0) {
        memcpy(buffer, candidate, strlen(candidate) + 1);
      }
      return 1;
    }
  }

  return 0;
}
#endif

MOONBIT_FFI_EXPORT int32_t desktop_notification_is_supported(void) {
#ifdef _WIN32
  return 1;
#elif defined(__APPLE__)
  return access("/usr/bin/osascript", X_OK) == 0;
#elif defined(__linux__)
  return desktop_notification_find_program("notify-send", NULL, 0);
#else
  return 0;
#endif
}

MOONBIT_FFI_EXPORT moonbit_bytes_t desktop_notification_support_error(void) {
#ifdef _WIN32
  return (moonbit_bytes_t)
      "desktop notifications are unavailable on the current Windows runtime";
#elif defined(__APPLE__)
  return (moonbit_bytes_t)
      "desktop notifications on macOS require /usr/bin/osascript";
#elif defined(__linux__)
  return (moonbit_bytes_t)
      "desktop notifications on Linux require the notify-send executable";
#else
  return (moonbit_bytes_t)
      "desktop notifications are not supported on this platform";
#endif
}

#ifdef _WIN32
static HICON desktop_notification_level_icon(int32_t level) {
  switch (level) {
  case 1:
    return LoadIconW(NULL, (LPCWSTR)IDI_WARNING);
  case 2:
    return LoadIconW(NULL, (LPCWSTR)IDI_ERROR);
  default:
    return LoadIconW(NULL, (LPCWSTR)IDI_INFORMATION);
  }
}

static DWORD desktop_notification_info_flags(int32_t level) {
  switch (level) {
  case 1:
    return NIIF_WARNING;
  case 2:
    return NIIF_ERROR;
  default:
    return NIIF_INFO;
  }
}

static wchar_t *desktop_notification_utf8_to_wide(const char *text) {
  int wide_length = 0;
  wchar_t *wide_text = NULL;

  if (text == NULL) {
    return NULL;
  }

  wide_length = MultiByteToWideChar(CP_UTF8, 0, text, -1, NULL, 0);
  if (wide_length <= 0) {
    return NULL;
  }

  wide_text = (wchar_t *)malloc((size_t)wide_length * sizeof(wchar_t));
  if (wide_text == NULL) {
    return NULL;
  }

  if (MultiByteToWideChar(CP_UTF8, 0, text, -1, wide_text, wide_length) == 0) {
    free(wide_text);
    return NULL;
  }

  return wide_text;
}
#endif

MOONBIT_FFI_EXPORT int32_t desktop_notification_show(
    int64_t window_handle, moonbit_bytes_t title, moonbit_bytes_t body,
    int32_t level) {
#ifdef _WIN32
  const char *title_utf8 = (const char *)title;
  const char *body_utf8 = (const char *)body;
  wchar_t *owned_title_text = NULL;
  wchar_t *owned_body_text = NULL;
  const wchar_t *title_text = NULL;
  const wchar_t *body_text = NULL;
  const wchar_t *tooltip_text = title_text;
  HRESULT hr;
  int initialized = 0;
  IUserNotification *notification = NULL;
  HICON icon = desktop_notification_level_icon(level);

  (void)window_handle;

  if (body_utf8 == NULL || body_utf8[0] == '\0') {
    return 0;
  }
  if (desktop_notification_dry_run_enabled()) {
    return 1;
  }

  owned_body_text = desktop_notification_utf8_to_wide(body_utf8);
  if (owned_body_text == NULL) {
    return 0;
  }

  if (title_utf8 == NULL || title_utf8[0] == '\0') {
    title_text = L"MoonBit";
    tooltip_text = L"MoonBit";
  } else {
    owned_title_text = desktop_notification_utf8_to_wide(title_utf8);
    if (owned_title_text == NULL) {
      free(owned_body_text);
      return 0;
    }
    title_text = owned_title_text;
    tooltip_text = owned_title_text;
  }
  body_text = owned_body_text;

  hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
  if (SUCCEEDED(hr)) {
    initialized = 1;
  } else if (hr != RPC_E_CHANGED_MODE) {
    free(owned_title_text);
    free(owned_body_text);
    return 0;
  }

  hr = CoCreateInstance(&CLSID_UserNotification, NULL, CLSCTX_INPROC_SERVER,
                        &IID_IUserNotification, (void **)&notification);
  if (FAILED(hr) || notification == NULL) {
    free(owned_title_text);
    free(owned_body_text);
    if (initialized) {
      CoUninitialize();
    }
    return 0;
  }

  hr = IUserNotification_SetIconInfo(notification, icon, tooltip_text);
  if (FAILED(hr)) {
    IUserNotification_Release(notification);
    free(owned_title_text);
    free(owned_body_text);
    if (initialized) {
      CoUninitialize();
    }
    return 0;
  }

  hr = IUserNotification_SetBalloonInfo(
      notification, title_text, body_text, desktop_notification_info_flags(level));
  if (FAILED(hr)) {
    IUserNotification_Release(notification);
    free(owned_title_text);
    free(owned_body_text);
    if (initialized) {
      CoUninitialize();
    }
    return 0;
  }

  hr = IUserNotification_Show(notification, NULL, 5000);
  IUserNotification_Release(notification);
  free(owned_title_text);
  free(owned_body_text);
  if (initialized) {
    CoUninitialize();
  }
  return SUCCEEDED(hr);
#elif defined(__APPLE__)
  const char *title_text = (const char *)title;
  const char *body_text = (const char *)body;
  char *const argv[] = {
      "/usr/bin/osascript",
      "-e",
      "on run argv",
      "-e",
      "display notification (item 1 of argv) with title (item 2 of argv)",
      "-e",
      "end run",
      (char *)body_text,
      (char *)title_text,
      NULL,
  };

  (void)window_handle;

  if (title_text == NULL || title_text[0] == '\0') {
    title_text = DESKTOP_NOTIFICATION_DEFAULT_TITLE;
    argv[7] = (char *)title_text;
  }
  if (body_text == NULL || body_text[0] == '\0') {
    return 0;
  }
  if (desktop_notification_dry_run_enabled()) {
    return 1;
  }

  return desktop_notification_run_process(argv);
#elif defined(__linux__)
  char notify_send_path[DESKTOP_NOTIFICATION_PATH_BUFFER_SIZE];
  const char *title_text = (const char *)title;
  const char *body_text = (const char *)body;
  const char *urgency_text = "normal";
  char *const argv[] = {
      notify_send_path, "--app-name", "MoonBit", "--urgency",
      (char *)urgency_text, (char *)title_text, (char *)body_text, NULL,
  };

  (void)window_handle;

  if (!desktop_notification_find_program("notify-send", notify_send_path,
                                         sizeof(notify_send_path))) {
    return 0;
  }
  if (title_text == NULL || title_text[0] == '\0') {
    title_text = DESKTOP_NOTIFICATION_DEFAULT_TITLE;
    argv[5] = (char *)title_text;
  }
  if (body_text == NULL || body_text[0] == '\0') {
    return 0;
  }
  switch (level) {
  case 0:
    urgency_text = "low";
    break;
  case 2:
    urgency_text = "critical";
    break;
  default:
    urgency_text = "normal";
    break;
  }
  argv[4] = (char *)urgency_text;
  if (desktop_notification_dry_run_enabled()) {
    return 1;
  }

  return desktop_notification_run_process(argv);
#else
  (void)window_handle;
  (void)title;
  (void)body;
  (void)level;
  return 0;
#endif
}
