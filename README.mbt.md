# justjavac/notification

[![coverage](https://img.shields.io/codecov/c/github/justjavac/moonbit-notification/main?label=coverage)](https://codecov.io/gh/justjavac/moonbit-notification)
[![linux](https://img.shields.io/codecov/c/github/justjavac/moonbit-notification/main?flag=linux&label=linux)](https://codecov.io/gh/justjavac/moonbit-notification)
[![macos](https://img.shields.io/codecov/c/github/justjavac/moonbit-notification/main?flag=macos&label=macos)](https://codecov.io/gh/justjavac/moonbit-notification)
[![windows](https://img.shields.io/codecov/c/github/justjavac/moonbit-notification/main?flag=windows&label=windows)](https://codecov.io/gh/justjavac/moonbit-notification)

Cross-platform desktop notifications for MoonBit `native` targets.

- One small API for Windows, macOS, and Linux
- Public entry points: `show`, `show_notification`, `show_with_window`
- Support checks: `is_supported()` and `ensure_supported()`

## Example

```mbt check
///|
test "send a notification request" {
  let result = @notification.show("Build finished", title=Some("CI"))

  ignore(result)
}
```

## Request Type

```mbt check
///|
test "build a notification value" {
  let request = @notification.Notification::new(
    "Artifacts uploaded",
    title=Some("Release"),
    level=@notification.Warning,
  )

  assert_eq(request.body, "Artifacts uploaded")
}
```

## Notes

- `body` must not be empty
- missing or empty titles fall back to `"Lepus"`
- Windows uses a native shell implementation
- macOS uses `/usr/bin/osascript`
- Linux uses `notify-send`
