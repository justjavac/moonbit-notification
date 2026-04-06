///|
/// Returns the fallback title used when callers omit one.
fn default_title() -> String {
  "MoonBit"
}

///|
/// Ensures the notification body contains user-facing text.
fn ensure_body(body : String) -> Result[String, String] {
  if body.is_empty() {
    Err("notification body must not be empty")
  } else {
    Ok(body)
  }
}

///|
/// Replaces a missing or empty title with the package default.
fn resolve_title(title : String?) -> String {
  match title {
    Some(title) => if title.is_empty() { default_title() } else { title }
    None => default_title()
  }
}

///|
/// Maps the public severity enum to the Windows native integer level.
fn level_to_native_code(level : NotificationLevel) -> Int {
  match level {
    Info => 0
    Warning => 1
    Error => 2
  }
}

///|
/// Validates a request and returns the normalized title/body pair.
fn prepare_notification(
  notification : Notification,
) -> Result[(String, String), String] {
  match ensure_body(notification.body) {
    Err(error) => Err(error)
    Ok(body) => Ok((resolve_title(notification.title), body))
  }
}
