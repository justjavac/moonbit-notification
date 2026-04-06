///|
test "Notification::new preserves explicit fields" {
  let request = @notification.Notification::new(
    "Deployment finished",
    title=Some("CI"),
    level=@notification.Error,
  )

  assert_eq(request.title, Some("CI"))
  assert_eq(request.body, "Deployment finished")
  assert_eq(request.level, @notification.Error)
}

///|
test "show rejects an empty body before calling native code" {
  assert_eq(
    @notification.show("", title=Some("CI")),
    Err("notification body must not be empty"),
  )
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
test "current_platform reports a desktop backend in CI targets" {
  assert_true(@notification.current_platform() != @notification.Unsupported)
}
