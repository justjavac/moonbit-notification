///|
/// Displays a desktop notification using the current platform backend.
///
/// This is the most direct entry point when the caller already has a complete
/// `Notification` value. The function performs the full delivery pipeline in
/// order:
///
/// - It verifies that desktop notifications are available in the current
///   runtime environment.
/// - It normalizes the request by rejecting an empty body and filling in the
///   default title when one is missing or empty.
/// - It dispatches the normalized request to the platform backend:
///
/// - Windows uses a native shell notification implementation.
/// - macOS uses UserNotifications for an identified app and `/usr/bin/osascript`
///   for an unbundled command-line process.
/// - Linux uses `notify-send`.
///
/// The shared native dispatcher selects the concrete platform backend during
/// compilation. `delivery` defaults to `Auto`; on macOS that mode selects the
/// delivery path at runtime from the host application identity. `App` forces
/// UserNotifications, while `Cli` forces `/usr/bin/osascript`.
///
/// Returns `Ok(())` when the backend reports a successful delivery attempt.
/// Returns `Err(...)` when notifications are unsupported on the current
/// runtime, when the request body is empty, or when the native backend reports
/// a failure.
///
/// # Example
/// ```mbt nocheck
/// let request = @notification.Notification::new(
///   "Build finished",
///   title=Some("CI"),
///   level=@notification.NotificationLevel::Info,
/// )
/// let _ = @notification.show_notification(request)
/// ```
pub fn show_notification(
  notification : Notification,
  delivery? : NotificationDelivery = Auto,
) -> Result[Unit, String] {
  show_notification_with_window(0, notification, delivery)
}

///|
/// Displays a desktop notification built from the provided fields.
///
/// This convenience wrapper builds a `Notification` value and then delegates to
/// `show_notification`, so it follows the same validation and delivery rules.
///
/// `body` must not be empty. When `title` is omitted or provided as an empty
/// string, the package falls back to `"Lepus"`. The `level` value is treated
/// as a cross-platform severity hint and is mapped to the closest urgency or
/// emphasis level supported by the active backend. `delivery` defaults to
/// `Auto` and may be set to `App` or `Cli` to select a path explicitly.
///
/// Returns `Ok(())` when the notification is accepted by the native backend.
/// Returns `Err(...)` with the same failure reasons as `show_notification`.
///
/// # Example
/// ```mbt nocheck
/// // Title defaults to the package name; level defaults to `Info`.
/// let _ = @notification.show("Download complete")
///
/// // Or provide a title and raise the severity.
/// let _ = @notification.show(
///   "Disk almost full",
///   title=Some("Storage"),
///   level=@notification.NotificationLevel::Warning,
/// )
/// ```
pub fn show(
  body : String,
  title? : String? = None,
  level? : NotificationLevel = Info,
  delivery? : NotificationDelivery = Auto,
) -> Result[Unit, String] {
  show_notification(Notification::new(body, title~, level~), delivery~)
}

///|
/// Displays a desktop notification while keeping compatibility with APIs that
/// already track a native window handle.
///
/// The `window_handle` is currently used only by the Windows-oriented calling
/// convention inherited from the reference implementation; other platforms
/// ignore it.
///
/// Apart from accepting the extra handle, this function behaves the same as
/// `show`: it rejects an empty body, applies the default title when needed,
/// checks runtime support, and forwards the request to the selected backend.
///
/// Returns `Ok(())` on successful delivery and `Err(...)` when validation,
/// support checks, or backend execution fails.
///
/// # Example
/// ```mbt nocheck
/// // `window_handle` is forwarded to the Windows backend and ignored elsewhere.
/// let _ = @notification.show_with_window(0L, "Render finished")
/// ```
pub fn show_with_window(
  window_handle : Int64,
  body : String,
  title? : String? = None,
  level? : NotificationLevel = Info,
  delivery? : NotificationDelivery = Auto,
) -> Result[Unit, String] {
  show_notification_with_window(
    window_handle,
    Notification::new(body, title~, level~),
    delivery,
  )
}

///|
/// Validates and dispatches a notification with an explicit window handle.
fn show_notification_with_window(
  window_handle : Int64,
  notification : Notification,
  delivery : NotificationDelivery,
) -> Result[Unit, String] {
  if delivery == Auto && !backend_is_supported() {
    Err(backend_support_error())
  } else {
    match prepare_notification(notification) {
      Err(error) => Err(error)
      Ok((title, body)) =>
        backend_deliver_with_mode(
          window_handle,
          title,
          body,
          notification.level,
          delivery,
        )
    }
  }
}
