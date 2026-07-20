# justjavac/notification

[![coverage](https://img.shields.io/codecov/c/github/justjavac/moonbit-notification/main?label=coverage)](https://codecov.io/gh/justjavac/moonbit-notification)
[![linux](https://img.shields.io/codecov/c/github/justjavac/moonbit-notification/main?flag=linux&label=linux)](https://codecov.io/gh/justjavac/moonbit-notification)
[![macos](https://img.shields.io/codecov/c/github/justjavac/moonbit-notification/main?flag=macos&label=macos)](https://codecov.io/gh/justjavac/moonbit-notification)
[![windows](https://img.shields.io/codecov/c/github/justjavac/moonbit-notification/main?flag=windows&label=windows)](https://codecov.io/gh/justjavac/moonbit-notification)

Cross-platform desktop notifications for MoonBit `native` targets.

- One small API for Windows, macOS, and Linux
- Public entry points: `show`, `show_notification`, `show_with_window`
- Explicit delivery: `show_app_notification`, `show_cli_notification`
- Support checks: `is_supported()` and `ensure_supported()`

## Example

```mbt nocheck
let result = @notification.show("Build finished", title=Some("CI"))

ignore(result)
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
- macOS App Notifications use UserNotifications and require an identified `.app`
- macOS CLI Notifications use `/usr/bin/osascript`
- automatic macOS delivery selects between those paths without bypassing an
  authorization denial
- Linux uses `notify-send`
