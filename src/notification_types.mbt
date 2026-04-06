///|
/// Severity hint passed to the native notification backend.
///
/// The package keeps the set intentionally small so the same API remains easy
/// to use across Windows, macOS, and Linux:
///
/// - `Info` is the default level for routine updates.
/// - `Warning` requests a more attention-grabbing style when the platform
///   supports it.
/// - `Error` requests the strongest available emphasis.
pub(all) enum NotificationLevel {
  Info
  Warning
  Error
} derive(Show, Eq)

///|
/// Immutable request object used by `show_notification`.
///
/// Keeping the request as a value makes it easy to validate, reuse, and test.
/// `title` is optional; when omitted, the package falls back to `"Lepus"`.
/// `body` must be non-empty.
pub(all) struct Notification {
  title : String?
  body : String
  level : NotificationLevel
} derive(Show, Eq)

///|
/// Builds a notification value with the same defaults used by the convenience
/// `show` helper.
pub fn Notification::new(
  body : String,
  title? : String? = None,
  level? : NotificationLevel = Info,
) -> Notification {
  { title, body, level }
}
