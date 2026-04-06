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
  char *argv[8];

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

  argv[0] = notify_send_path;
  argv[1] = "--app-name";
  argv[2] = (char *)DESKTOP_NOTIFICATION_APP_NAME;
  argv[3] = "--urgency";
  argv[4] = (char *)urgency_text;
  argv[5] = (char *)title_text;
  argv[6] = (char *)body_text;
  argv[7] = NULL;

  if (desktop_notification_dry_run_enabled()) {
    return 1;
  }

  return desktop_notification_run_process(argv);
}
#endif
