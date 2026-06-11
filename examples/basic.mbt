fn main {
  if @notification.is_supported() {
    ignore(@notification.show("Hello from MoonBit", title=Some("MoonBit")))
  }
}
