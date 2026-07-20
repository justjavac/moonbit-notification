---
status: accepted
---

# Separate app and CLI notification delivery

The library provides distinct App Notification and CLI Notification delivery
paths on macOS while retaining automatic selection for the existing API. An
identified app uses UserNotifications and respects the app's authorization
state; an unbundled command-line process uses `osascript`. Automatic delivery
must not fall back to the CLI path after a user denies an app notification
request, because doing so would bypass the user's application-level choice.

The explicit APIs accept a `Notification` value. The existing field-based and
window-compatible APIs remain automatic so current callers keep their source
compatibility. The library does not replace the host application's notification
center delegate, and therefore foreground presentation remains the host
application's responsibility.
