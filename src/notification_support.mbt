///|
/// Returns whether the current runtime can deliver a desktop notification.
///
/// The concrete support check is selected at compile time with
/// `#cfg(platform=...)`.
pub fn is_supported() -> Bool {
  backend_is_supported()
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
    Err(backend_support_error())
  }
}
