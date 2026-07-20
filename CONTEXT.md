# Desktop Notifications

This context distinguishes notifications by the identity and runtime environment
from which they are delivered.

## Language

**App Notification**:
A desktop notification attributed to an identified application and governed by
that application's notification authorization.
_Avoid_: Native notification, bundled notification

**CLI Notification**:
A desktop notification emitted on behalf of an unbundled command-line process
that has no stable application identity.
_Avoid_: Script notification, fallback notification
