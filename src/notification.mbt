///|
/// Displays a desktop notification using the current platform backend.
///
/// This is the most convenient entry point when you already have the complete
/// notification data in a `Notification` value. The function validates the
/// request, checks runtime support, and dispatches to the platform backend:
///
/// - Windows uses a native shell notification implementation.
/// - macOS uses `/usr/bin/osascript`.
/// - Linux uses `notify-send`.
///
/// The MoonBit layer selects the concrete backend during compilation with
/// `#cfg(platform=...)`.
pub fn show_notification(notification : Notification) -> Result[Unit, String] {
  show_notification_with_window(0, notification)
}

///|
/// Displays a desktop notification built from the provided fields.
///
/// `body` must not be empty. When `title` is omitted, the package falls back
/// to `"Lepus"`. The `level` value influences urgency where the platform
/// offers a matching concept.
pub fn show(
  body : String,
  title? : String? = None,
  level? : NotificationLevel = Info,
) -> Result[Unit, String] {
  show_notification(Notification::new(body, title~, level~))
}

///|
/// Displays a desktop notification while keeping compatibility with APIs that
/// already track a native window handle.
///
/// The `window_handle` is currently used only by the Windows-oriented calling
/// convention inherited from the reference implementation; other platforms
/// ignore it.
pub fn show_with_window(
  window_handle : Int64,
  body : String,
  title? : String? = None,
  level? : NotificationLevel = Info,
) -> Result[Unit, String] {
  show_notification_with_window(
    window_handle,
    Notification::new(body, title~, level~),
  )
}

///|
/// Validates and dispatches a notification with an explicit window handle.
fn show_notification_with_window(
  window_handle : Int64,
  notification : Notification,
) -> Result[Unit, String] {
  match ensure_supported() {
    Err(error) => Err(error)
    Ok(_) =>
      match prepare_notification(notification) {
        Err(error) => Err(error)
        Ok((title, body)) =>
          backend_deliver(window_handle, title, body, notification.level)
      }
  }
}
