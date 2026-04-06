#include "notification_native.h"

#ifdef _WIN32
#define COBJMACROS
#include <objbase.h>
#include <shobjidl.h>
#include <windows.h>

#include <stdlib.h>

#ifdef _MSC_VER
#pragma comment(lib, "ole32.lib")
#pragma comment(lib, "shell32.lib")
#pragma comment(lib, "user32.lib")
#endif

/* Maps the shared notification level to a stock Windows icon. */
static HICON desktop_notification_level_icon(int32_t level) {
  switch (level) {
  case DESKTOP_NOTIFICATION_LEVEL_WARNING:
    return LoadIconW(NULL, (LPCWSTR)IDI_WARNING);
  case DESKTOP_NOTIFICATION_LEVEL_ERROR:
    return LoadIconW(NULL, (LPCWSTR)IDI_ERROR);
  default:
    return LoadIconW(NULL, (LPCWSTR)IDI_INFORMATION);
  }
}

/* Maps the shared notification level to the matching balloon flag. */
static DWORD desktop_notification_info_flags(int32_t level) {
  switch (level) {
  case DESKTOP_NOTIFICATION_LEVEL_WARNING:
    return NIIF_WARNING;
  case DESKTOP_NOTIFICATION_LEVEL_ERROR:
    return NIIF_ERROR;
  default:
    return NIIF_INFO;
  }
}

/* Converts a UTF-8 string into a newly allocated wide string. */
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

/*
 * Sends a desktop notification through the legacy Windows shell API.
 *
 * The function validates the payload, converts UTF-8 strings to UTF-16, and
 * uses `IUserNotification` to display a balloon notification with the closest
 * stock icon for the requested level.
 */
MOONBIT_FFI_EXPORT int32_t desktop_notification_windows_show(
    int64_t window_handle, moonbit_bytes_t title, moonbit_bytes_t body,
    int32_t level) {
  const char *title_utf8 = (const char *)title;
  const char *body_utf8 = (const char *)body;
  wchar_t *owned_title_text = NULL;
  wchar_t *owned_body_text = NULL;
  const wchar_t *title_text = NULL;
  const wchar_t *body_text = NULL;
  const wchar_t *tooltip_text = NULL;
  HRESULT hr;
  int initialized = 0;
  int32_t success = 0;
  IUserNotification *notification = NULL;
  HICON icon = NULL;

  (void)window_handle;

  if (!desktop_notification_payload_is_valid(title_utf8, body_utf8)) {
    return 0;
  }
  if (desktop_notification_dry_run_enabled()) {
    return 1;
  }

  owned_title_text = desktop_notification_utf8_to_wide(title_utf8);
  owned_body_text = desktop_notification_utf8_to_wide(body_utf8);
  if (owned_title_text == NULL || owned_body_text == NULL) {
    goto cleanup;
  }

  title_text = owned_title_text;
  tooltip_text = owned_title_text;
  body_text = owned_body_text;

  hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
  if (SUCCEEDED(hr)) {
    initialized = 1;
  } else if (hr != RPC_E_CHANGED_MODE) {
    goto cleanup;
  }

  hr = CoCreateInstance(&CLSID_UserNotification, NULL, CLSCTX_INPROC_SERVER,
                        &IID_IUserNotification, (void **)&notification);
  if (FAILED(hr) || notification == NULL) {
    goto cleanup;
  }

  icon = desktop_notification_level_icon(level);
  hr = IUserNotification_SetIconInfo(notification, icon, tooltip_text);
  if (FAILED(hr)) {
    goto cleanup;
  }

  hr = IUserNotification_SetBalloonInfo(
      notification, title_text, body_text, desktop_notification_info_flags(level));
  if (FAILED(hr)) {
    goto cleanup;
  }

  hr = IUserNotification_Show(notification, NULL, 5000);
  success = SUCCEEDED(hr);

cleanup:
  if (notification != NULL) {
    IUserNotification_Release(notification);
  }
  free(owned_title_text);
  free(owned_body_text);
  if (initialized) {
    CoUninitialize();
  }
  return success;
}
#endif
