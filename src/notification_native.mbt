///|
/// Invokes the Windows backend selected at compile time.
#cfg(platform="windows")
#borrow(title, body)
extern "C" fn show_notification_ffi(
  window_handle : Int64,
  title : Bytes,
  body : Bytes,
  level : Int,
) -> Bool = "desktop_notification_windows_show"

///|
/// Invokes the macOS backend selected at compile time.
#cfg(platform="macos")
#borrow(title, body)
extern "C" fn show_notification_ffi(
  window_handle : Int64,
  title : Bytes,
  body : Bytes,
  level : Int,
) -> Bool = "desktop_notification_macos_show"

///|
/// Invokes the Linux backend selected at compile time.
#cfg(platform="linux")
#borrow(title, body)
extern "C" fn show_notification_ffi(
  window_handle : Int64,
  title : Bytes,
  body : Bytes,
  level : Int,
) -> Bool = "desktop_notification_linux_show"

///|
/// Converts a MoonBit string into a UTF-8 C string for native FFI calls.
fn to_cstr(value : String) -> Bytes {
  @ffi.to_cstr(value)
}

///|
/// Executes the selected compile-time backend and normalizes native failures.
#cfg(any(platform="windows", platform="macos", platform="linux"))
fn deliver(
  window_handle : Int64,
  title : String,
  body : String,
  level : NotificationLevel,
) -> Result[Unit, String] {
  if show_notification_ffi(
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

///|
/// Reports unsupported native platforms that do not have a backend.
#cfg(not(any(platform="windows", platform="macos", platform="linux")))
fn deliver(
  _window_handle : Int64,
  _title : String,
  _body : String,
  _level : NotificationLevel,
) -> Result[Unit, String] {
  Err("desktop notifications are not supported on this platform")
}
