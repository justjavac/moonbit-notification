///|
/// Returns whether the current build has a notification backend.
#cfg(not(any(platform="windows", platform="macos", platform="darwin", platform="linux")))
fn backend_is_supported() -> Bool {
  false
}

///|
/// Returns the support error for unsupported native platforms.
#cfg(not(any(platform="windows", platform="macos", platform="darwin", platform="linux")))
fn backend_support_error() -> String {
  "desktop notifications are not supported on this platform"
}

///|
/// Reports unsupported native platforms that do not have a backend.
#cfg(not(any(platform="windows", platform="macos", platform="darwin", platform="linux")))
fn backend_deliver(
  _window_handle : Int64,
  _title : String,
  _body : String,
  _level : NotificationLevel,
) -> Result[Unit, String] {
  Err(backend_support_error())
}
