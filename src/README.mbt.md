# justjavac/notification

Cross-platform desktop notifications for MoonBit native targets.

```mbt check
///|
test "create a notification request" {
  let request = @notification.Notification::new(
    "Build finished",
    title=Some("CI"),
  )

  assert_eq(request.title, Some("CI"))
  assert_eq(request.body, "Build finished")
  assert_eq(request.level, @notification.Info)
}
```

```mbt check
///|
test "empty notification bodies are rejected" {
  assert_eq(
    @notification.show("", title=Some("CI")),
    Err("notification body must not be empty"),
  )
}
```
