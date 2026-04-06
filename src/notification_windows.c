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
  IUserNotification *notification = NULL;
  HICON icon = desktop_notification_level_icon(level);

  (void)window_handle;

  if (title_utf8 == NULL || title_utf8[0] == '\0' || body_utf8 == NULL ||
      body_utf8[0] == '\0') {
    return 0;
  }
  if (desktop_notification_dry_run_enabled()) {
    return 1;
  }

  owned_title_text = desktop_notification_utf8_to_wide(title_utf8);
  owned_body_text = desktop_notification_utf8_to_wide(body_utf8);
  if (owned_title_text == NULL || owned_body_text == NULL) {
    free(owned_title_text);
    free(owned_body_text);
    return 0;
  }

  title_text = owned_title_text;
  tooltip_text = owned_title_text;
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
}
#endif
