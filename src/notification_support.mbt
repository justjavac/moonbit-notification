///|
/// Returns whether the current runtime can deliver a desktop notification.
///
/// The concrete support check is selected at compile time with
/// `#cfg(platform=...)` and may still depend on runtime prerequisites such as
/// system executables or platform services being available.
///
/// This function is a lightweight probe that does not attempt to show a
/// notification. It returns `true` only when the active backend believes a
/// delivery attempt can be made right now.
///
/// On macOS the probe follows the same automatic selection as `show`: an
/// identified app checks its UserNotifications authorization state, while an
/// unbundled command-line process checks for `/usr/bin/osascript`. A user who
/// has denied App Notification authorization makes this function return `false`;
/// a not-yet-determined authorization still returns `true` because `show` may
/// request it.
///
/// # Example
/// ```mbt check
/// test "is_supported probes capability without delivering" {
///   let _ : Bool = is_supported()
/// }
/// ```
pub fn is_supported() -> Bool {
  backend_is_supported()
}

///|
/// Converts support probing into a `Result`.
///
/// This is helpful when callers want a human-readable failure reason before
/// attempting `show` or `show_notification`.
///
/// Returns `Ok(())` when `is_supported()` is `true`. Otherwise it returns
/// `Err(message)` where `message` describes the missing platform capability,
/// such as an unavailable runtime service or required executable.
///
/// # Example
/// ```mbt check
/// test "ensure_supported reports a reason when unavailable" {
///   match ensure_supported() {
///     Ok(_) => ()
///     Err(message) => assert_true(!message.is_empty())
///   }
/// }
/// ```
pub fn ensure_supported() -> Result[Unit, String] {
  if is_supported() {
    Ok(())
  } else {
    Err(backend_support_error())
  }
}
