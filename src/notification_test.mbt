///|
test "Notification::new preserves explicit fields" {
  let request = @notification.Notification::new(
    "Deployment finished",
    title=Some("CI"),
    level=Error,
  )

  assert_true(request.title == Some("CI"))
  assert_eq(request.body, "Deployment finished")
  assert_true(request.level == Error)
}

///|
test "show rejects an empty body before calling native code" {
  let result = @notification.show("", title=Some("CI"))
  assert_true(result == Err("notification body must not be empty"))
}

///|
test "ensure_supported stays in sync with is_supported" {
  let supported = @notification.is_supported()
  let result = @notification.ensure_supported()

  match result {
    Ok(_) => assert_true(supported)
    Err(_) => assert_true(!supported)
  }
}

///|
test "explicit delivery modes validate before entering native code" {
  let request = @notification.Notification::new("")
  assert_true(
    @notification.show_notification(request, delivery=App) ==
    Err("notification body must not be empty"),
  )
  assert_true(
    @notification.show_notification(request, delivery=Cli) ==
    Err("notification body must not be empty"),
  )
}
