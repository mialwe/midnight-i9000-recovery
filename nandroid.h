#ifndef NANDROID_H
#define NANDROID_H

#define DONT_BACKUP_SYSTEM 1

int nandroid_backup_flags(const char* backup_path, int flags);
int nandroid_main(int argc, char** argv);
int nandroid_backup(const char* backup_path);
int nandroid_restore(const char* backup_path, int restore_boot, int restore_system, int restore_data, int restore_cache, int restore_sdext);
void nandroid_generate_timestamp_path(char* backup_path);

#endif
