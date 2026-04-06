///|
/// Invokes the Windows native backend with UTF-16 arguments.
#borrow(title, body)
extern "C" fn show_notification_windows_ffi(
  window_handle : Int64,
  title : Bytes,
  body : Bytes,
  level : Int,
) -> Bool = "desktop_notification_show_windows"

///|
/// Invokes the macOS backend with UTF-8 arguments.
#borrow(title, body)
extern "C" fn show_notification_macos_ffi(title : Bytes, body : Bytes) -> Bool = "desktop_notification_show_macos"

///|
/// Invokes the Linux backend with UTF-8 arguments.
#borrow(title, body, urgency)
extern "C" fn show_notification_linux_ffi(
  title : Bytes,
  body : Bytes,
  urgency : Bytes,
) -> Bool = "desktop_notification_show_linux"

///|
/// Converts a MoonBit string into a UTF-16 buffer for Windows FFI calls.
fn to_wstr(value : String) -> Bytes {
  @ffi.to_wstr(value)
}

///|
/// Converts a MoonBit string into a UTF-8 C string for POSIX FFI calls.
fn to_cstr(value : String) -> Bytes {
  @ffi.to_cstr(value)
}

///|
/// Executes the platform backend and normalizes native failures.
fn deliver(
  platform : NotificationPlatform,
  window_handle : Int64,
  title : String,
  body : String,
  level : NotificationLevel,
) -> Result[Unit, String] {
  match platform {
    Windows =>
      if show_notification_windows_ffi(
          window_handle,
          to_wstr(title),
          to_wstr(body),
          level_to_native_code(level),
        ) {
        Ok(())
      } else {
        Err("native Windows notification delivery failed")
      }
    MacOS =>
      if show_notification_macos_ffi(to_cstr(title), to_cstr(body)) {
        Ok(())
      } else {
        Err("native macOS notification delivery failed")
      }
    Linux =>
      if show_notification_linux_ffi(
          to_cstr(title),
          to_cstr(body),
          to_cstr(level_to_linux_urgency(level)),
        ) {
        Ok(())
      } else {
        Err("native Linux notification delivery failed")
      }
    Unsupported =>
      Err("desktop notifications are not supported on this platform")
  }
}
