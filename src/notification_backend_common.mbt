///|
/// Converts a MoonBit string into a UTF-8 C string for native FFI calls.
fn to_cstr(value : String) -> Bytes {
  let bytes = @utf8.encode(value).to_array()
  bytes.push(0)
  Bytes::from_array(bytes)
}

///|
/// Maps the public severity enum to the native integer level used by the backends.
fn level_to_native_code(level : NotificationLevel) -> Int {
  match level {
    Info => 0
    Warning => 1
    Error => 2
  }
}
