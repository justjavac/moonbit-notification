///|
/// Returns whether the macOS backend can launch `osascript`.
#cfg(platform="macos")
extern "C" fn notification_is_supported_ffi() -> Bool = "desktop_notification_macos_is_supported"

///|
/// Returns whether the Linux backend can launch `notify-send`.
#cfg(platform="linux")
extern "C" fn notification_is_supported_ffi() -> Bool = "desktop_notification_linux_is_supported"

///|
/// Returns whether the Windows build can attempt to deliver a notification.
#cfg(platform="windows")
fn current_platform_is_supported() -> Bool {
  true
}

///|
/// Returns whether the current macOS build can attempt to deliver a notification.
#cfg(platform="macos")
fn current_platform_is_supported() -> Bool {
  notification_is_supported_ffi()
}

///|
/// Returns whether the current Linux build can attempt to deliver a notification.
#cfg(platform="linux")
fn current_platform_is_supported() -> Bool {
  notification_is_supported_ffi()
}

///|
/// Returns whether the current build has a notification backend.
#cfg(not(any(platform="windows", platform="macos", platform="linux")))
fn current_platform_is_supported() -> Bool {
  false
}

///|
/// Returns the support error message for the selected compile-time backend.
#cfg(platform="windows")
fn support_error_message() -> String {
  "desktop notifications are unavailable on the current Windows runtime"
}

///|
/// Returns the support error message for the selected compile-time backend.
#cfg(platform="macos")
fn support_error_message() -> String {
  "desktop notifications on macOS require /usr/bin/osascript"
}

///|
/// Returns the support error message for the selected compile-time backend.
#cfg(platform="linux")
fn support_error_message() -> String {
  "desktop notifications on Linux require the notify-send executable"
}

///|
/// Returns the support error message for unsupported native platforms.
#cfg(not(any(platform="windows", platform="macos", platform="linux")))
fn support_error_message() -> String {
  "desktop notifications are not supported on this platform"
}

///|
/// Returns whether the current runtime can deliver a desktop notification.
///
/// The concrete support check is selected at compile time with
/// `#cfg(platform=...)`.
pub fn is_supported() -> Bool {
  current_platform_is_supported()
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
    Err(support_error_message())
  }
}
