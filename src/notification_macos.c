#include "notification_native.h"

#ifdef __APPLE__
#include <unistd.h>

/*
 * Returns whether the macOS backend can invoke `/usr/bin/osascript`.
 */
MOONBIT_FFI_EXPORT int32_t desktop_notification_macos_is_supported(void) {
  return access("/usr/bin/osascript", X_OK) == 0;
}

/*
 * Sends a notification through AppleScript using `/usr/bin/osascript`.
 *
 * `window_handle` and `level` are accepted for cross-platform API
 * compatibility, but the current macOS implementation does not use them.
 */
MOONBIT_FFI_EXPORT int32_t desktop_notification_macos_show(
    int64_t window_handle, moonbit_bytes_t title, moonbit_bytes_t body,
    int32_t level) {
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
  (void)level;

  if (!desktop_notification_payload_is_valid(title_text, body_text)) {
    return 0;
  }
  if (desktop_notification_dry_run_enabled()) {
    return 1;
  }

  return desktop_notification_run_process(argv);
}
#endif
