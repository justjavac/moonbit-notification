#include "notification_native.h"

#ifdef __APPLE__
#include <dlfcn.h>
#include <pthread.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>

typedef void *desktop_notification_id;
typedef void *desktop_notification_class;
typedef void *desktop_notification_selector;
typedef int8_t desktop_notification_bool;
typedef long desktop_notification_integer;
typedef unsigned long desktop_notification_options;

typedef desktop_notification_class (*desktop_notification_get_class_fn)(
    const char *name);
typedef desktop_notification_selector (
    *desktop_notification_register_selector_fn)(const char *name);

struct desktop_notification_block_descriptor {
  uintptr_t reserved;
  uintptr_t size;
};

struct desktop_notification_block {
  void *isa;
  int32_t flags;
  int32_t reserved;
  void (*invoke)(void);
  const struct desktop_notification_block_descriptor *descriptor;
};

struct desktop_notification_callback_state {
  pthread_mutex_t mutex;
  pthread_cond_t condition;
  int completed;
  int32_t value;
};

#define DESKTOP_NOTIFICATION_BLOCK_IS_GLOBAL (1 << 28)
#define DESKTOP_NOTIFICATION_AUTHORIZATION_NOT_DETERMINED 0
#define DESKTOP_NOTIFICATION_AUTHORIZATION_DENIED 1
#define DESKTOP_NOTIFICATION_AUTHORIZATION_AUTHORIZED 2
#define DESKTOP_NOTIFICATION_AUTHORIZATION_PROVISIONAL 3
#define DESKTOP_NOTIFICATION_AUTHORIZATION_OPTION_ALERT (1UL << 2)

static pthread_once_t desktop_notification_runtime_once = PTHREAD_ONCE_INIT;
static pthread_mutex_t desktop_notification_operation_mutex =
    PTHREAD_MUTEX_INITIALIZER;
static struct desktop_notification_callback_state
    desktop_notification_callback_state = {
        PTHREAD_MUTEX_INITIALIZER,
        PTHREAD_COND_INITIALIZER,
        0,
        0,
};
static desktop_notification_get_class_fn desktop_notification_get_class = NULL;
static desktop_notification_register_selector_fn
    desktop_notification_register_selector = NULL;
static void *desktop_notification_message_send = NULL;
static void *desktop_notification_global_block_class = NULL;
static int desktop_notification_runtime_available = 0;

static const struct desktop_notification_block_descriptor
    desktop_notification_settings_descriptor = {
        0,
        sizeof(struct desktop_notification_block),
};
static const struct desktop_notification_block_descriptor
    desktop_notification_authorization_descriptor = {
        0,
        sizeof(struct desktop_notification_block),
};
static const struct desktop_notification_block_descriptor
    desktop_notification_schedule_descriptor = {
        0,
        sizeof(struct desktop_notification_block),
};

static desktop_notification_id
desktop_notification_send_id(desktop_notification_id receiver,
                             const char *selector_name) {
  desktop_notification_selector selector =
      desktop_notification_register_selector(selector_name);
  return ((desktop_notification_id (*)(desktop_notification_id,
                                       desktop_notification_selector))
              desktop_notification_message_send)(receiver, selector);
}

static desktop_notification_id desktop_notification_send_id_id_id_id(
    desktop_notification_id receiver, const char *selector_name,
    desktop_notification_id first, desktop_notification_id second,
    desktop_notification_id third) {
  desktop_notification_selector selector =
      desktop_notification_register_selector(selector_name);
  return ((desktop_notification_id (*)(
      desktop_notification_id, desktop_notification_selector,
      desktop_notification_id, desktop_notification_id,
      desktop_notification_id))desktop_notification_message_send)(
      receiver, selector, first, second, third);
}

static void
desktop_notification_send_void_id(desktop_notification_id receiver,
                                  const char *selector_name,
                                  desktop_notification_id argument) {
  desktop_notification_selector selector =
      desktop_notification_register_selector(selector_name);
  ((void (*)(desktop_notification_id, desktop_notification_selector,
             desktop_notification_id))desktop_notification_message_send)(
      receiver, selector, argument);
}

static void
desktop_notification_send_void_block(desktop_notification_id receiver,
                                     const char *selector_name,
                                     struct desktop_notification_block *block) {
  desktop_notification_selector selector =
      desktop_notification_register_selector(selector_name);
  ((void (*)(desktop_notification_id, desktop_notification_selector,
             struct desktop_notification_block *))
       desktop_notification_message_send)(receiver, selector, block);
}

static void desktop_notification_send_void_options_block(
    desktop_notification_id receiver, const char *selector_name,
    desktop_notification_options options,
    struct desktop_notification_block *block) {
  desktop_notification_selector selector =
      desktop_notification_register_selector(selector_name);
  ((void (*)(desktop_notification_id, desktop_notification_selector,
             desktop_notification_options, struct desktop_notification_block *))
       desktop_notification_message_send)(receiver, selector, options, block);
}

static void desktop_notification_send_void_id_block(
    desktop_notification_id receiver, const char *selector_name,
    desktop_notification_id argument,
    struct desktop_notification_block *block) {
  desktop_notification_selector selector =
      desktop_notification_register_selector(selector_name);
  ((void (*)(desktop_notification_id, desktop_notification_selector,
             desktop_notification_id, struct desktop_notification_block *))
       desktop_notification_message_send)(receiver, selector, argument, block);
}

static desktop_notification_integer
desktop_notification_send_integer(desktop_notification_id receiver,
                                  const char *selector_name) {
  desktop_notification_selector selector =
      desktop_notification_register_selector(selector_name);
  return ((desktop_notification_integer (*)(desktop_notification_id,
                                            desktop_notification_selector))
              desktop_notification_message_send)(receiver, selector);
}

static const char *
desktop_notification_send_c_string(desktop_notification_id receiver,
                                   const char *selector_name) {
  desktop_notification_selector selector =
      desktop_notification_register_selector(selector_name);
  return (
      (const char *(*)(desktop_notification_id, desktop_notification_selector))
          desktop_notification_message_send)(receiver, selector);
}

static void desktop_notification_callback_begin(void) {
  pthread_mutex_lock(&desktop_notification_callback_state.mutex);
  desktop_notification_callback_state.completed = 0;
  desktop_notification_callback_state.value = 0;
  pthread_mutex_unlock(&desktop_notification_callback_state.mutex);
}

static void desktop_notification_callback_complete(int32_t value) {
  pthread_mutex_lock(&desktop_notification_callback_state.mutex);
  desktop_notification_callback_state.value = value;
  desktop_notification_callback_state.completed = 1;
  pthread_cond_signal(&desktop_notification_callback_state.condition);
  pthread_mutex_unlock(&desktop_notification_callback_state.mutex);
}

static int32_t desktop_notification_callback_wait(void) {
  int32_t value = 0;
  pthread_mutex_lock(&desktop_notification_callback_state.mutex);
  while (!desktop_notification_callback_state.completed) {
    pthread_cond_wait(&desktop_notification_callback_state.condition,
                      &desktop_notification_callback_state.mutex);
  }
  value = desktop_notification_callback_state.value;
  pthread_mutex_unlock(&desktop_notification_callback_state.mutex);
  return value;
}

static void
desktop_notification_settings_callback(struct desktop_notification_block *block,
                                       desktop_notification_id settings) {
  (void)block;
  desktop_notification_callback_complete(
      (int32_t)desktop_notification_send_integer(settings,
                                                 "authorizationStatus"));
}

static void desktop_notification_authorization_callback(
    struct desktop_notification_block *block, desktop_notification_bool granted,
    desktop_notification_id error) {
  (void)block;
  if (error != NULL) {
    desktop_notification_callback_complete(
        DESKTOP_NOTIFICATION_STATUS_AUTHORIZATION_FAILED);
  } else if (granted) {
    desktop_notification_callback_complete(DESKTOP_NOTIFICATION_STATUS_OK);
  } else {
    desktop_notification_callback_complete(
        DESKTOP_NOTIFICATION_STATUS_AUTHORIZATION_DENIED);
  }
}

static void
desktop_notification_schedule_callback(struct desktop_notification_block *block,
                                       desktop_notification_id error) {
  (void)block;
  desktop_notification_callback_complete(
      error == NULL ? DESKTOP_NOTIFICATION_STATUS_OK
                    : DESKTOP_NOTIFICATION_STATUS_SCHEDULING_FAILED);
}

static struct desktop_notification_block desktop_notification_settings_block = {
    NULL,
    DESKTOP_NOTIFICATION_BLOCK_IS_GLOBAL,
    0,
    (void (*)(void))desktop_notification_settings_callback,
    &desktop_notification_settings_descriptor,
};
static struct desktop_notification_block
    desktop_notification_authorization_block = {
        NULL,
        DESKTOP_NOTIFICATION_BLOCK_IS_GLOBAL,
        0,
        (void (*)(void))desktop_notification_authorization_callback,
        &desktop_notification_authorization_descriptor,
};
static struct desktop_notification_block desktop_notification_schedule_block = {
    NULL,
    DESKTOP_NOTIFICATION_BLOCK_IS_GLOBAL,
    0,
    (void (*)(void))desktop_notification_schedule_callback,
    &desktop_notification_schedule_descriptor,
};

static void desktop_notification_load_runtime(void) {
  void *foundation =
      dlopen("/System/Library/Frameworks/Foundation.framework/Foundation",
             RTLD_LAZY | RTLD_LOCAL);
  void *user_notifications =
      dlopen("/System/Library/Frameworks/UserNotifications.framework/"
             "UserNotifications",
             RTLD_LAZY | RTLD_LOCAL);

  if (foundation == NULL || user_notifications == NULL) {
    return;
  }

  desktop_notification_get_class =
      (desktop_notification_get_class_fn)dlsym(RTLD_DEFAULT, "objc_getClass");
  desktop_notification_register_selector =
      (desktop_notification_register_selector_fn)dlsym(RTLD_DEFAULT,
                                                       "sel_registerName");
  desktop_notification_message_send = dlsym(RTLD_DEFAULT, "objc_msgSend");
  desktop_notification_global_block_class =
      dlsym(RTLD_DEFAULT, "_NSConcreteGlobalBlock");
  if (desktop_notification_get_class == NULL ||
      desktop_notification_register_selector == NULL ||
      desktop_notification_message_send == NULL ||
      desktop_notification_global_block_class == NULL ||
      desktop_notification_get_class("UNUserNotificationCenter") == NULL) {
    return;
  }

  desktop_notification_settings_block.isa =
      desktop_notification_global_block_class;
  desktop_notification_authorization_block.isa =
      desktop_notification_global_block_class;
  desktop_notification_schedule_block.isa =
      desktop_notification_global_block_class;
  desktop_notification_runtime_available = 1;
}

static int desktop_notification_user_notifications_available(void) {
  pthread_once(&desktop_notification_runtime_once,
               desktop_notification_load_runtime);
  return desktop_notification_runtime_available;
}

static desktop_notification_id desktop_notification_autorelease_pool_new(void) {
  desktop_notification_class pool_class =
      desktop_notification_get_class("NSAutoreleasePool");
  return desktop_notification_send_id(pool_class, "new");
}

static void
desktop_notification_autorelease_pool_drain(desktop_notification_id pool) {
  if (pool != NULL) {
    desktop_notification_send_id(pool, "drain");
  }
}

static desktop_notification_id desktop_notification_string(const char *value) {
  desktop_notification_class string_class =
      desktop_notification_get_class("NSString");
  desktop_notification_selector selector =
      desktop_notification_register_selector("stringWithUTF8String:");
  return ((desktop_notification_id (*)(
      desktop_notification_id, desktop_notification_selector,
      const char *))desktop_notification_message_send)(string_class, selector,
                                                       value);
}

static int desktop_notification_has_app_identity(void) {
  desktop_notification_id pool = NULL;
  desktop_notification_id bundle = NULL;
  desktop_notification_id bundle_identifier = NULL;
  desktop_notification_id bundle_url = NULL;
  desktop_notification_id path_extension = NULL;
  const char *identifier_text = NULL;
  const char *extension_text = NULL;
  int has_identity = 0;

  if (!desktop_notification_user_notifications_available()) {
    return 0;
  }

  pool = desktop_notification_autorelease_pool_new();
  bundle = desktop_notification_send_id(
      desktop_notification_get_class("NSBundle"), "mainBundle");
  bundle_identifier = desktop_notification_send_id(bundle, "bundleIdentifier");
  bundle_url = desktop_notification_send_id(bundle, "bundleURL");
  path_extension = desktop_notification_send_id(bundle_url, "pathExtension");
  if (bundle_identifier != NULL && path_extension != NULL) {
    identifier_text =
        desktop_notification_send_c_string(bundle_identifier, "UTF8String");
    extension_text =
        desktop_notification_send_c_string(path_extension, "UTF8String");
    has_identity = identifier_text != NULL && identifier_text[0] != '\0' &&
                   extension_text != NULL && strcmp(extension_text, "app") == 0;
  }
  desktop_notification_autorelease_pool_drain(pool);
  return has_identity;
}

static desktop_notification_id desktop_notification_center(void) {
  return desktop_notification_send_id(
      desktop_notification_get_class("UNUserNotificationCenter"),
      "currentNotificationCenter");
}

static int32_t desktop_notification_authorization_status(void) {
  desktop_notification_id center = desktop_notification_center();
  if (center == NULL) {
    return -1;
  }
  desktop_notification_callback_begin();
  desktop_notification_send_void_block(
      center, "getNotificationSettingsWithCompletionHandler:",
      &desktop_notification_settings_block);
  return desktop_notification_callback_wait();
}

static int32_t desktop_notification_ensure_authorized(void) {
  desktop_notification_id center = desktop_notification_center();
  int32_t authorization_status = desktop_notification_authorization_status();

  if (authorization_status == DESKTOP_NOTIFICATION_AUTHORIZATION_AUTHORIZED ||
      authorization_status == DESKTOP_NOTIFICATION_AUTHORIZATION_PROVISIONAL) {
    return DESKTOP_NOTIFICATION_STATUS_OK;
  }
  if (authorization_status == DESKTOP_NOTIFICATION_AUTHORIZATION_DENIED) {
    return DESKTOP_NOTIFICATION_STATUS_AUTHORIZATION_DENIED;
  }
  if (authorization_status !=
          DESKTOP_NOTIFICATION_AUTHORIZATION_NOT_DETERMINED ||
      center == NULL) {
    return DESKTOP_NOTIFICATION_STATUS_AUTHORIZATION_FAILED;
  }

  desktop_notification_callback_begin();
  desktop_notification_send_void_options_block(
      center, "requestAuthorizationWithOptions:completionHandler:",
      DESKTOP_NOTIFICATION_AUTHORIZATION_OPTION_ALERT,
      &desktop_notification_authorization_block);
  return desktop_notification_callback_wait();
}

static int32_t
desktop_notification_schedule_app_notification(const char *title,
                                               const char *body) {
  desktop_notification_id pool = desktop_notification_autorelease_pool_new();
  desktop_notification_id content = NULL;
  desktop_notification_id identifier = NULL;
  desktop_notification_id request = NULL;
  desktop_notification_id center = desktop_notification_center();
  int32_t status = DESKTOP_NOTIFICATION_STATUS_SCHEDULING_FAILED;

  content = desktop_notification_send_id(
      desktop_notification_get_class("UNMutableNotificationContent"), "new");
  if (content == NULL || center == NULL) {
    goto cleanup;
  }
  desktop_notification_send_void_id(
      content, "setTitle:", desktop_notification_string(title));
  desktop_notification_send_void_id(
      content, "setBody:", desktop_notification_string(body));
  identifier = desktop_notification_send_id(
      desktop_notification_send_id(desktop_notification_get_class("NSUUID"),
                                   "UUID"),
      "UUIDString");
  request = desktop_notification_send_id_id_id_id(
      desktop_notification_get_class("UNNotificationRequest"),
      "requestWithIdentifier:content:trigger:", identifier, content, NULL);
  if (request == NULL) {
    goto cleanup;
  }

  desktop_notification_callback_begin();
  desktop_notification_send_void_id_block(
      center, "addNotificationRequest:withCompletionHandler:", request,
      &desktop_notification_schedule_block);
  status = desktop_notification_callback_wait();

cleanup:
  if (content != NULL) {
    desktop_notification_send_id(content, "release");
  }
  desktop_notification_autorelease_pool_drain(pool);
  return status;
}

/* Returns whether automatic macOS delivery can currently be attempted. */
int32_t desktop_notification_macos_support_status(void) {
  desktop_notification_id pool = NULL;
  int32_t authorization_status = 0;

  if (desktop_notification_has_app_identity()) {
    pthread_mutex_lock(&desktop_notification_operation_mutex);
    pool = desktop_notification_autorelease_pool_new();
    authorization_status = desktop_notification_authorization_status();
    desktop_notification_autorelease_pool_drain(pool);
    pthread_mutex_unlock(&desktop_notification_operation_mutex);
    if (authorization_status == DESKTOP_NOTIFICATION_AUTHORIZATION_DENIED) {
      return DESKTOP_NOTIFICATION_STATUS_AUTHORIZATION_DENIED;
    }
    return authorization_status >= 0
               ? DESKTOP_NOTIFICATION_STATUS_OK
               : DESKTOP_NOTIFICATION_STATUS_AUTHORIZATION_FAILED;
  }
  return access("/usr/bin/osascript", X_OK) == 0
             ? DESKTOP_NOTIFICATION_STATUS_OK
             : DESKTOP_NOTIFICATION_STATUS_BACKEND_UNAVAILABLE;
}

/* Returns whether automatic macOS delivery can currently be attempted. */
MOONBIT_FFI_EXPORT int32_t desktop_notification_macos_is_supported(void) {
  return desktop_notification_macos_support_status() ==
         DESKTOP_NOTIFICATION_STATUS_OK;
}

/* Sends through Apple UserNotifications for an identified application. */
int32_t desktop_notification_macos_show_app(int64_t window_handle,
                                            moonbit_bytes_t title,
                                            moonbit_bytes_t body,
                                            int32_t level) {
  const char *title_text = (const char *)title;
  const char *body_text = (const char *)body;
  desktop_notification_id pool = NULL;
  int32_t status = DESKTOP_NOTIFICATION_STATUS_OK;

  (void)window_handle;
  (void)level;

  if (!desktop_notification_payload_is_valid(title_text, body_text)) {
    return DESKTOP_NOTIFICATION_STATUS_DELIVERY_FAILED;
  }
  if (desktop_notification_dry_run_enabled()) {
    return DESKTOP_NOTIFICATION_STATUS_OK;
  }
  if (!desktop_notification_user_notifications_available()) {
    return DESKTOP_NOTIFICATION_STATUS_APP_BACKEND_UNAVAILABLE;
  }
  if (!desktop_notification_has_app_identity()) {
    return DESKTOP_NOTIFICATION_STATUS_APP_IDENTITY_REQUIRED;
  }

  pthread_mutex_lock(&desktop_notification_operation_mutex);
  pool = desktop_notification_autorelease_pool_new();
  status = desktop_notification_ensure_authorized();
  if (status == DESKTOP_NOTIFICATION_STATUS_OK) {
    status =
        desktop_notification_schedule_app_notification(title_text, body_text);
  }
  desktop_notification_autorelease_pool_drain(pool);
  pthread_mutex_unlock(&desktop_notification_operation_mutex);
  return status;
}

/* Sends through osascript for an unbundled command-line process. */
int32_t desktop_notification_macos_show_cli(int64_t window_handle,
                                            moonbit_bytes_t title,
                                            moonbit_bytes_t body,
                                            int32_t level) {
  const char *title_text = (const char *)title;
  const char *body_text = (const char *)body;
  char *const argv[] = {
      "/usr/bin/osascript",
      "-e",
      "on run argv",
      "-e",
      "display notification (item 1 of argv) with title (item 2 of argv)",
      "-e",
      "end run",
      (char *)body_text,
      (char *)title_text,
      NULL,
  };

  (void)window_handle;
  (void)level;

  if (!desktop_notification_payload_is_valid(title_text, body_text)) {
    return DESKTOP_NOTIFICATION_STATUS_DELIVERY_FAILED;
  }
  if (desktop_notification_dry_run_enabled()) {
    return DESKTOP_NOTIFICATION_STATUS_OK;
  }
  if (access("/usr/bin/osascript", X_OK) != 0) {
    return DESKTOP_NOTIFICATION_STATUS_BACKEND_UNAVAILABLE;
  }
  return desktop_notification_run_process(argv)
             ? DESKTOP_NOTIFICATION_STATUS_OK
             : DESKTOP_NOTIFICATION_STATUS_DELIVERY_FAILED;
}

/* Selects App Notification or CLI Notification from the host identity. */
MOONBIT_FFI_EXPORT int32_t
desktop_notification_macos_show(int64_t window_handle, moonbit_bytes_t title,
                                moonbit_bytes_t body, int32_t level) {
  if (desktop_notification_has_app_identity()) {
    return desktop_notification_macos_show_app(window_handle, title, body,
                                               level);
  }
  return desktop_notification_macos_show_cli(window_handle, title, body, level);
}
#endif
