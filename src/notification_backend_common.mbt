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
/// Native delivery completed successfully.
let delivery_status_ok : Int = 0

///|
/// The current platform has no notification backend.
let delivery_status_unsupported : Int = 1

///|
/// The selected platform backend is unavailable at runtime.
let delivery_status_backend_unavailable : Int = 2

///|
/// App Notification delivery requires an identified application bundle.
let delivery_status_app_identity_required : Int = 3

///|
/// The user denied application notification authorization.
let delivery_status_authorization_denied : Int = 4

///|
/// Application notification authorization could not be determined.
let delivery_status_authorization_failed : Int = 5

///|
/// UserNotifications rejected the notification request.
let delivery_status_scheduling_failed : Int = 6

///|
/// The selected native delivery operation failed.
let delivery_status_delivery_failed : Int = 7

///|
/// UserNotifications is unavailable on the current macOS runtime.
let delivery_status_app_backend_unavailable : Int = 8

///|
/// Returns the backend selected by the native build.
extern "C" fn notification_backend_kind_ffi() -> Int = "desktop_notification_backend_kind"

///|
/// Returns whether the current runtime can deliver notifications.
extern "C" fn notification_support_status_ffi() -> Int = "desktop_notification_support_status"

///|
/// Invokes the native backend selected at compile time.
#borrow(title, body)
extern "C" fn show_notification_ffi(
  window_handle : Int64,
  title : Bytes,
  body : Bytes,
  level : Int,
) -> Int = "desktop_notification_show"

///|
/// Invokes the app-oriented native backend.
#borrow(title, body)
extern "C" fn show_app_notification_ffi(
  window_handle : Int64,
  title : Bytes,
  body : Bytes,
  level : Int,
) -> Int = "desktop_notification_show_app"

///|
/// Invokes the command-line-oriented native backend.
#borrow(title, body)
extern "C" fn show_cli_notification_ffi(
  window_handle : Int64,
  title : Bytes,
  body : Bytes,
  level : Int,
) -> Int = "desktop_notification_show_cli"

///|
/// Returns whether the current build/runtime can attempt to deliver a notification.
fn backend_is_supported() -> Bool {
  notification_support_status_ffi() == delivery_status_ok
}

///|
/// Returns the support error for the active native backend.
fn backend_support_error() -> String {
  delivery_error(notification_support_status_ffi())
}

///|
/// Maps native delivery status into stable public error text.
fn delivery_error(status : Int) -> String {
  let backend_kind = notification_backend_kind_ffi()
  if status == delivery_status_backend_unavailable {
    if backend_kind == backend_kind_windows {
      "desktop notifications are unavailable on the current Windows runtime"
    } else if backend_kind == backend_kind_macos {
      "CLI notifications on macOS require /usr/bin/osascript"
    } else if backend_kind == backend_kind_linux {
      "desktop notifications on Linux require the notify-send executable"
    } else {
      "desktop notifications are not supported on this platform"
    }
  } else if status == delivery_status_app_identity_required {
    "App Notifications on macOS require an identified .app bundle"
  } else if status == delivery_status_authorization_denied {
    "App Notification authorization was denied"
  } else if status == delivery_status_authorization_failed {
    "App Notification authorization could not be determined"
  } else if status == delivery_status_scheduling_failed {
    "macOS failed to schedule the App Notification"
  } else if status == delivery_status_app_backend_unavailable {
    "App Notifications require UserNotifications on macOS 10.14 or later"
  } else if status == delivery_status_delivery_failed {
    "native notification delivery failed"
  } else if status == delivery_status_unsupported {
    "desktop notifications are not supported on this platform"
  } else {
    "native notification delivery failed"
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
  if status == delivery_status_ok {
    Ok(())
  } else {
    Err(delivery_error(status))
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
  if status == delivery_status_ok {
    Ok(())
  } else {
    Err(delivery_error(status))
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
  if status == delivery_status_ok {
    Ok(())
  } else {
    Err(delivery_error(status))
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
