///|
/// Returns the raw platform code reported by the native stub.
extern "C" fn notification_platform_ffi() -> Int = "desktop_notification_platform"

///|
/// Returns whether the current machine has everything needed to deliver a notification.
extern "C" fn notification_is_supported_ffi() -> Bool = "desktop_notification_is_supported"

///|
/// Converts a native platform code into the public enum.
fn platform_from_code(code : Int) -> NotificationPlatform {
  match code {
    1 => Windows
    2 => MacOS
    3 => Linux
    _ => Unsupported
  }
}

///|
/// Explains why notification delivery is unavailable on the given platform.
fn support_error(platform : NotificationPlatform) -> String {
  match platform {
    Windows =>
      "desktop notifications are unavailable on the current Windows runtime"
    MacOS => "desktop notifications on macOS require /usr/bin/osascript"
    Linux => "desktop notifications on Linux require the notify-send executable"
    Unsupported => "desktop notifications are not supported on this platform"
  }
}

///|
/// Reports which notification backend the current process will use.
///
/// This function is useful when applications want to log or branch on the
/// active desktop backend without attempting delivery yet.
pub fn current_platform() -> NotificationPlatform {
  platform_from_code(notification_platform_ffi())
}

///|
/// Returns whether the current runtime can deliver a desktop notification.
///
/// On Linux this also checks for the `notify-send` executable; on macOS it
/// checks for `/usr/bin/osascript`.
pub fn is_supported() -> Bool {
  notification_is_supported_ffi()
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
    Err(support_error(current_platform()))
  }
}
