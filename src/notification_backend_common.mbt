///|
/// Converts a MoonBit string into a UTF-8 C string for native FFI calls.
fn to_cstr(value : String) -> Bytes {
  let bytes = @utf8.encode(value).to_array()
  bytes.push(0)
  Bytes::from_array(bytes)
}

///|
/// Maps the public severity enum to the native integer level used by the backends.
fn level_to_native_code(level : NotificationLevel) -> Int {
  match level {
    Info => 0
    Warning => 1
    Error => 2
  }
}

///|
let backend_kind_windows : Int = 1

///|
let backend_kind_macos : Int = 2

///|
let backend_kind_linux : Int = 3

///|
/// Returns the backend selected by the native build.
extern "C" fn notification_backend_kind_ffi() -> Int = "desktop_notification_backend_kind"

///|
/// Returns whether the current runtime can deliver notifications.
extern "C" fn notification_is_supported_ffi() -> Bool = "desktop_notification_is_supported"

///|
/// Invokes the native backend selected at compile time.
#borrow(title, body)
extern "C" fn show_notification_ffi(
  window_handle : Int64,
  title : Bytes,
  body : Bytes,
  level : Int,
) -> Bool = "desktop_notification_show"

///|
/// Returns whether the current build/runtime can attempt to deliver a notification.
fn backend_is_supported() -> Bool {
  notification_is_supported_ffi()
}

///|
/// Returns the support error for the active native backend.
fn backend_support_error() -> String {
  let backend_kind = notification_backend_kind_ffi()
  if backend_kind == backend_kind_windows {
    "desktop notifications are unavailable on the current Windows runtime"
  } else if backend_kind == backend_kind_macos {
    "desktop notifications on macOS require /usr/bin/osascript"
  } else if backend_kind == backend_kind_linux {
    "desktop notifications on Linux require the notify-send executable"
  } else {
    "desktop notifications are not supported on this platform"
  }
}

///|
/// Executes the selected native backend and normalizes native failures.
fn backend_deliver(
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
