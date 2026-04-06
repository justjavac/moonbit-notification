#include "notification_native.h"

#ifdef __linux__

MOONBIT_FFI_EXPORT int32_t desktop_notification_linux_is_supported(void) {
  return desktop_notification_find_program("notify-send", NULL, 0);
}

MOONBIT_FFI_EXPORT int32_t desktop_notification_linux_show(
    int64_t window_handle, moonbit_bytes_t title, moonbit_bytes_t body,
    int32_t level) {
  char notify_send_path[DESKTOP_NOTIFICATION_PATH_BUFFER_SIZE];
  const char *title_text = (const char *)title;
  const char *body_text = (const char *)body;
  const char *urgency_text = "normal";
  char *const argv[] = {
      notify_send_path,
      "--app-name",
      (char *)DESKTOP_NOTIFICATION_APP_NAME,
      "--urgency",
      (char *)urgency_text,
      (char *)title_text,
      (char *)body_text,
      NULL,
  };

  (void)window_handle;

  if (title_text == NULL || title_text[0] == '\0' || body_text == NULL ||
      body_text[0] == '\0') {
    return 0;
  }
  if (!desktop_notification_find_program("notify-send", notify_send_path,
                                         sizeof(notify_send_path))) {
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
}
#endif
