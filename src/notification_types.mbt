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
///
/// This constructor does not perform validation; it simply packages the caller
/// input so it can be passed around, reused, or tested before delivery.
/// Validation happens later in `show`, `show_with_window`, and
/// `show_notification`.
///
/// `body` is stored exactly as provided and is expected to contain the
/// user-facing message text. `title` may be omitted when the caller wants the
/// runtime delivery path to inject the package default title. `level` defaults
/// to `Info`.
///
/// `title` may be omitted or set to `Some("")`, in which case the runtime
/// delivery path later replaces it with the default application name.
///
/// Returns a reusable `Notification` value that can be passed to
/// `show_notification`, cached for later delivery, or inspected in tests.
pub fn Notification::new(
  body : String,
  title? : String? = None,
  level? : NotificationLevel = Info,
) -> Notification {
  { title, body, level }
}
