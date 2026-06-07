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
test "show_notification_with_window succeeds in dry-run mode" {
  notification_set_dry_run_ffi(true)
  let result = show_notification_with_window(
    42,
    Notification::new(
      "Build finished",
      title=Some(default_title()),
      level=Warning,
    ),
  )
  notification_set_dry_run_ffi(false)

  assert_true(result == Ok(()))
}
