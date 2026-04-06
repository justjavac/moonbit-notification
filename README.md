# Notification

[![coverage](https://img.shields.io/codecov/c/github/justjavac/moonbit-notification/main?label=coverage)](https://codecov.io/gh/justjavac/moonbit-notification)
[![linux](https://img.shields.io/codecov/c/github/justjavac/moonbit-notification/main?flag=linux&label=linux)](https://codecov.io/gh/justjavac/moonbit-notification)
[![macos](https://img.shields.io/codecov/c/github/justjavac/moonbit-notification/main?flag=macos&label=macos)](https://codecov.io/gh/justjavac/moonbit-notification)
[![windows](https://img.shields.io/codecov/c/github/justjavac/moonbit-notification/main?flag=windows&label=windows)](https://codecov.io/gh/justjavac/moonbit-notification)

Cross-platform native desktop notifications for MoonBit.

This package provides a small, readable MoonBit API for sending desktop
notifications on all three major desktop platforms:

- Windows uses a native shell notification implementation based on the same
  Windows-first approach as the reference package in
  `D:\Code\moonbit-webview\notification`.
- macOS uses `/usr/bin/osascript`.
- Linux uses `notify-send`.

The public API is intentionally compact so applications can use one code path
for all supported operating systems while still receiving platform-aware
urgency handling.

## Features

- Works with MoonBit `native` targets on Windows, macOS, and Linux
- Keeps the reference package's familiar `show` and `show_with_window` helpers
- Exposes a reusable `Notification` value type for clearer application code
- Validates inputs before crossing the FFI boundary
- Includes black-box tests, white-box tests, and README examples
- Supports a dry-run mode for non-intrusive automated testing

## Installation

Add the dependency in your `moon.mod.json`:

```json
{
  "deps": {
    "justjavac/notification": "0.1.0"
  }
}
```

This module targets `native` only.

## Quick Start

```moonbit
let result = @notification.show(
  "Build finished successfully",
  title=Some("CI"),
)
```

If you want to prepare the request first, use `Notification::new`:

```moonbit
let request = @notification.Notification::new(
  "Artifacts uploaded",
  title=Some("Release"),
  level=@notification.Warning,
)
let result = @notification.show_notification(request)
```

## API Overview

### `NotificationLevel`

`NotificationLevel` describes the urgency of a notification:

- `Info` for ordinary updates
- `Warning` for attention-worthy situations
- `Error` for the strongest urgency supported by the platform

### `Notification`

`Notification` is an immutable request object with three fields:

- `title : String?`
- `body : String`
- `level : NotificationLevel`

When `title` is `None` or an empty string, the package falls back to
`"MoonBit"`. The `body` must be non-empty.

### Core Functions

- `show(body, title?, level?)` is the easiest way to send a notification.
- `show_notification(notification)` sends a pre-built request object.
- `show_with_window(window_handle, body, title?, level?)` keeps compatibility
  with Windows-oriented call sites that already track a window handle.
- `current_platform()` reports which backend is active.
- `is_supported()` and `ensure_supported()` let you probe the runtime before
  delivery.

## Platform Notes

### Windows

The Windows backend follows the native implementation style from the reference
package and maps `NotificationLevel` to Windows information, warning, and error
presentation hints.

### macOS

The macOS backend shells out to `/usr/bin/osascript`, which is available on a
standard macOS installation.

### Linux

The Linux backend calls `notify-send`. On Debian/Ubuntu based systems, install
it with:

```bash
sudo apt-get update
sudo apt-get install -y libnotify-bin
```

## Testing

The repository includes three layers of verification:

- black-box API tests in `src/notification_test.mbt`
- white-box helper tests in `src/notification_wbtest.mbt`
- executable package examples in `src/README.mbt.md`

To run tests without showing real desktop notifications, either rely on the
built-in white-box dry-run hook or set:

```bash
MOONBIT_NOTIFICATION_DRY_RUN=1
```

Then run:

```bash
moon test
moon coverage analyze -p src
```

## Development Notes

- Source files live in `src` as requested.
- The project is licensed under MIT.
- `moon info`, `moon fmt`, and `moon test` are part of the validation flow.
