#include "notification_native.h"

#include <stdlib.h>
#include <string.h>

#if defined(__APPLE__) || defined(__linux__)
#include <spawn.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
extern char **environ;
#endif

#ifdef __linux__
#include <stdio.h>
#include <unistd.h>
#endif

static int32_t desktop_notification_force_dry_run = 0;

int desktop_notification_dry_run_enabled(void) {
  const char *value = getenv("MOONBIT_NOTIFICATION_DRY_RUN");
  if (desktop_notification_force_dry_run) {
    return 1;
  }
  if (value == NULL) {
    return 0;
  }
  return strcmp(value, "1") == 0 || strcmp(value, "true") == 0 ||
         strcmp(value, "TRUE") == 0;
}

MOONBIT_FFI_EXPORT void desktop_notification_set_dry_run(int32_t enabled) {
  desktop_notification_force_dry_run = enabled != 0;
}

#if defined(__APPLE__) || defined(__linux__)
int desktop_notification_run_process(char *const argv[]) {
  pid_t pid = 0;
  int status = 0;

  if (posix_spawn(&pid, argv[0], NULL, NULL, argv, environ) != 0) {
    return 0;
  }
  if (waitpid(pid, &status, 0) < 0) {
    return 0;
  }
  return WIFEXITED(status) && WEXITSTATUS(status) == 0;
}
#endif

#ifdef __linux__
static int desktop_notification_copy_path(char *buffer, size_t buffer_size,
                                          const char *directory,
                                          const char *program) {
  int written = snprintf(buffer, buffer_size, "%s/%s", directory, program);
  return written > 0 && (size_t)written < buffer_size;
}

int desktop_notification_find_program(const char *program, char *buffer,
                                      size_t buffer_size) {
  const char *path_env = getenv("PATH");
  const char *fallback_dirs[] = {"/usr/bin", "/bin", "/usr/local/bin", NULL};
  size_t path_length = 0;

  if (path_env != NULL) {
    char *paths = NULL;
    char *cursor = NULL;
    char *segment = NULL;

    path_length = strlen(path_env);
    paths = (char *)malloc(path_length + 1);
    if (paths != NULL) {
      memcpy(paths, path_env, path_length + 1);
      cursor = paths;
      while ((segment = strtok(cursor, ":")) != NULL) {
        char candidate[DESKTOP_NOTIFICATION_PATH_BUFFER_SIZE];
        cursor = NULL;
        if (desktop_notification_copy_path(candidate, sizeof(candidate), segment,
                                           program) &&
            access(candidate, X_OK) == 0) {
          if (buffer != NULL && buffer_size > 0) {
            memcpy(buffer, candidate, strlen(candidate) + 1);
          }
          free(paths);
          return 1;
        }
      }
      free(paths);
    }
  }

  for (size_t index = 0; fallback_dirs[index] != NULL; index += 1) {
    char candidate[DESKTOP_NOTIFICATION_PATH_BUFFER_SIZE];
    if (desktop_notification_copy_path(candidate, sizeof(candidate),
                                       fallback_dirs[index], program) &&
        access(candidate, X_OK) == 0) {
      if (buffer != NULL && buffer_size > 0) {
        memcpy(buffer, candidate, strlen(candidate) + 1);
      }
      return 1;
    }
  }

  return 0;
}
#endif
