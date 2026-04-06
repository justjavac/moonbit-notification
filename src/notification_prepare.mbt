///|
let default_notification_title : String = "Lepus"

///|
/// Returns the fallback title used when callers omit one.
fn default_title() -> String {
  default_notification_title
}

///|
/// Replaces a missing or empty title with the package default.
fn resolve_title(title : String?) -> String {
  match title {
    Some(title) if !title.is_empty() => title
    _ => default_title()
  }
}

///|
/// Validates a request and returns the normalized title/body pair.
fn prepare_notification(
  notification : Notification,
) -> Result[(String, String), String] {
  if notification.body.is_empty() {
    Err("notification body must not be empty")
  } else {
    Ok((resolve_title(notification.title), notification.body))
  }
}
