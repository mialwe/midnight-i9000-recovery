/*
 * Copyright (C) 2007 The Android Open Source Project
 * Copyright (c) 2010, Code Aurora Forum. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <limits.h>
#include <linux/input.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/reboot.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>

#include "bootloader.h"
#include "common.h"
#include "cutils/properties.h"
#include "install.h"
#include "minui/minui.h"
#include "minzip/DirUtil.h"
#include "roots.h"
#include "mounts.h"
#include "recovery_ui.h"
#include "encryptedfs_provisioning.h"

#include "extendedcommands.h"
#include "flashutils/flashutils.h"
#include "midnight.h"

static const struct option OPTIONS[] = {
  { "send_intent", required_argument, NULL, 's' },
  { "update_package", required_argument, NULL, 'u' },
  { "wipe_data", no_argument, NULL, 'w' },
  { "wipe_cache", no_argument, NULL, 'c' },
  { "set_encrypted_filesystems", required_argument, NULL, 'e' },
  { "show_text", no_argument, NULL, 't' },
  { NULL, 0, NULL, 0 },
};

static const char *COMMAND_FILE = "/cache/recovery/command";
static const char *INTENT_FILE = "/cache/recovery/intent";
static const char *LOG_FILE = "/cache/recovery/log";
static const char *LAST_LOG_FILE = "/cache/recovery/last_log";
static const char *SDCARD_ROOT = "/sdcard";
static int allow_display_toggle = 1;
static int poweroff = 0;
static const char *SDCARD_PACKAGE_FILE = "/sdcard/update.zip";
static const char *TEMPORARY_LOG_FILE = "/tmp/recovery.log";
static const char *SIDELOAD_TEMP_DIR = "/tmp/sideload";

/*
 * The recovery tool communicates with the main system through /cache files.
 *   /cache/recovery/command - INPUT - command line for tool, one arg per line
 *   /cache/recovery/log - OUTPUT - combined log file from recovery run(s)
 *   /cache/recovery/intent - OUTPUT - intent that was passed in
 *
 * The arguments which may be supplied in the recovery.command file:
 *   --send_intent=anystring - write the text out to recovery.intent
 *   --update_package=path - verify install an OTA package file
 *   --wipe_data - erase user data (and cache), then reboot
 *   --wipe_cache - wipe cache (but not user data), then reboot
 *   --set_encrypted_filesystem=on|off - enables / diasables encrypted fs
 *
 * After completing, we remove /cache/recovery/command and reboot.
 * Arguments may also be supplied in the bootloader control block (BCB).
 * These important scenarios must be safely restartable at any point:
 *
 * FACTORY RESET
 * 1. user selects "factory reset"
 * 2. main system writes "--wipe_data" to /cache/recovery/command
 * 3. main system reboots into recovery
 * 4. get_args() writes BCB with "boot-recovery" and "--wipe_data"
 *    -- after this, rebooting will restart the erase --
 * 5. erase_volume() reformats /data
 * 6. erase_volume() reformats /cache
 * 7. finish_recovery() erases BCB
 *    -- after this, rebooting will restart the main system --
 * 8. main() calls reboot() to boot main system
 *
 * OTA INSTALL
 * 1. main system downloads OTA package to /cache/some-filename.zip
 * 2. main system writes "--update_package=/cache/some-filename.zip"
 * 3. main system reboots into recovery
 * 4. get_args() writes BCB with "boot-recovery" and "--update_package=..."
 *    -- after this, rebooting will attempt to reinstall the update --
 * 5. install_package() attempts to install the update
 *    NOTE: the package install must itself be restartable from any point
 * 6. finish_recovery() erases BCB
 *    -- after this, rebooting will (try to) restart the main system --
 * 7. ** if install failed **
 *    7a. prompt_and_wait() shows an error icon and waits for the user
 *    7b; the user reboots (pulling the battery, etc) into the main system
 * 8. main() calls maybe_install_firmware_update()
 *    ** if the update contained radio/hboot firmware **:
 *    8a. m_i_f_u() writes BCB with "boot-recovery" and "--wipe_cache"
 *        -- after this, rebooting will reformat cache & restart main system --
 *    8b. m_i_f_u() writes firmware image into raw cache partition
 *    8c. m_i_f_u() writes BCB with "update-radio/hboot" and "--wipe_cache"
 *        -- after this, rebooting will attempt to reinstall firmware --
 *    8d. bootloader tries to flash firmware
 *    8e. bootloader writes BCB with "boot-recovery" (keeping "--wipe_cache")
 *        -- after this, rebooting will reformat cache & restart main system --
 *    8f. erase_volume() reformats /cache
 *    8g. finish_recovery() erases BCB
 *        -- after this, rebooting will (try to) restart the main system --
 * 9. main() calls reboot() to boot main system
 *
 * SECURE FILE SYSTEMS ENABLE/DISABLE
 * 1. user selects "enable encrypted file systems"
 * 2. main system writes "--set_encrypted_filesystems=on|off" to
 *    /cache/recovery/command
 * 3. main system reboots into recovery
 * 4. get_args() writes BCB with "boot-recovery" and
 *    "--set_encrypted_filesystems=on|off"
 *    -- after this, rebooting will restart the transition --
 * 5. read_encrypted_fs_info() retrieves encrypted file systems settings from /data
 *    Settings include: property to specify the Encrypted FS istatus and
 *    FS encryption key if enabled (not yet implemented)
 * 6. erase_volume() reformats /data
 * 7. erase_volume() reformats /cache
 * 8. restore_encrypted_fs_info() writes required encrypted file systems settings to /data
 *    Settings include: property to specify the Encrypted FS status and
 *    FS encryption key if enabled (not yet implemented)
 * 9. finish_recovery() erases BCB
 *    -- after this, rebooting will restart the main system --
 * 10. main() calls reboot() to boot main system
 */

static const int MAX_ARG_LENGTH = 4096;
static const int MAX_ARGS = 100;

// open a given path, mounting partitions as necessary
static FILE*
fopen_path(const char *path, const char *mode) {
    if (ensure_path_mounted(path) != 0) {
        LOGE("Can't mount %s\n", path);
        return NULL;
    }

    // When writing, try to create the containing directory, if necessary.
    // Use generous permissions, the system (init.rc) will reset them.
    if (strchr("wa", mode[0])) dirCreateHierarchy(path, 0777, NULL, 1);

    FILE *fp = fopen(path, mode);
    if (fp == NULL && path != COMMAND_FILE) LOGE("Can't open %s\n", path);
    return fp;
}

// close a file, log an error if the error indicator is set
static void
check_and_fclose(FILE *fp, const char *name) {
    fflush(fp);
    if (ferror(fp)) LOGE("Error in %s\n(%s)\n", name, strerror(errno));
    fclose(fp);
}

// command line args come from, in decreasing precedence:
//   - the actual command line
//   - the bootloader control block (one per line, after "recovery")
//   - the contents of COMMAND_FILE (one per line)
static void
get_args(int *argc, char ***argv) {
    struct bootloader_message boot;
    memset(&boot, 0, sizeof(boot));
    if (device_flash_type() == MTD) {
        get_bootloader_message(&boot);  // this may fail, leaving a zeroed structure
    }

    if (boot.command[0] != 0 && boot.command[0] != 255) {
        LOGI("Boot command: %.*s\n", sizeof(boot.command), boot.command);
    }

    if (boot.status[0] != 0 && boot.status[0] != 255) {
        LOGI("Boot status: %.*s\n", sizeof(boot.status), boot.status);
    }

    struct stat file_info;

    // --- if arguments weren't supplied, look in the bootloader control block
    if (*argc <= 1 && 0 != stat("/tmp/.ignorebootmessage", &file_info)) {
        boot.recovery[sizeof(boot.recovery) - 1] = '\0';  // Ensure termination
        const char *arg = strtok(boot.recovery, "\n");
        if (arg != NULL && !strcmp(arg, "recovery")) {
            *argv = (char **) malloc(sizeof(char *) * MAX_ARGS);
            (*argv)[0] = strdup(arg);
            for (*argc = 1; *argc < MAX_ARGS; ++*argc) {
                if ((arg = strtok(NULL, "\n")) == NULL) break;
                (*argv)[*argc] = strdup(arg);
            }
            LOGI("Got arguments from boot message\n");
        } else if (boot.recovery[0] != 0 && boot.recovery[0] != 255) {
            LOGE("Bad boot message\n\"%.20s\"\n", boot.recovery);
        }
    }

    // --- if that doesn't work, try the command file
    if (*argc <= 1) {
        FILE *fp = fopen_path(COMMAND_FILE, "r");
        if (fp != NULL) {
            char *argv0 = (*argv)[0];
            *argv = (char **) malloc(sizeof(char *) * MAX_ARGS);
            (*argv)[0] = argv0;  // use the same program name

            char buf[MAX_ARG_LENGTH];
            for (*argc = 1; *argc < MAX_ARGS; ++*argc) {
                if (!fgets(buf, sizeof(buf), fp)) break;
                (*argv)[*argc] = strdup(strtok(buf, "\r\n"));  // Strip newline.
            }

            check_and_fclose(fp, COMMAND_FILE);
            LOGI("Got arguments from %s\n", COMMAND_FILE);
        }
    }

    // --> write the arguments we have back into the bootloader control block
    // always boot into recovery after this (until finish_recovery() is called)
    strlcpy(boot.command, "boot-recovery", sizeof(boot.command));
    strlcpy(boot.recovery, "recovery\n", sizeof(boot.recovery));
    int i;
    for (i = 1; i < *argc; ++i) {
        strlcat(boot.recovery, (*argv)[i], sizeof(boot.recovery));
        strlcat(boot.recovery, "\n", sizeof(boot.recovery));
    }
    if (device_flash_type() == MTD) {
        set_bootloader_message(&boot);
    }
}

void
set_sdcard_update_bootloader_message() {
    struct bootloader_message boot;
    memset(&boot, 0, sizeof(boot));
    strlcpy(boot.command, "boot-recovery", sizeof(boot.command));
    strlcpy(boot.recovery, "recovery\n", sizeof(boot.recovery));
    set_bootloader_message(&boot);
}

// How much of the temp log we have copied to the copy in cache.
static long tmplog_offset = 0;

static void
copy_log_file(const char* destination, int append) {
    FILE *log = fopen_path(destination, append ? "a" : "w");
    if (log == NULL) {
        LOGE("Can't open %s\n", destination);
    } else {
        FILE *tmplog = fopen(TEMPORARY_LOG_FILE, "r");
        if (tmplog == NULL) {
            LOGE("Can't open %s\n", TEMPORARY_LOG_FILE);
        } else {
            if (append) {
                fseek(tmplog, tmplog_offset, SEEK_SET);  // Since last write
            }
            char buf[4096];
            while (fgets(buf, sizeof(buf), tmplog)) fputs(buf, log);
            if (append) {
                tmplog_offset = ftell(tmplog);
            }
            check_and_fclose(tmplog, TEMPORARY_LOG_FILE);
        }
        check_and_fclose(log, destination);
    }
}


// clear the recovery command and prepare to boot a (hopefully working) system,
// copy our log file to cache as well (for the system to read), and
// record any intent we were asked to communicate back to the system.
// this function is idempotent: call it as many times as you like.
static void
finish_recovery(const char *send_intent) {
    // By this point, we're ready to return to the main system...
    if (send_intent != NULL) {
        FILE *fp = fopen_path(INTENT_FILE, "w");
        if (fp == NULL) {
            LOGE("Can't open %s\n", INTENT_FILE);
        } else {
            fputs(send_intent, fp);
            check_and_fclose(fp, INTENT_FILE);
        }
    }

    // Copy logs to cache so the system can find out what happened.
    copy_log_file(LOG_FILE, true);
    copy_log_file(LAST_LOG_FILE, false);
    chmod(LAST_LOG_FILE, 0640);

    if (device_flash_type() == MTD) {
        // Reset to mormal system boot so recovery won't cycle indefinitely.
        struct bootloader_message boot;
        memset(&boot, 0, sizeof(boot));
        set_bootloader_message(&boot);
    }

    // Remove the command file, so recovery won't repeat indefinitely.
    if (ensure_path_mounted(COMMAND_FILE) != 0 ||
        (unlink(COMMAND_FILE) && errno != ENOENT)) {
        LOGW("Can't unlink %s\n", COMMAND_FILE);
    }

    sync();  // For good measure.
}

static int
erase_volume(const char *volume) {
    ui_set_background(BACKGROUND_ICON_INSTALLING);
    ui_show_indeterminate_progress();
    ui_print("Erasing %s...\n", volume);

    if (strcmp(volume, "/cache") == 0) {
        // Any part of the log we'd copied to cache is now gone.
        // Reset the pointer so we copy from the beginning of the temp
        // log.
        tmplog_offset = 0;
    }

    return format_volume(volume);
}

static char*
copy_sideloaded_package(const char* original_path) {
  if (ensure_path_mounted(original_path) != 0) {
    LOGE("Can't mount %s\n", original_path);
    return NULL;
  }

  if (ensure_path_mounted(SIDELOAD_TEMP_DIR) != 0) {
    LOGE("Can't mount %s\n", SIDELOAD_TEMP_DIR);
    return NULL;
  }

  if (mkdir(SIDELOAD_TEMP_DIR, 0700) != 0) {
    if (errno != EEXIST) {
      LOGE("Can't mkdir %s (%s)\n", SIDELOAD_TEMP_DIR, strerror(errno));
      return NULL;
    }
  }

  // verify that SIDELOAD_TEMP_DIR is exactly what we expect: a
  // directory, owned by root, readable and writable only by root.
  struct stat st;
  if (stat(SIDELOAD_TEMP_DIR, &st) != 0) {
    LOGE("failed to stat %s (%s)\n", SIDELOAD_TEMP_DIR, strerror(errno));
    return NULL;
  }
  if (!S_ISDIR(st.st_mode)) {
    LOGE("%s isn't a directory\n", SIDELOAD_TEMP_DIR);
    return NULL;
  }
  if ((st.st_mode & 0777) != 0700) {
    LOGE("%s has perms %o\n", SIDELOAD_TEMP_DIR, st.st_mode);
    return NULL;
  }
  if (st.st_uid != 0) {
    LOGE("%s owned by %lu; not root\n", SIDELOAD_TEMP_DIR, st.st_uid);
    return NULL;
  }

  char copy_path[PATH_MAX];
  strcpy(copy_path, SIDELOAD_TEMP_DIR);
  strcat(copy_path, "/package.zip");

  char* buffer = malloc(BUFSIZ);
  if (buffer == NULL) {
    LOGE("Failed to allocate buffer\n");
    return NULL;
  }

  size_t read;
  FILE* fin = fopen(original_path, "rb");
  if (fin == NULL) {
    LOGE("Failed to open %s (%s)\n", original_path, strerror(errno));
    return NULL;
  }
  FILE* fout = fopen(copy_path, "wb");
  if (fout == NULL) {
    LOGE("Failed to open %s (%s)\n", copy_path, strerror(errno));
    return NULL;
  }

  while ((read = fread(buffer, 1, BUFSIZ, fin)) > 0) {
    if (fwrite(buffer, 1, read, fout) != read) {
      LOGE("Short write of %s (%s)\n", copy_path, strerror(errno));
      return NULL;
    }
  }

  free(buffer);

  if (fclose(fout) != 0) {
    LOGE("Failed to close %s (%s)\n", copy_path, strerror(errno));
    return NULL;
  }

  if (fclose(fin) != 0) {
    LOGE("Failed to close %s (%s)\n", original_path, strerror(errno));
    return NULL;
  }

  // "adb push" is happy to overwrite read-only files when it's
  // running as root, but we'll try anyway.
  if (chmod(copy_path, 0400) != 0) {
    LOGE("Failed to chmod %s (%s)\n", copy_path, strerror(errno));
    return NULL;
  }

  return strdup(copy_path);
}

static char**
prepend_title(char** headers) {
    char* title[] = { "MIDNIGHT KERNEL / CWM4 BASED RECOVERY",      //EXPAND(RECOVERY_VERSION),
                      "",
                      NULL };

    // count the number of lines in our title, plus the
    // caller-provided headers.
    int count = 0;
    char** p;
    for (p = title; *p; ++p, ++count);
    for (p = headers; *p; ++p, ++count);

    char** new_headers = malloc((count+1) * sizeof(char*));
    char** h = new_headers;
    for (p = title; *p; ++p, ++h) *h = *p;
    for (p = headers; *p; ++p, ++h) *h = *p;
    *h = NULL;

    return new_headers;
}

int
get_menu_selection(char** headers, char** items, int menu_only,
                   int initial_selection) {
    // throw away keys pressed previously, so user doesn't
    // accidentally trigger menu items.
    ui_clear_key_queue();

    int item_count = ui_start_menu(headers, items, initial_selection);
    int selected = initial_selection;
    int chosen_item = -1;

    // Some users with dead enter keys need a way to turn on power to select.
    // Jiggering across the wrapping menu is one "secret" way to enable it.
    // We can't rely on /cache or /sdcard since they may not be available.
    int wrap_count = 0;

    while (chosen_item < 0 && chosen_item != GO_BACK) {
        int key = ui_wait_key();
        int visible = ui_text_visible();

        int action = device_handle_key(key, visible);

        int old_selected = selected;

        if (action < 0) {
            switch (action) {
                case HIGHLIGHT_UP:
                    --selected;
                    selected = ui_menu_select(selected);
                    break;
                case HIGHLIGHT_DOWN:
                    ++selected;
                    selected = ui_menu_select(selected);
                    break;
                case SELECT_ITEM:
                    chosen_item = selected;
                    if (ui_get_showing_back_button()) {
                        if (chosen_item == item_count) {
                            chosen_item = GO_BACK;
                        }
                    }
                    break;
                case NO_ACTION:
                    break;
                case GO_BACK:
                    chosen_item = GO_BACK;
                    break;
            }
        } else if (!menu_only) {
            chosen_item = action;
        }

        if (abs(selected - old_selected) > 1) {
            wrap_count++;
            if (wrap_count == 3) {
                wrap_count = 0;
                if (ui_get_showing_back_button()) {
                    ui_print("Back menu button disabled.\n");
                    ui_set_showing_back_button(0);
                }
                else {
                    ui_print("Back menu button enabled.\n");
                    ui_set_showing_back_button(1);
                }
            }
        }
    }

    ui_end_menu();
    ui_clear_key_queue();
    return chosen_item;
}

static int compare_string(const void* a, const void* b) {
    return strcmp(*(const char**)a, *(const char**)b);
}

static int
sdcard_directory(const char* path) {
    ensure_path_mounted(SDCARD_ROOT);

    const char* MENU_HEADERS[] = { "Choose a package to install:",
                                   path,
                                   "",
                                   NULL };
    DIR* d;
    struct dirent* de;
    d = opendir(path);
    if (d == NULL) {
        LOGE("error opening %s: %s\n", path, strerror(errno));
        ensure_path_unmounted(SDCARD_ROOT);
        return 0;
    }

    char** headers = prepend_title(MENU_HEADERS);

    int d_size = 0;
    int d_alloc = 10;
    char** dirs = malloc(d_alloc * sizeof(char*));
    int z_size = 1;
    int z_alloc = 10;
    char** zips = malloc(z_alloc * sizeof(char*));
    zips[0] = strdup("../");

    while ((de = readdir(d)) != NULL) {
        int name_len = strlen(de->d_name);

        if (de->d_type == DT_DIR) {
            // skip "." and ".." entries
            if (name_len == 1 && de->d_name[0] == '.') continue;
            if (name_len == 2 && de->d_name[0] == '.' &&
                de->d_name[1] == '.') continue;

            if (d_size >= d_alloc) {
                d_alloc *= 2;
                dirs = realloc(dirs, d_alloc * sizeof(char*));
            }
            dirs[d_size] = malloc(name_len + 2);
            strcpy(dirs[d_size], de->d_name);
            dirs[d_size][name_len] = '/';
            dirs[d_size][name_len+1] = '\0';
            ++d_size;
        } else if (de->d_type == DT_REG &&
                   name_len >= 4 &&
                   strncasecmp(de->d_name + (name_len-4), ".zip", 4) == 0) {
            if (z_size >= z_alloc) {
                z_alloc *= 2;
                zips = realloc(zips, z_alloc * sizeof(char*));
            }
            zips[z_size++] = strdup(de->d_name);
        }
    }
    closedir(d);

    qsort(dirs, d_size, sizeof(char*), compare_string);
    qsort(zips, z_size, sizeof(char*), compare_string);

    // append dirs to the zips list
    if (d_size + z_size + 1 > z_alloc) {
        z_alloc = d_size + z_size + 1;
        zips = realloc(zips, z_alloc * sizeof(char*));
    }
    memcpy(zips + z_size, dirs, d_size * sizeof(char*));
    free(dirs);
    z_size += d_size;
    zips[z_size] = NULL;

    int result;
    int chosen_item = 0;
    do {
        chosen_item = get_menu_selection(headers, zips, 1, chosen_item);

        char* item = zips[chosen_item];
        int item_len = strlen(item);
        if (chosen_item == 0) {          // item 0 is always "../"
            // go up but continue browsing (if the caller is sdcard_directory)
            result = -1;
            break;
        } else if (item[item_len-1] == '/') {
            // recurse down into a subdirectory
            char new_path[PATH_MAX];
            strlcpy(new_path, path, PATH_MAX);
            strlcat(new_path, "/", PATH_MAX);
            strlcat(new_path, item, PATH_MAX);
            new_path[strlen(new_path)-1] = '\0';  // truncate the trailing '/'
            result = sdcard_directory(new_path);
            if (result >= 0) break;
        } else {
            // selected a zip file:  attempt to install it, and return
            // the status to the caller.
            char new_path[PATH_MAX];
            strlcpy(new_path, path, PATH_MAX);
            strlcat(new_path, "/", PATH_MAX);
            strlcat(new_path, item, PATH_MAX);

            ui_print("\n-- Install %s ...\n", path);
            set_sdcard_update_bootloader_message();
            char* copy = copy_sideloaded_package(new_path);
            ensure_path_unmounted(SDCARD_ROOT);
            if (copy) {
                result = install_package(copy);
                free(copy);
            } else {
                result = INSTALL_ERROR;
            }
            break;
        }
    } while (true);

    int i;
    for (i = 0; i < z_size; ++i) free(zips[i]);
    free(zips);
    free(headers);

    ensure_path_unmounted(SDCARD_ROOT);
    return result;
}

static void
wipe_data(int confirm) {
    if (confirm) {
        static char** title_headers = NULL;

        if (title_headers == NULL) {
            char* headers[] = { "Confirm wipe of all user data?",
                                "  THIS CAN NOT BE UNDONE.",
                                "",
                                NULL };
            title_headers = prepend_title((const char**)headers);
        }

        char* items[] = { " No",
                          " No",
                          " No",
                          " No",
                          " No",
                          " No",
                          " No",
                          " Yes -- delete all user data",   // [7]
                          " No",
                          " No",
                          " No",
                          NULL };

        int chosen_item = get_menu_selection(title_headers, items, 1, 0);
        if (chosen_item != 7) {
            return;
        }
    }

    ui_print("\nWiping data...\n");
    device_wipe_data();
    erase_volume("/data");
    erase_volume("/cache");
    if (has_datadata()) {
        erase_volume("/dbdata");
    }
    // Midnight: skip sd-ext on stock Samsung ROM
    //erase_volume("/sd-ext");
    erase_volume("/sdcard/.android_secure");
    ui_print("Data wipe complete.\n");
}



void backup_menu(){
/*
 * Nandroid FULL
 * Nandroid DATA
 * Nandroid SYSTEM
 * ANIM
 * SOUNDS
 * MIDNIGHTCONF
 */
    ensure_path_mounted("/system");
    ensure_path_mounted("/data"); 
    create_backup_dirs();
    static char* headers[] = {  "BACKUP MENU",
                                "Perform a Nandroid backup or backup your chosen" 
                                "files to /data/midnight/backup.",
                                NULL};
    static char* list[] = { "Nandroid backup FULL",
                            "Nandroid backup SYSTEM",
                            "Nandroid backup DATA",
                            "Backup custom bootanimations",
                            "Backup start/shutdown sounds",
                            "Backup Midnight configfiles",
                            "Backup SystemUI / framework-res",
                            "Backup /data/app tp SDCARD",
                            NULL
    };
    
    

    char backup_path[PATH_MAX];
    
    for (;;)
    {
        int chosen_item = get_menu_selection(headers, list, 0, 0);
        if (chosen_item == GO_BACK)
            break;        
        switch (chosen_item)
        {
          case 0:
                backup_path[0]='\0';nandroid_generate_timestamp_path(backup_path);
                sprintf(backup_path,"%s-FULL",backup_path);
                nandroid_backup(backup_path);
                break;
          case 1:
                backup_path[0]='\0';nandroid_generate_timestamp_path(backup_path);
                sprintf(backup_path,"%s-SYSTEM",backup_path);
                nandroid_backup_selective(backup_path,1); // system
                break;        
          case 2:
                backup_path[0]='\0';nandroid_generate_timestamp_path(backup_path);
                sprintf(backup_path,"%s-DATA",backup_path);
                nandroid_backup_selective(backup_path,2); // data / datadata
                break;              
          case 3:
          {
            if (confirm_selection("Confirm backup custom bootanimations","Yes - backup custom bootanimations")) {  
            ensure_path_mounted("/system");
            ensure_path_mounted("/data");
              ui_print("\nSearching bootanimations......\n");
              if( show_file_exists("Original","/data/local/bootanimation.zip","bootanimation.zip (data):","found.", "not found.") ||
                show_file_exists("Original","/data/local/sanim.zip","sanim.zip (data):","found.", "not found.") ||
                show_file_exists("Original","/data/local/bootanimation.bin","bootanimation.bin (data):","found.", "not found.") ||
                show_file_exists("Original","/system/media/bootanimation.zip","bootanimation.zip (system):","found.", "not found.") ||
                show_file_exists("Original","/system/media/sanim.zip","sanim.zip (system):","found.", "not found.") ){
                    ui_print("Cleaning backup directories...\n");
                    __system("rm -r /data/midnight/backups/bootanimation/local/*");                
                    __system("rm -r /data/midnight/backups/bootanimation/system/*");                
                    ui_print("Backup bootanimations...\n");
                    ui_print("Backup to /data/midnight/backups/...\n");
                    __system("cp /data/local/bootanimation.zip /data/midnight/backups/bootanimation/local");                
                    __system("cp /data/local/sanim.zip /data/midnight/backups/bootanimation/local");                
                    __system("cp /data/local/bootanimation.bin /data/midnight/backups/bootanimation/local");                
                    __system("cp /system/media/bootanimation.zip /data/midnight/backups/bootanimation/system");                
                    __system("cp /system/media/sanim.zip /data/midnight/backups/bootanimation/system");                
                    ui_print("Done.\n");
                }
            }
            break;
          }
          case 4:
                {
                    ensure_path_mounted("/system");
                    ensure_path_mounted("/data");
                    if (confirm_selection("Confirm backup start/shutdown sounds","Yes - backup start/shutdown sounds")) {  
                        ui_print("\nSearching sounds...\n");
                        if( show_file_exists("Original","/system/etc/PowerOn.snd","PowerOn.snd:","found.", "not found.") ||
                        show_file_exists("Original","/system/etc/PowerOn.wav","PowerOn.wav:","found.", "not found.") ||
                        show_file_exists("Original","/system/media/audio/ui/shutdown.ogg","shutdown.ogg:","found.", "not found.") ||
                        show_file_exists("Original","/system/media/audio/ui/PowerOff.wav","PowerOff.wav:","found.", "not found.")){
                            ui_print("Cleaning backup directory...\n");
                            __system("rm -r /data/midnight/backups/sounds/*");                
                            ui_print("Backup to /data/midnight/backups/sounds...\n");
                            ui_print("Backup sounds...\n");
                            __system("cp /system/etc/PowerOn.snd /data/midnight/backups/sounds");                
                            __system("cp /system/etc/PowerOn.wav /data/midnight/backups/sounds");                
                            __system("cp /system/media/audio/ui/shutdown.ogg /data/midnight/backups/sounds");                
                            __system("cp /system/media/audio/ui/PowerOff.wav /data/midnight/backups/sounds");                
                            ui_print("Done.\n");
                        }
                    }
                    break;
                  }
          case 5:
                  {
                    ensure_path_mounted("/system");
                    ensure_path_mounted("/data");
                    if (confirm_selection("Confirm backup Midnight conf files","Yes - backup Midnight config files")) {  
                      ui_print("\nCleaning backup directory...\n");
                      __system("rm -r /data/midnight/backups/midnight-conf/*");                                      
                      ui_print("Backup to /data/midnight/backups/midnight-conf...\n");
                      ui_print("Backup Midnight configfiles...\n");
                      __system("cp /system/etc/midnight_*.conf /data/midnight/backups/midnight-conf");
                      ui_print("Done.\n");
                    }
                    break;
                  }
          case 6:
                  {
                    ensure_path_mounted("/system");
                    ensure_path_mounted("/data");
                    if (confirm_selection("Confirm backup SystemUI/framework-res","Yes - backup theme files")) {  
                        ui_print("\nSearching for files...\n");
                        if( show_file_exists("Original","/system/app/SystemUI.apk","SystemUI.apk:","found.", "not found.") &&
                        show_file_exists("Original","/system/framework/framework-res.apk","framework-res.apk:","found.", "not found.")){ 
                          show_file_exists("Original","/system/app/SystemUI.odex","SystemUI.odex:","found.", "not found.");                         
                          ui_print("\nCleaning backup directory...\n");
                          __system("rm -r /data/midnight/backups/theme/*");                                      
                          ui_print("Backup to /data/midnight/backups/theme...\n");
                          ui_print("Backup theme files...\n");
                          __system("cp /system/app/SystemUI.apk /data/midnight/backups/theme");
                          __system("cp /system/app/SystemUI.odex /data/midnight/backups/theme");
                          __system("cp /system/framework/framework-res.apk /data/midnight/backups/theme");
                          ui_print("Done.\n");
                        }
                    }
                    break;
                  }
          case 7:
          {
            if (confirm_selection("Confirm backup /data/app","Yes - backup APKs to SDCARD")) {  
                apply_appbackup();                
            }
            break;
          }
        }
    }
}

void restore_menu(){
    ensure_path_mounted("/system");
    ensure_path_mounted("/data");
    create_backup_dirs();
    static char* headers[] = {  "RESTORE MENU",
                                "Restore Nandroid backup or your chosen" 
                                "files.",
                                NULL};
    static char* list[] = { "Nandroid restore FULL",
                            "Nandroid restore SYSTEM",
                            "Nandroid restore DATA",
                            "Nandroid restore selected file",
                            "Restore custom bootanimations",
                            "Restore start/shutdown sounds",
                            "Restore Midnight configfiles",
                            "Restore SystemUI / framework-res",
                            NULL
    };
    
    // generate backup_path
    char backup_path[PATH_MAX];
    time_t t = time(NULL);
    struct tm *tmp = localtime(&t);
    if (tmp == NULL)
    {
        struct timeval tp;
        gettimeofday(&tp, NULL);
        sprintf(backup_path, "/sdcard/clockworkmod/backup/%d", tp.tv_sec);
    }
    else
    {
        strftime(backup_path, sizeof(backup_path), "/sdcard/clockworkmod/backup/%F.%H.%M.%S", tmp);
    }


    for (;;)
    {
        int chosen_item = get_menu_selection(headers, list, 0, 0);
        if (chosen_item == GO_BACK)
            break;        
        switch (chosen_item)
        {
         case 0:
            show_nandroid_restore_delete_menu(0);
            break;
        case 1:
            show_nandroid_advanced_restore_menu(1);
            break;
        case 2:
            show_nandroid_advanced_restore_menu(2);
            break;
        case 3:
            show_nandroid_advanced_restore_menu(0);
            break; 
          case 4:
          {
              {
                ensure_path_mounted("/system");
                ensure_path_mounted("/data");
                if (confirm_selection("Confirm restoring custom bootanimations","Yes - restore bootanimations")) {  
                  ui_print("\nRestoring bootanimations...\n");
                  __system("cp /data/midnight/backups/bootanimation/local/bootanimation.zip /data/local");                
                  __system("cp /data/midnight/backups/bootanimation/local/sanim.zip /data/local");                
                  __system("cp /data/midnight/backups/bootanimation/local/bootanimation.bin /data/local");                
                  __system("cp /data/midnight/backups/bootanimation/system/bootanimation.zip /system/media");                
                  __system("cp /data/midnight/backups/bootanimation/system/sanim.zip /system/media");                
                  ui_print("Done.\n");
                }
                break;
              }
          }
          case 5:
                {
                    ensure_path_mounted("/system");
                    ensure_path_mounted("/data");
                    if (confirm_selection("Confirm restoring boot/shutdown sounds","Yes - restore sounds")) {  
                      ui_print("\nRestoring sounds...\n");
                      __system("cp /data/midnight/backups/sounds/PowerOn.snd /system/etc");                
                      __system("cp /data/midnight/backups/sounds/PowerOn.wav /system/etc");                
                      __system("cp /data/midnight/backups/sounds/shutdown.ogg /system/media/audio/ui");                
                      __system("cp /data/midnight/backups/sounds/PowerOff.wav /system/media/audio/ui");                
                      ui_print("Done.\n");
                    }
                    break;
                  }
          case 6:
                  {
                    ensure_path_mounted("/system");
                    ensure_path_mounted("/data");
                    if (confirm_selection("Confirm restoring Midnight config files","Yes - restore Midnight config files")) {  
                      ui_print("\nCleaning configfiles...\n");
                      __system("rm /system/etc/midnight_*.conf");
                      ui_print("Restoring configfiles...\n");
                      __system("cp /data/midnight/backups/midnight-conf/midnight_*.conf /system/etc");
                      ui_print("Done.\n");
                    }
                    break;
                  }
          case 7:
                  {
                    ensure_path_mounted("/system");
                    ensure_path_mounted("/data");
                    if (confirm_selection("Confirm restoring SystemUI/framework-res","Yes - restore theme files")) {  
                        ui_print("\nSearching for theme files...\n");
                        if( show_file_exists("Backup","/data/midnight/backups/theme/SystemUI.apk","SystemUI.apk:","found.", "not found.") &&
                        show_file_exists("Backup","/data/midnight/backups/theme/framework-res.apk","framework-res.apk:","found.", "not found.")){  
                        show_file_exists("Backup","/data/midnight/backups/theme/SystemUI.odex","SystemUI.odex:","found.", "not found.");    
                        if((
                            0 == file_exists("/data/midnight/backups/theme/SystemUI.odex") && 
                            0 != file_exists("/system/app/SystemUI.odex")) ||
                            (
                            0 != file_exists("/data/midnight/backups/theme/SystemUI.odex") && 
                            0 == file_exists("/system/app/SystemUI.odex"))                            
                            ){
                                ui_print("Odexed/deodexed mismatch, exiting...\n");
                                return; 
                            }
                          ui_print("\nRestoring theme files...\n");
                          __system("cp /data/midnight/backups/theme/SystemUI.apk /system/app");
                          __system("cp /data/midnight/backups/theme/SystemUI.odex /system/app");
                          __system("cp /data/midnight/backups/theme/framework-res.apk /system/framework");
                          ui_print("Done.\n");
                        }
                    }
                    break;
                  }

        }
    }
}

void rm_bloat(const char *filename){
    char tmp[PATH_MAX];
    sprintf(tmp,"rm /system/app/%s.apk",filename);
    if(0 == __system(tmp)){
        ui_print("Deleted: %s.apk\n",filename);
        LOGI("Deleted: %s.apk\n",filename);
    }else{
        //ui_print("Failed: %s.apk\n",filename);
        LOGI("Failed: %s.apk\n",filename);
        }
    sprintf(tmp,"rm /system/app/%s.odex",filename);
    if(0 == __system(tmp)){
        ui_print("Deleted: %s.odex\n",filename);
        LOGI("Deleted: %s.odex\n",filename);
    }else{
        //ui_print("Failed: %s.odex\n",filename);
        LOGI("Failed: %s.odex\n",filename);
        }
}

void cleanup_menu(){
    create_backup_dirs();
    static char* headers[] = {  "CLEANUP / WIPE MENU",
                                "Cleanup or wipe whatever you want." 
                                "Be sure to have a backup if necessary.",
                                NULL};
    static char* list[] = { "Wipe cache...",
                            "Wipe dalvik-cache...",
                            "Wipe battery stats...",
                            "Factory reset...",
                            "Delete Nandroid backup",
                            "Delete custom bootanimations",
                            "Delete start/shutdown sounds",
                            "Delete Midnight configfiles",
                            "Delete bloatware (~15Mb)",
                            "Delete some Google apps (~0.8Mb)",
                            "Delete SamsungApps (~5Mb)",
                            "Delete Facebook/Twitter sync (~0.7Mb)",
                            "Delete some fonts (~3Mb)",
                            "Delete Samsung Widgets (~3.7Mb)",
                            "Delete a lot of bloat/software (~52Mb)",
                            NULL
    };
    
    ensure_path_mounted("/system");
    ensure_path_mounted("/data");
      
    ui_print("\nBloatware-cleanup info\n");
    ui_print("------------------\n");
    ui_print("THE LAST 7 OPTIONS REMOVE BLOATWARE\n");
    ui_print("AND POTENTIALLY UNNEDED SOFTWARE!\n");
    ui_print("The options will display a list of\n");
    ui_print("all packages which will be deleted.\n");
    ui_print("Be sure to read this list before\n");
    ui_print("you confirm.\n");
    ui_print("A Nandroid-SYSTEM backup might be useful\n");
    ui_print("as there will be no apk/odex backup.\n");
    ui_print("------------------\n");
    
    for (;;)
    {
        int chosen_item = get_menu_selection(headers, list, 0, 0);
        if (chosen_item == GO_BACK)
            break;        
        switch (chosen_item)
        {
              case 0:
              {
                if (confirm_selection("Confirm wipe?", "Yes - wipe cache"))
                {
                    ui_print("\nWiping cache...\n");
                    erase_volume("/cache");
                    ui_print("Cache wipe complete.\n");
                    if (!ui_text_visible()) return;
                }
                break;                  
              }
              case 1:
              {
                if (0 != ensure_path_mounted("/data"))
                    break;
                if (confirm_selection( "Confirm wipe?", "Yes - Wipe dalvik-cache")) {
                    // Midnight: clear cache and data in any case
                    ensure_path_mounted("/cache");
                    ui_print("\nWiping /data/dalvik-cache...\n");
                    __system("rm -r /data/dalvik-cache");
                    ui_print("Wiping /cache/dalvik-cache...\n");
                    __system("rm -r /cache/dalvik-cache");
                    ui_print("Dalvik cache wiped.\n");
                    /*
                     * MIDNIGHT: No sd-ext on stock Samsung ROM
                     */
                    //ensure_path_mounted("/sd-ext");
                    //__system("rm -r /sd-ext/dalvik-cache");
                }
                ensure_path_unmounted("/data");
                break;                  
              }
              case 2:
              {
               if (confirm_selection( "Confirm wipe?", "Yes - wipe battery stats"))
                    ui_print("\nWiping battery stats...");
                    wipe_battery_stats();
                    /*
                     * MIDNIGHT: Give proper feedback
                     */
                    ui_print("Battery stats wiped.\n");
                break;
              }
              case 3:
              {
                wipe_data(ui_text_visible());
                if (!ui_text_visible()) return;
                break;                  
              }            
         case 4:
            show_nandroid_restore_delete_menu(1);
            break;
          case 5:
          {
                   if (confirm_selection("Confirm deleting custom bootanimations","Yes - delete custom bootanimations")) {  
                        ensure_path_mounted("/system");
                        ensure_path_mounted("/data");
                      ui_print("\nDeleting bootanimations...\n");
                      __system("rm -f /data/local/bootanimation.zip");                
                      __system("rm -f /data/local/sanim.zip");                
                      __system("rm -f /data/local/bootanimation.bin");                
                      __system("rm -f /system/media/bootanimation.zip");                
                      __system("rm -f /system/media/sanim.zip");                
                      ui_print("Done.\n");
                    }
                    break;
          }
          case 6:
                {
                        ensure_path_mounted("/system");
                    if (confirm_selection("Confirm deleting start/shutdown sounds","Yes - delete start/shutdown sounds")) {  
                      ui_print("\nDeleting startup/shutdown sounds...\n");
                      __system("rm -f /system/etc/PowerOn.snd");                
                      __system("rm -f /system/etc/PowerOn.wav");                
                      __system("rm -f /system/media/audio/ui/shutdown.ogg");                
                      ui_print("Done.\n");
                    }
                    break;
                  }
          case 7:
                  {
                   if (confirm_selection("Confirm deleting Midnight conf files","Yes - delete ALL Midnight config files")) {  
                        ensure_path_mounted("/system");
                      ui_print("\nDeleting /system/etc/midnight_*.conf...\n");
                      __system("rm -rf /system/etc/midnight_*.conf");                
                      ui_print("Done.\n");
                    }
                    break;
                  }
          case 8:
                  {
                        ensure_path_mounted("/system");
                    ui_print("\nREMOVING APK/ODEX WITHOUT BACKUP:\n");
                    ui_print("BuddiesNow, Days, Layar-samsung, Memo,\n");
                    ui_print("MiniDiary, PressReader, WriteandGo\n");
                    ui_print("aldiko-standard, thinkdroid, Protips\n");
                    ui_print("YouTube, HRS-Android\n");
                    ui_print("\nFree space on /system before removing: %d Mb\n",get_partition_free("/system"));

                   if (confirm_selection("Confirm deleting /system bloatware","Yes - I know what I'm doing")) {  
                        ui_print("\nDeleting /system bloatware...\n");
                        rm_bloat("BuddiesNow");
                        rm_bloat("Days");
                        rm_bloat("Layar-samsung");
                        rm_bloat("Memo");
                        rm_bloat("MiniDiary");
                        rm_bloat("PressReader");
                        rm_bloat("WriteandGo");
                        rm_bloat("aldiko-standard-1.2.6.1-samsung-s1");
                        rm_bloat("thinkdroid");
                        rm_bloat("Protips");
                        rm_bloat("YouTube");
                        rm_bloat("HRS-Android");
                        ui_print("\nFree @ /system after removing: %d Mb\n",get_partition_free("/system"));
                        ui_print("Done.\n");
                    }
                    break;
                  }
          case 9:
                  {
                        ensure_path_mounted("/system");
                    ui_print("\nREMOVING APK/ODEX WITHOUT BACKUP:\n");
                    ui_print("GoogleFeedback, GooglePartnerSetup,\n");
                    ui_print("GoogleQuickSearchBox\n");
                    ui_print("\nFree space on /system before removing: %d Mb\n",get_partition_free("/system"));

                   if (confirm_selection("Confirm deleting some /system GoogleApps","Yes - I know what I'm doing")) {  
                        ui_print("\nDeleting /system GoogleApps...\n");
                        rm_bloat("GoogleFeedback");
                        rm_bloat("GooglePartnerSetup");
                        rm_bloat("GoogleQuickSearchBox");
                        ui_print("\nFree @ /system after removing: %d Mb\n",get_partition_free("/system"));
                        ui_print("Done.\n");
                    }
                    break;
                  }
          case 10:
                  {
                        ensure_path_mounted("/system");
                    ui_print("\nREMOVING APK/ODEX WITHOUT BACKUP:\n");
                    ui_print("SamsungApps, SamsungAppsUNAService\n");
                    ui_print("SamsungAppsUNA3, signin, wssyncmlnps,\n");
                    ui_print("WlanTest syncmldm syncmlds\n");
                    ui_print("\nFree space on /system before removing: %d Mb\n",get_partition_free("/system"));

                   if (confirm_selection("Confirm deleting /system SamsungApps","Yes - I know what I'm doing")) {  
                        ui_print("\nDeleting /system SamsungApps...\n");
                        rm_bloat("SamsungApps");
                        rm_bloat("SamsungAppsUNAService");
                        rm_bloat("SamsungAppsUNA3"); 
                        rm_bloat("signin"); 
                        rm_bloat("wssyncmlnps"); 
                        rm_bloat("WlanTest"); 
                        rm_bloat("syncmldm"); 
                        rm_bloat("syncmlds");
                        ui_print("\nFree @ /system after removing: %d Mb\n",get_partition_free("/system"));
                        ui_print("Done.\n");
                    }
                    break;
                  }
          case 11:
                  {
                        ensure_path_mounted("/system");
                    ui_print("\nREMOVING APK/ODEX WITHOUT BACKUP:\n");
                    ui_print("SnsAccount, SnsProvider\n");
                    ui_print("\nFree space on /system before removing: %d Mb\n",get_partition_free("/system"));

                   if (confirm_selection("Confirm deleting /system Facebook/Twitter sync","Yes - I know what I'm doing")) {  
                        ui_print("\nDeleting /system Facebook/Twitter sync apps...\n");
                        rm_bloat("SnsAccount");
                        rm_bloat("SnsProvider");
                        ui_print("\nFree @ /system after removing: %d Mb\n",get_partition_free("/system"));
                        ui_print("Done.\n");
                    }
                    break;
                  }
          case 12:
                  {
                        ensure_path_mounted("/system");
                    ui_print("\nREMOVING APK/ODEX WITHOUT BACKUP:\n");
                    ui_print("ChocoEUKor, CoolEUKor, RoseEUKor\n");
                    ui_print("\nFree space on /system before removing: %d Mb\n",get_partition_free("/system"));

                   if (confirm_selection("Confirm deleting /system fonts","Yes - delete some fonts")) {  
                        ui_print("\nDeleting /system fonts...\n");
                        rm_bloat("ChocoEUKor");
                        rm_bloat("CoolEUKor");
                        rm_bloat("RoseEUKor");
                        ui_print("\nFree @ /system after removing: %d Mb\n",get_partition_free("/system"));
                        ui_print("Done.\n");
                    }
                    break;
                  }
          case 13:
                  {
                        ensure_path_mounted("/system");
                    ui_print("\nREMOVING APK/ODEX WITHOUT BACKUP:\n");
                    ui_print("SamsungWidget_CalendarClock\n");
                    ui_print("SamsungWidget_FeedAndUpdate\n");
                    ui_print("SamsungWidget_ProgramMonitor\n");
                    ui_print("SamsungWidget_StockClock\n");
                    ui_print("DualClock\n");
                    ui_print("\nFree space on /system before removing: %d Mb\n",get_partition_free("/system"));

                   if (confirm_selection("Confirm deleting /system Samsung Widgets","Yes - I know what I'm doing")) {  
                        ui_print("\nDeleting /system Samsung widgets...\n");
                        rm_bloat("SamsungWidget_CalendarClock");
                        rm_bloat("SamsungWidget_FeedAndUpdate");
                        rm_bloat("SamsungWidget_ProgramMonitor");
                        rm_bloat("SamsungWidget_StockClock");
                        rm_bloat("DualClock");
                        ui_print("\nFree @ /system after removing: %d Mb\n",get_partition_free("/system"));
                        ui_print("Done.\n");
                    }
                    break;
                  }

          case 14:
                  {
                        ensure_path_mounted("/system");
                    ui_print("\nREMOVING APK/ODEX WITHOUT BACKUP:\n");

                    ui_print("aldiko-standard-1.2.6.1-samsung-s1,");
                    ui_print("AngryGPS, BuddiesNow, ChocoEUKor,");
                    ui_print("CoolEUKor, Days, DualClock, Gallery3D,");
                    ui_print("GoogleBackupTransport, GoogleFeedback,");
                    ui_print("GooglePartnerSetup, GoogleQuickSearchBox,");
                    ui_print("HTMLViewer, InfoAlarm, LiveWallpapers,");
                    ui_print("LiveWallpapersPicker, Memo, MiniDiary,");
                    ui_print("MobileTrackerEngineTwo, MobileTrackerUI,");
                    ui_print("MtpApplication, MyFiles, NetworkLocation,");
                    ui_print("PicoTts, PressReader, Protips,");
                    ui_print("RoseEUKor, SamsungApps,");
                    ui_print("SamsungWidget_CalendarClock,");
                    ui_print("SamsungWidget_FeedAndUpdate,");
                    ui_print("SamsungWidget_ProgramMonitor,");
                    ui_print("SamsungWidget_StockClock,");
                    ui_print("SnsAccount, SnsProvider, Street,");
                    ui_print("Swype, thinkdroid, TwWallpaperChooser,");
                    ui_print("VideoPlayer, VisualizationWallpapers,");
                    ui_print("VoiceDialer, VoiceRecorder,");
                    ui_print("VoiceSearch, WriteandGo,MagicSmokeWallpapers");
 
                    ui_print("\nFree space on /system before removing: %d Mb\n",get_partition_free("/system"));

                   if (confirm_selection("Confirm deleting /system MIALWE preset","Yes - I know what I'm doing")) {  
                        ui_print("\nDeleting /system MIALWE preset...\n");
                        rm_bloat("aldiko-standard-1.2.6.1-samsung-s1");
                        rm_bloat("AngryGPS");
                        rm_bloat("BuddiesNow");
                        rm_bloat("ChocoEUKor");
                        rm_bloat("CoolEUKor");
                        rm_bloat("Days");
                        rm_bloat("DualClock");
                        rm_bloat("Gallery3D");
                        rm_bloat("GoogleBackupTransport");
                        rm_bloat("GoogleFeedback");
                        rm_bloat("GooglePartnerSetup");
                        rm_bloat("GoogleQuickSearchBox");
                        rm_bloat("HTMLViewer");
                        rm_bloat("InfoAlarm");
                        rm_bloat("LiveWallpapers");
                        rm_bloat("LiveWallpapersPicker");
                        rm_bloat("Memo");
                        rm_bloat("MiniDiary");
                        rm_bloat("MobileTrackerEngineTwo");
                        rm_bloat("MobileTrackerUI");
                        rm_bloat("MtpApplication");
                        rm_bloat("MyFiles");
                        rm_bloat("NetworkLocation");
                        rm_bloat("PicoTts");
                        rm_bloat("PressReader");
                        rm_bloat("Protips");
                        rm_bloat("RoseEUKor");
                        rm_bloat("SamsungApps");
                        rm_bloat("SamsungWidget_CalendarClock");
                        rm_bloat("SamsungWidget_FeedAndUpdate");
                        rm_bloat("SamsungWidget_ProgramMonitor");
                        rm_bloat("SamsungWidget_StockClock");
                        rm_bloat("SnsAccount");
                        rm_bloat("SnsProvider");
                        rm_bloat("Street");
                        rm_bloat("Swype");
                        rm_bloat("thinkdroid");
                        rm_bloat("TwWallpaperChooser");
                        rm_bloat("VideoPlayer");
                        rm_bloat("VisualizationWallpapers");
                        rm_bloat("VoiceDialer");
                        rm_bloat("VoiceRecorder");
                        rm_bloat("VoiceSearch");
                        rm_bloat("WriteandGo");
                        rm_bloat("MagicSmokeWallpapers");
                        ui_print("\nFree @ /system after removing: %d Mb\n",get_partition_free("/system"));
                        ui_print("Done.\n");
                    }
                    break;
                  }
        }
    }
}


static void
prompt_and_wait() {
    char** headers = prepend_title((const char**)MENU_HEADERS);

    for (;;) {
        finish_recovery(NULL);
        ui_reset_progress();

        allow_display_toggle = 1;
        int chosen_item = get_menu_selection(headers, MENU_ITEMS, 0, 0);
        allow_display_toggle = 0;

        // device-specific code may take some action here.  It may
        // return one of the core actions handled in the switch
        // statement below.
        chosen_item = device_perform_action(chosen_item);

        switch (chosen_item) {
            case ITEM_REBOOT:
                poweroff=0;
                return;

            case ITEM_REBOOT_RECOVERY:
                poweroff=0;
                reboot_wrapper("recovery");
                return;

            case ITEM_REBOOT_DOWNLOAD:
                poweroff=0;
                reboot_wrapper("download");
                return;
            case ITEM_POWEROFF:
                poweroff=1;
                return;
            /*
             * MIDNIGHT:
             * - WIPE_DATA moved to cleanup menu
             * - WIPE_CACHE moved to cleanup menu
             * - APPLY_UPDATE temporarily disabled
             *                 
            case ITEM_WIPE_DATA:
                wipe_data(ui_text_visible());
                if (!ui_text_visible()) return;
                break;

            case ITEM_WIPE_CACHE:
                if (confirm_selection("Confirm wipe?", "Yes - Wipe Cache"))
                {
                    ui_print("\n-- Wiping cache...\n");
                    erase_volume("/cache");
                    ui_print("Cache wipe complete.\n");
                    if (!ui_text_visible()) return;
                }
                break;

            case ITEM_APPLY_SDCARD:
                if (confirm_selection("Confirm install?", "Yes - Install /sdcard/update.zip"))
                {
                    ui_print("\n-- Install from sdcard...\n");
                    int status = install_package(SDCARD_PACKAGE_FILE);
                    if (status != INSTALL_SUCCESS) {
                        ui_set_background(BACKGROUND_ICON_ERROR);
                        ui_print("Installation aborted.\n");
                    } else if (!ui_text_visible()) {
                        return;  // reboot if logs aren't visible
                    } else {
                        ui_print("\nInstall from sdcard complete.\n");
                    }
                }
                break;
            */
            case ITEM_PARTITION:
                show_partition_menu();
                break;
            case ITEM_INSTALL_ZIP:
                show_install_update_menu();
                break;
            case ITEM_BACKUP:
                backup_menu();
                break;
            case ITEM_CLEANUP:
                cleanup_menu();
                break;
            case ITEM_RESTORE:
                restore_menu();
                break;                
            case ITEM_ROOT:
               show_root_menu();
                break;
            case ITEM_ADVANCED:
                show_advanced_menu();
                break;
            /*
             * MIDNIGHT: Advanced menu temporarily disabled
             * 
            case ITEM_ADVANCED:
                show_advanced_menu();
                break;
            */
        }
    }
}

static void
print_property(const char *key, const char *name, void *cookie) {
    printf("%s=%s\n", key, name);
}

int
main(int argc, char **argv) {
	if (strstr(argv[0], "recovery") == NULL)
	{   
# if 0
    // DEBUG OUTPUT
    int i;
    printf("call: [ %d args]= ",argc);
    for (i = 0; i < argc; i++)
        printf("\"%s\" ",argv[i]);
    printf("\n");
#endif
        if (strstr(argv[0], "flash_image") != NULL)
	        return flash_image_main(argc, argv);
	    if (strstr(argv[0], "volume") != NULL)
	        return volume_main(argc, argv);
	    if (strstr(argv[0], "edify") != NULL)
	        return edify_main(argc, argv);
	    if (strstr(argv[0], "dump_image") != NULL)
	        return dump_image_main(argc, argv);
	    if (strstr(argv[0], "erase_image") != NULL)
	        return erase_image_main(argc, argv);
	    if (strstr(argv[0], "mkyaffs2image") != NULL)
	        return mkyaffs2image_main(argc, argv);
	    if (strstr(argv[0], "unyaffs") != NULL)
	        return unyaffs_main(argc, argv);           
        if (strstr(argv[0], "nandroid"))
            return nandroid_main(argc, argv);                                
        if (strstr(argv[0], "reboot"))
            return reboot_main(argc, argv);
        if (strstr(argv[0], "mount") && !strstr(argv[0], "umount"))
        {
            LOGI("CWM handling mount call...\n");
            load_volume_table();
            if(argc==2){ // only path
                return ensure_path_mounted(argv[1]);
            }else{
                if (argc==3){ // device + path
                    return ensure_path_mounted(argv[2]);                
                    }
            }
        }
        if (strstr(argv[0], "poweroff")){
            return reboot_main(argc, argv);
        }
        if (strstr(argv[0], "setprop"))
            return setprop_main(argc, argv);
		return busybox_driver(argc, argv);
	}
    __system("/sbin/postrecoveryboot.sh");

    int is_user_initiated_recovery = 0;
    time_t start = time(NULL);

    // If these fail, there's not really anywhere to complain...
    freopen(TEMPORARY_LOG_FILE, "a", stdout); setbuf(stdout, NULL);
    freopen(TEMPORARY_LOG_FILE, "a", stderr); setbuf(stderr, NULL);
    printf("Starting recovery on %s", ctime(&start));

    ui_init();
    //ui_print(EXPAND(RECOVERY_VERSION)"\n");
    load_volume_table();
    process_volumes();
    LOGI("Processing arguments.\n");
    get_args(&argc, &argv);

    int previous_runs = 0;
    const char *send_intent = NULL;
    const char *update_package = NULL;
    const char *encrypted_fs_mode = NULL;
    int wipe_data = 0, wipe_cache = 0;
    int toggle_secure_fs = 0;
    int toggle_uilog = 0;
    int freemb = 0;
    encrypted_fs_info encrypted_fs_data;

    LOGI("Checking arguments.\n");
    int arg;
    while ((arg = getopt_long(argc, argv, "", OPTIONS, NULL)) != -1) {
        switch (arg) {
        case 'p': previous_runs = atoi(optarg); break;
        case 's': send_intent = optarg; break;
        case 'u': update_package = optarg; break;
        case 'w': 
#ifndef BOARD_RECOVERY_ALWAYS_WIPES
		wipe_data = wipe_cache = 1;
#endif
		break;
        case 'c': wipe_cache = 1; break;
        case 'e': encrypted_fs_mode = optarg; toggle_secure_fs = 1; break;
        case 't': ui_show_text(1); break;
        case '?':
            LOGE("Invalid command argument\n");
            continue;
        }
    }

    LOGI("device_recovery_start()\n");
    device_recovery_start();

    printf("Command:");
    for (arg = 0; arg < argc; arg++) {
        printf(" \"%s\"", argv[arg]);
    }
    printf("\n");

   /* 
     * MIDNIGHT: Ensure partitions mounting without errors
     * and enable Chanifires EXT4-conversion tool to work
     * properly (failed because of mounting-data failure
     * before). Maybe not needed after roots.c RFS fixes.
     * TODO:    - check if really needed
     *          - if yes put code in a function
     */
    MountedVolume *mv = NULL; 
    scan_mounted_volumes();
    if (ensure_path_mounted("/system") != 0) {
        ui_print("Can't mount %s\n", "/system");
    }else{
        freemb=get_partition_free("/system");
        mv = find_mounted_volume_by_mount_point("/system");
        ui_print("Mounted ok: %s   %s, free: %dMb\n", "/system",mv->filesystem,freemb);
        }

    if (ensure_path_mounted("/cache") != 0) {
        ui_print("Can't mount %s\n", "/cache");
    }else{
        freemb=get_partition_free("/cache");
        mv = find_mounted_volume_by_mount_point("/cache");
        ui_print("Mounted ok: %s    %s, free: %dMb\n", "/cache",mv->filesystem,freemb);
        }

    if (ensure_path_mounted("/data") != 0) {
        ui_print("Can't mount %s\n", "/data");
    }else{
        freemb=get_partition_free("/data");
        mv = find_mounted_volume_by_mount_point("/data");
        ui_print("Mounted ok: %s     %s, free: %dMb\n", "/data",mv->filesystem,freemb);
        }

    if (ensure_path_mounted("/dbdata") != 0) {
        ui_print("Can't mount %s\n", "/dbdata");
    }else{
        freemb=get_partition_free("/dbdata");
        mv = find_mounted_volume_by_mount_point("/dbdata");
        ui_print("Mounted ok: %s   %s, free: %dMb\n", "/dbdata",mv->filesystem,freemb);
        }
        
    if (ensure_path_mounted("/sdcard") != 0) {
        ui_print("Can't mount %s\n", "/sdcard");
    }else{
        freemb=get_partition_free("/sdcard");
        mv = find_mounted_volume_by_mount_point("/sdcard");
        ui_print("Mounted ok: %s   %s, free: %dMb\n", "/sdcard",mv->filesystem,freemb);
        }
        
    if (update_package) {
        // For backwards compatibility on the cache partition only, if
        // we're given an old 'root' path "CACHE:foo", change it to
        // "/cache/foo".
        if (strncmp(update_package, "CACHE:", 6) == 0) {
            int len = strlen(update_package) + 10;
            char* modified_path = malloc(len);
            strlcpy(modified_path, "/cache/", len);
            strlcat(modified_path, update_package+6, len);
            printf("(replacing path \"%s\" with \"%s\")\n",
                   update_package, modified_path);
            update_package = modified_path;
        }
    }
    printf("\n");

    property_list(print_property, NULL);
    printf("\n");

    int status = INSTALL_SUCCESS;
    
    if (toggle_secure_fs) {
        if (strcmp(encrypted_fs_mode,"on") == 0) {
            encrypted_fs_data.mode = MODE_ENCRYPTED_FS_ENABLED;
            ui_print("Enabling Encrypted FS.\n");
        } else if (strcmp(encrypted_fs_mode,"off") == 0) {
            encrypted_fs_data.mode = MODE_ENCRYPTED_FS_DISABLED;
            ui_print("Disabling Encrypted FS.\n");
        } else {
            ui_print("Error: invalid Encrypted FS setting.\n");
            status = INSTALL_ERROR;
        }

        // Recovery strategy: if the data partition is damaged, disable encrypted file systems.
        // This preventsthe device recycling endlessly in recovery mode.
        if ((encrypted_fs_data.mode == MODE_ENCRYPTED_FS_ENABLED) &&
                (read_encrypted_fs_info(&encrypted_fs_data))) {
            ui_print("Encrypted FS change aborted, resetting to disabled state.\n");
            encrypted_fs_data.mode = MODE_ENCRYPTED_FS_DISABLED;
        }

        if (status != INSTALL_ERROR) {
            if (erase_volume("/data")) {
                ui_print("Data wipe failed.\n");
                status = INSTALL_ERROR;
            } else if (erase_volume("/cache")) {
                ui_print("Cache wipe failed.\n");
                status = INSTALL_ERROR;
            } else if ((encrypted_fs_data.mode == MODE_ENCRYPTED_FS_ENABLED) &&
                      (restore_encrypted_fs_info(&encrypted_fs_data))) {
                ui_print("Encrypted FS change aborted.\n");
                status = INSTALL_ERROR;
            } else {
                ui_print("Successfully updated Encrypted FS.\n");
                status = INSTALL_SUCCESS;
            }
        }
    } else if (update_package != NULL) {
        status = install_package(update_package);
        if (status != INSTALL_SUCCESS) ui_print("Installation aborted.\n");
    } else if (wipe_data) {
        if (device_wipe_data()) status = INSTALL_ERROR;
        if (erase_volume("/data")) status = INSTALL_ERROR;
        if (wipe_cache && erase_volume("/cache")) status = INSTALL_ERROR;
        if (status != INSTALL_SUCCESS) ui_print("Data wipe failed.\n");
    } else if (wipe_cache) {
        if (wipe_cache && erase_volume("/cache")) status = INSTALL_ERROR;
        if (status != INSTALL_SUCCESS) ui_print("Cache wipe failed.\n");
    } else if (toggle_uilog) {
        ui_print("UILOG INITIATED\n");
    } else {
        LOGI("Checking for extendedcommand...\n");
        status = INSTALL_ERROR;  // No command specified
        // we are starting up in user initiated recovery here
        // let's set up some default options
        signature_check_enabled = 0;
        script_assert_enabled = 0;
        is_user_initiated_recovery = 1;
        ui_set_show_text(1);
        ui_set_background(BACKGROUND_ICON_CLOCKWORK);
        
        if (extendedcommand_file_exists()) {
            LOGI("Running extendedcommand...\n");
            int ret;
            if (0 == (ret = run_and_remove_extendedcommand())) {
                status = INSTALL_SUCCESS;
                ui_set_show_text(0);
            }
            else {
                handle_failure(ret);
            }
        } else {
            LOGI("Skipping execution of extendedcommand, file not found...\n");
        }
    }

    if (status != INSTALL_SUCCESS && !is_user_initiated_recovery) ui_set_background(BACKGROUND_ICON_ERROR);
    if (status != INSTALL_SUCCESS || ui_text_visible()) {
        prompt_and_wait();
    }

    // Otherwise, get ready to boot the main system...
    finish_recovery(send_intent);
    if(!poweroff)
        ui_print("Rebooting...\n");
    else
        ui_print("Shutting down...\n");
    sync();
    reboot((!poweroff) ? RB_AUTOBOOT : RB_POWER_OFF);
    return EXIT_SUCCESS;
}

int get_allow_toggle_display() {
    return allow_display_toggle;
}
