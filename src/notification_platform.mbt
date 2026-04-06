///|
/// Returns whether the current machine has everything needed to deliver a notification.
#cfg(target="native")
extern "C" fn notification_is_supported_ffi() -> Bool = "desktop_notification_is_supported"

///|
/// Returns the support error chosen by the native backend at compile time.
#cfg(target="native")
extern "C" fn notification_support_error_ffi() -> Bytes = "desktop_notification_support_error"

///|
/// Returns whether the current runtime can deliver a desktop notification.
///
/// The concrete support check is provided by the native backend selected at
/// compile time for the host OS.
pub fn is_supported() -> Bool {
  notification_is_supported_ffi()
}

///|
/// Converts support probing into a `Result`.
///
/// This is helpful when callers want a human-readable failure reason before
/// attempting `show` or `show_notification`.
pub fn ensure_supported() -> Result[Unit, String] {
  if is_supported() {
    Ok(())
  } else {
    Err(@ffi.from_cstr(notification_support_error_ffi()))
  }
}
