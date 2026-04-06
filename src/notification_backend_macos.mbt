///|
/// Returns whether the macOS backend can launch `osascript`.
#cfg(any(platform="macos", platform="darwin"))
extern "C" fn notification_macos_is_supported_ffi() -> Bool = "desktop_notification_macos_is_supported"

///|
/// Returns whether the current macOS build can attempt to deliver a notification.
#cfg(any(platform="macos", platform="darwin"))
fn backend_is_supported() -> Bool {
  notification_macos_is_supported_ffi()
}

///|
/// Returns the support error for the macOS backend.
#cfg(any(platform="macos", platform="darwin"))
fn backend_support_error() -> String {
  "desktop notifications on macOS require /usr/bin/osascript"
}

///|
/// Invokes the macOS backend selected at compile time.
#cfg(any(platform="macos", platform="darwin"))
#borrow(title, body)
extern "C" fn show_notification_macos_ffi(
  window_handle : Int64,
  title : Bytes,
  body : Bytes,
  level : Int,
) -> Bool = "desktop_notification_macos_show"

///|
/// Executes the macOS backend and normalizes native failures.
#cfg(any(platform="macos", platform="darwin"))
fn backend_deliver(
  window_handle : Int64,
  title : String,
  body : String,
  level : NotificationLevel,
) -> Result[Unit, String] {
  if show_notification_macos_ffi(
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
