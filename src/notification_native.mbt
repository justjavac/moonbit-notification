///|
/// Invokes the platform-specific native backend selected at compile time.
#cfg(target="native")
#borrow(title, body)
extern "C" fn show_notification_ffi(
  window_handle : Int64,
  title : Bytes,
  body : Bytes,
  level : Int,
) -> Bool = "desktop_notification_show"

///|
/// Converts a MoonBit string into a UTF-8 C string for native FFI calls.
fn to_cstr(value : String) -> Bytes {
  @ffi.to_cstr(value)
}

///|
/// Executes the platform backend and normalizes native failures.
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
