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
/// Native backend identifier for the Windows shell notification path.
let backend_kind_windows : Int = 1

///|
/// Native backend identifier for the macOS `osascript` path.
let backend_kind_macos : Int = 2

///|
/// Native backend identifier for the Linux `notify-send` path.
let backend_kind_linux : Int = 3

///|
/// Status returned across the native FFI seam.
///
/// The variants are ordered to match `DESKTOP_NOTIFICATION_STATUS_*` in
/// `notification_native.h`.
priv enum DeliveryStatus {
  Success = 0
  Unsupported
  BackendUnavailable
  AppIdentityRequired
  AuthorizationDenied
  AuthorizationFailed
  SchedulingFailed
  DeliveryFailed
  AppBackendUnavailable
}

///|
/// Returns the backend selected by the native build.
extern "C" fn notification_backend_kind_ffi() -> Int = "desktop_notification_backend_kind"

///|
/// Returns whether the current runtime can deliver notifications.
extern "C" fn notification_support_status_ffi() -> DeliveryStatus = "desktop_notification_support_status"

///|
/// Invokes the native backend selected at compile time.
#borrow(title, body)
extern "C" fn show_notification_ffi(
  window_handle : Int64,
  title : Bytes,
  body : Bytes,
  level : Int,
) -> DeliveryStatus = "desktop_notification_show"

///|
/// Invokes the app-oriented native backend.
#borrow(title, body)
extern "C" fn show_app_notification_ffi(
  window_handle : Int64,
  title : Bytes,
  body : Bytes,
  level : Int,
) -> DeliveryStatus = "desktop_notification_show_app"

///|
/// Invokes the command-line-oriented native backend.
#borrow(title, body)
extern "C" fn show_cli_notification_ffi(
  window_handle : Int64,
  title : Bytes,
  body : Bytes,
  level : Int,
) -> DeliveryStatus = "desktop_notification_show_cli"

///|
/// Returns whether the current build/runtime can attempt to deliver a notification.
fn backend_is_supported() -> Bool {
  notification_support_status_ffi() is Success
}

///|
/// Returns the support error for the active native backend.
fn backend_support_error() -> String {
  delivery_error(notification_support_status_ffi())
}

///|
/// Maps native delivery status into stable public error text.
fn delivery_error(status : DeliveryStatus) -> String {
  let backend_kind = notification_backend_kind_ffi()
  match status {
    BackendUnavailable =>
      if backend_kind == backend_kind_windows {
        "desktop notifications are unavailable on the current Windows runtime"
      } else if backend_kind == backend_kind_macos {
        "CLI notifications on macOS require /usr/bin/osascript"
      } else if backend_kind == backend_kind_linux {
        "desktop notifications on Linux require the notify-send executable"
      } else {
        "desktop notifications are not supported on this platform"
      }
    AppIdentityRequired =>
      "App Notifications on macOS require an identified .app bundle"
    AuthorizationDenied => "App Notification authorization was denied"
    AuthorizationFailed =>
      "App Notification authorization could not be determined"
    SchedulingFailed => "macOS failed to schedule the App Notification"
    AppBackendUnavailable =>
      "App Notifications require UserNotifications on macOS 10.14 or later"
    Unsupported => "desktop notifications are not supported on this platform"
    Success | DeliveryFailed => "native notification delivery failed"
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
  let status = show_notification_ffi(
    window_handle,
    to_cstr(title),
    to_cstr(body),
    level_to_native_code(level),
  )
  match status {
    Success => Ok(())
    _ => Err(delivery_error(status))
  }
}

///|
/// Executes the app-oriented backend and normalizes native failures.
fn backend_deliver_app(
  window_handle : Int64,
  title : String,
  body : String,
  level : NotificationLevel,
) -> Result[Unit, String] {
  let status = show_app_notification_ffi(
    window_handle,
    to_cstr(title),
    to_cstr(body),
    level_to_native_code(level),
  )
  match status {
    Success => Ok(())
    _ => Err(delivery_error(status))
  }
}

///|
/// Executes the command-line-oriented backend and normalizes native failures.
fn backend_deliver_cli(
  window_handle : Int64,
  title : String,
  body : String,
  level : NotificationLevel,
) -> Result[Unit, String] {
  let status = show_cli_notification_ffi(
    window_handle,
    to_cstr(title),
    to_cstr(body),
    level_to_native_code(level),
  )
  match status {
    Success => Ok(())
    _ => Err(delivery_error(status))
  }
}

///|
/// Dispatches through the delivery path selected by the caller.
fn backend_deliver_with_mode(
  window_handle : Int64,
  title : String,
  body : String,
  level : NotificationLevel,
  delivery : NotificationDelivery,
) -> Result[Unit, String] {
  match delivery {
    Auto => backend_deliver(window_handle, title, body, level)
    App => backend_deliver_app(window_handle, title, body, level)
    Cli => backend_deliver_cli(window_handle, title, body, level)
  }
}
