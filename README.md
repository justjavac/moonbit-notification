# Notification

[![coverage](https://img.shields.io/codecov/c/github/justjavac/moonbit-notification/main?label=coverage)](https://codecov.io/gh/justjavac/moonbit-notification)
[![linux](https://img.shields.io/codecov/c/github/justjavac/moonbit-notification/main?flag=linux&label=linux)](https://codecov.io/gh/justjavac/moonbit-notification)
[![macos](https://img.shields.io/codecov/c/github/justjavac/moonbit-notification/main?flag=macos&label=macos)](https://codecov.io/gh/justjavac/moonbit-notification)
[![windows](https://img.shields.io/codecov/c/github/justjavac/moonbit-notification/main?flag=windows&label=windows)](https://codecov.io/gh/justjavac/moonbit-notification)

`justjavac/notification` is a MoonBit `native` package for sending desktop
notifications on Windows, macOS, and Linux through one small API.

## Supported Platforms

The package is intended for MoonBit `native` targets only.

- Windows: uses a native shell notification implementation through
  `IUserNotification`
- macOS App Notification: uses Apple's UserNotifications framework
- macOS CLI Notification: uses `/usr/bin/osascript`
- Linux: uses `notify-send`
- Other native platforms: compile, but report the platform as unsupported at
  runtime

## Quick Start

The simplest way to send a notification is:

```moonbit
let result = @notification.show(
  "Build finished successfully",
  title=Some("CI"),
)
```

On macOS, the existing entry points choose the delivery path automatically. An
identified `.app` uses UserNotifications; an unbundled command-line process uses
`osascript`. To select a path explicitly, construct a request and call:

```moonbit
let request = @notification.Notification::new(
  "Artifacts uploaded",
  title=Some("Release"),
)

let app_result = @notification.show_app_notification(request)
let cli_result = @notification.show_cli_notification(request)
```

If you want to construct the request first:

```moonbit
let request = @notification.Notification::new(
  "Artifacts uploaded",
  title=Some("Release"),
  level=@notification.Warning,
)

let result = @notification.show_notification(request)
```

For existing Windows-oriented integrations that already keep a native handle:

```moonbit
let result = @notification.show_with_window(
  0,
  "Deployment finished",
  title=Some("Ops"),
  level=@notification.Info,
)
```

## Public API

### Types

`NotificationLevel` exposes three urgency hints:

- `Info`
- `Warning`
- `Error`

`Notification` is the immutable request type used by the public API:

```moonbit
pub(all) struct Notification {
  title : String?
  body : String
  level : NotificationLevel
}
```

### Functions

- `show(body, title?, level?)`
  Sends a notification from raw fields.
- `show_notification(notification)`
  Sends a pre-built `Notification`.
- `show_with_window(window_handle, body, title?, level?)`
  Preserves compatibility with call sites that already track a native window
  handle.
- `show_app_notification(notification)`
  Uses App Notification delivery on macOS and the platform's normal backend on
  Windows and Linux.
- `show_cli_notification(notification)`
  Uses CLI Notification delivery on macOS and the platform's normal backend on
  Windows and Linux.
- `is_supported()`
  Reports whether the current runtime can deliver notifications.
- `ensure_supported()`
  Converts support probing into `Result[Unit, String]`.

## Behavior Notes

The repository intentionally keeps behavior predictable across platforms.

- `body` must not be empty
- `title` is optional
- missing or empty titles fall back to `"Lepus"`
- urgency is mapped from `NotificationLevel` into backend-specific values
- validation happens in MoonBit before native delivery is attempted

Current urgency mapping:

- Windows: information, warning, and error icon/flag hints
- macOS: the level value is accepted by both delivery paths but is not currently
  used for visual differentiation
- Linux: `Info -> low`, `Warning -> normal`, `Error -> critical`

### macOS authorization and foreground behavior

App Notification delivery requires macOS 10.14 or later, a stable bundle
identifier, and an executable hosted inside an `.app` bundle. The first App
Notification call requests alert authorization when needed. A denial is
returned as an error and automatic delivery does not bypass it through
`osascript`.

The library does not replace the host application's
`UNUserNotificationCenterDelegate`. When the app is in the foreground, the host
is responsible for choosing presentation options in its delegate.

## Testing

The repository uses several verification layers:

- unit-style black-box tests for public API behavior
- white-box tests for normalization helpers and urgency mapping
- a dry-run native path to avoid real notification popups during automated runs
- GitHub Actions jobs on `ubuntu-latest`, `macos-latest`, and
  `windows-latest`

Useful commands during development:

```bash
moon info src --target native
moon fmt
moon test --target native
moon test --target native --enable-coverage
moon coverage analyze -p notification
```

To suppress real desktop notifications while testing locally:

```bash
MOONBIT_NOTIFICATION_DRY_RUN=1
```

## License

MIT
