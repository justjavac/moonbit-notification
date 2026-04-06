///|
/// Returns whether the Linux backend can launch `notify-send`.
#cfg(platform="linux")
extern "C" fn notification_linux_is_supported_ffi() -> Bool = "desktop_notification_linux_is_supported"

///|
/// Returns whether the current Linux build can attempt to deliver a notification.
#cfg(platform="linux")
fn backend_is_supported() -> Bool {
  notification_linux_is_supported_ffi()
}

///|
/// Returns the support error for the Linux backend.
#cfg(platform="linux")
fn backend_support_error() -> String {
  "desktop notifications on Linux require the notify-send executable"
}

///|
/// Invokes the Linux backend selected at compile time.
#cfg(platform="linux")
#borrow(title, body)
extern "C" fn show_notification_linux_ffi(
  window_handle : Int64,
  title : Bytes,
  body : Bytes,
  level : Int,
) -> Bool = "desktop_notification_linux_show"

///|
/// Executes the Linux backend and normalizes native failures.
#cfg(platform="linux")
fn backend_deliver(
  window_handle : Int64,
  title : String,
  body : String,
  level : NotificationLevel,
) -> Result[Unit, String] {
  if show_notification_linux_ffi(
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
