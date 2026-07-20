///|
/// Forces the native layer into dry-run mode for tests.
extern "C" fn notification_set_dry_run_ffi(enabled : Bool) -> Unit = "desktop_notification_set_dry_run"

///|
test "private helpers normalize titles and urgency levels" {
  assert_eq(default_title(), "Lepus")
  assert_eq(resolve_title(None), "Lepus")
  assert_eq(resolve_title(Some("")), "Lepus")
  assert_eq(resolve_title(Some("Build")), "Build")
  assert_eq(level_to_native_code(Info), 0)
  assert_eq(level_to_native_code(Warning), 1)
  assert_eq(level_to_native_code(Error), 2)
}

///|
test "prepare_notification validates body and fills the default title" {
  let valid_result = prepare_notification(
    Notification::new("Artifacts uploaded"),
  )
  assert_true(valid_result == Ok((default_title(), "Artifacts uploaded")))
  let invalid_result = prepare_notification(
    Notification::new("", title=Some("CI")),
  )
  assert_true(invalid_result == Err("notification body must not be empty"))
}

///|
test "all delivery paths succeed in dry-run mode" {
  notification_set_dry_run_ffi(true)
  let automatic_result = show_with_window(
    42,
    "Build finished",
    title=Some(default_title()),
    level=Warning,
  )
  let app_result = show_app_notification(
    Notification::new("App build finished", title=Some(default_title())),
  )
  let cli_result = show_cli_notification(
    Notification::new("CLI build finished", title=Some(default_title())),
  )
  notification_set_dry_run_ffi(false)

  assert_true(automatic_result == Ok(()))
  assert_true(app_result == Ok(()))
  assert_true(cli_result == Ok(()))
}

///|
test "native delivery status maps to stable errors" {
  assert_eq(
    delivery_error(delivery_status_app_identity_required),
    "App Notifications on macOS require an identified .app bundle",
  )
  assert_eq(
    delivery_error(delivery_status_authorization_denied),
    "App Notification authorization was denied",
  )
  assert_eq(
    delivery_error(delivery_status_scheduling_failed),
    "macOS failed to schedule the App Notification",
  )
  assert_eq(
    delivery_error(delivery_status_authorization_failed),
    "App Notification authorization could not be determined",
  )
  assert_eq(
    delivery_error(delivery_status_app_backend_unavailable),
    "App Notifications require UserNotifications on macOS 10.14 or later",
  )
  assert_eq(
    delivery_error(delivery_status_delivery_failed),
    "native notification delivery failed",
  )
  assert_eq(
    delivery_error(delivery_status_unsupported),
    "desktop notifications are not supported on this platform",
  )
}
