///|
/// Returns whether the Windows build can attempt to deliver a notification.
#cfg(platform="windows")
fn backend_is_supported() -> Bool {
  true
}

///|
/// Returns the support error for the Windows backend.
#cfg(platform="windows")
fn backend_support_error() -> String {
  "desktop notifications are unavailable on the current Windows runtime"
}

///|
/// Invokes the Windows backend selected at compile time.
#cfg(platform="windows")
#borrow(title, body)
extern "C" fn show_notification_windows_ffi(
  window_handle : Int64,
  title : Bytes,
  body : Bytes,
  level : Int,
) -> Bool = "desktop_notification_windows_show"

///|
/// Executes the Windows backend and normalizes native failures.
#cfg(platform="windows")
fn backend_deliver(
  window_handle : Int64,
  title : String,
  body : String,
  level : NotificationLevel,
) -> Result[Unit, String] {
  if show_notification_windows_ffi(
      window_handle,
      to_cstr(title),
      to_cstr(body),
      level_to_native_code(level),
    ) {
    Ok(())
  } else {
    Err("native notification delivery failed")
  }
}
