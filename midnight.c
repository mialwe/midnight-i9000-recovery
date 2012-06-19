/* Copyright (C) 2010 Zsolt Sz Sztupák
 * Modified by HardCORE (Rodney Clinton Chua)
 * Modified and renamed (orig. lagfixutils.c) by mialwe (Michael Weingärtner)
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
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/vfs.h>
#include "bootloader.h"
#include "common.h"
#include "cutils/properties.h"
#include "install.h"
#include "minui/minui.h"
#include "minzip/DirUtil.h"
#include "roots.h"
#include "recovery_ui.h"

#include "extendedcommands.h"
#include "midnight.h"
#include "nandroid.h"

extern char **environ;

void apply_ln(char* name) {
  char tmp[128];
  sprintf(tmp,"ln -s /system/xbin/busybox /system/xbin/%s",name);
  __system(tmp);
}

void remove_ln(char* name) {
  char tmp[128];
  sprintf(tmp,"rm /system/xbin/%s",name);
  __system(tmp);
}

void apply_rm(char* name) {
  char tmp[128];
  sprintf(tmp,"/system/xbin/rm /system/bin/%s",name);
  __system(tmp);
}

void remove_root_from_device(void){
    ensure_path_mounted("/system");
    ensure_path_mounted("/data");

    ui_print("Deleting all su binaries...\n");
    __system("rm /system/bin/su");
    __system("rm /system/xbin/su");
    __system("rm /vendor/bin/su");
    __system("rm /system/sbin/su");

    ui_print("Deleting all busybox binaries...\n");
    __system("rm /system/xbin/busybox");
    __system("rm /system/bin/busybox");
    __system("rm /vendor/bin/busybox");
    __system("rm /system/sbin/busybox");

    ui_print("Deleting Superuser.apk...\n");
    __system("rm /system/app/Superuser.apk");
    __system("rm /data/app/Superuser.apk");
  
    ui_print("Removing xbin symlinks...\n");
    remove_ln("[");
    remove_ln("[[");
    remove_ln("ash");
    remove_ln("awk");
    remove_ln("basename");
    remove_ln("bbconfig");
    remove_ln("bunzip2");
    remove_ln("bzcat");
    remove_ln("bzip2");
    remove_ln("cal");
    remove_ln("cat");
    remove_ln("catv");
    remove_ln("chgrp");
    remove_ln("chmod");
    remove_ln("chown");
    remove_ln("chroot");
    remove_ln("cksum");
    remove_ln("clear");
    remove_ln("cmp");
    remove_ln("cp");
    remove_ln("cpio");
    remove_ln("cut");
    remove_ln("date");
    remove_ln("dc");
    remove_ln("dd");
    remove_ln("depmod");
    remove_ln("devmem");
    remove_ln("df");
    remove_ln("diff");
    remove_ln("dirname");
    remove_ln("dmesg");
    remove_ln("dos2unix");
    remove_ln("du");
    remove_ln("echo");
    remove_ln("egrep");
    remove_ln("env");
    remove_ln("expr");
    remove_ln("false");
    remove_ln("fdisk");
    remove_ln("fgrep");
    remove_ln("find");
    remove_ln("fold");
    remove_ln("free");
    remove_ln("freeramdisk");
    remove_ln("fuser");
    remove_ln("getopt");
    remove_ln("grep");
    remove_ln("gunzip");
    remove_ln("gzip");
    remove_ln("head");
    remove_ln("hexdump");
    remove_ln("id");
    remove_ln("insmod");
    remove_ln("install");
    remove_ln("kill");
    remove_ln("killall");
    remove_ln("killall5");
    remove_ln("length");
    remove_ln("less");
    remove_ln("ln");
    remove_ln("losetup");
    remove_ln("ls");
    remove_ln("lsmod");
    remove_ln("lspci");
    remove_ln("lsusb");
    remove_ln("lzop");
    remove_ln("lzopcat");
    remove_ln("md5sum");
    remove_ln("mkdir");
    remove_ln("mke2fs");
    remove_ln("mkfifo");
    remove_ln("mkfs.ext2");
    remove_ln("mknod");
    remove_ln("mkswap");
    remove_ln("mktemp");
    remove_ln("modprobe");
    remove_ln("more");
    remove_ln("mount");
    remove_ln("mountpoint");
    remove_ln("mv");
    remove_ln("nice");
    remove_ln("nohup");
    remove_ln("od");
    remove_ln("patch");
    remove_ln("pgrep");
    remove_ln("pidof");
    remove_ln("pkill");
    remove_ln("printenv");
    remove_ln("printf");
    remove_ln("ps");
    remove_ln("pwd");
    remove_ln("rdev");
    remove_ln("readlink");
    remove_ln("realpath");
    remove_ln("renice");
    remove_ln("reset");
    remove_ln("rm");
    remove_ln("rmdir");
    remove_ln("rmmod");
    remove_ln("run-parts");
    remove_ln("sed");
    remove_ln("seq");
    remove_ln("setsid");
    remove_ln("sh");
    remove_ln("sha1sum");
    remove_ln("sha256sum");
    remove_ln("sha512sum");
    remove_ln("sleep");
    remove_ln("sort");
    remove_ln("split");
    remove_ln("stat");
    remove_ln("strings");
    remove_ln("stty");
    remove_ln("swapoff");
    remove_ln("swapon");
    remove_ln("sync");
    remove_ln("sysctl");
    remove_ln("tac");
    remove_ln("tail");
    remove_ln("tar");
    remove_ln("tee");
    remove_ln("test");
    remove_ln("time");
    remove_ln("top");
    remove_ln("touch");
    remove_ln("tr");
    remove_ln("true");
    remove_ln("tty");
    remove_ln("tune2fs");
    remove_ln("umount");
    remove_ln("uname");
    remove_ln("uniq");
    remove_ln("unix2dos");
    remove_ln("unlzop");
    remove_ln("unzip");
    remove_ln("uptime");
    remove_ln("usleep");
    remove_ln("uudecode");
    remove_ln("uuencode");
    remove_ln("watch");
    remove_ln("wc");
    remove_ln("which");
    remove_ln("whoami");
    remove_ln("xargs");
    remove_ln("yes");
    remove_ln("zcat");  
    ui_print("Done.");
}
    
void apply_root_to_device() {
    
    int freemb = 0;
    ui_print("\nInstalling Busybox, su and Superuser...\n");
    if (ensure_path_mounted("/system") != 0) {
        ui_print("Can't mount %s, exiting...\n", "/system");
        return -1;
        }
    
    scan_mounted_volumes();
    freemb=get_partition_free("/system");
    if( freemb < 2){
        ui_print("Free space on /system: %dMb\n",freemb);
        ui_print("Please ensure at least 2Mb of free space\n");
        ui_print("on your /system partition. You could try\n");
        ui_print("the CWM backup/cleanup options. Exiting...\n");
        }

    ui_print("Free space on /system: %dMb, proceeding...\n",freemb);
    
    // using "cat" here as stock toolbox cp is crippled...
    ui_print("Copying Busybox executable\n");
    __system("rm /system/xbin/busybox");
    __system("rm /system/bin/busybox");
    __system("rm /vendor/bin/busybox");
    __system("rm /system/sbin/busybox");
    __system("cat /sbin/recovery > /system/xbin/busybox");
    __system("chmod 755 /system/xbin/busybox");
    __system("chown 0.0 /system/xbin/busybox");

    ui_print("Creating xbin symlinks...\n");
    apply_ln("[");
    apply_ln("[[");
    apply_ln("ash");
    apply_ln("awk");
    apply_ln("basename");
    apply_ln("bbconfig");
    apply_ln("bunzip2");
    apply_ln("bzcat");
    apply_ln("bzip2");
    apply_ln("cal");
    apply_ln("cat");
    apply_ln("catv");
    apply_ln("chgrp");
    apply_ln("chmod");
    apply_ln("chown");
    apply_ln("chroot");
    apply_ln("cksum");
    apply_ln("clear");
    apply_ln("cmp");
    apply_ln("cp");
    apply_ln("cpio");
    apply_ln("cut");
    apply_ln("date");
    apply_ln("dc");
    apply_ln("dd");
    apply_ln("depmod");
    apply_ln("devmem");
    apply_ln("df");
    apply_ln("diff");
    apply_ln("dirname");
    apply_ln("dmesg");
    apply_ln("dos2unix");
    apply_ln("du");
    apply_ln("echo");
    apply_ln("egrep");
    apply_ln("env");
    apply_ln("expr");
    apply_ln("false");
    apply_ln("fdisk");
    apply_ln("fgrep");
    apply_ln("find");
    apply_ln("fold");
    apply_ln("free");
    apply_ln("freeramdisk");
    apply_ln("fuser");
    apply_ln("getopt");
    apply_ln("grep");
    apply_ln("gunzip");
    apply_ln("gzip");
    apply_ln("head");
    apply_ln("hexdump");
    apply_ln("id");
    apply_ln("insmod");
    apply_ln("install");
    apply_ln("kill");
    apply_ln("killall");
    apply_ln("killall5");
    apply_ln("length");
    apply_ln("less");
    apply_ln("ln");
    apply_ln("losetup");
    apply_ln("ls");
    apply_ln("lsmod");
    apply_ln("lspci");
    apply_ln("lsusb");
    apply_ln("lzop");
    apply_ln("lzopcat");
    apply_ln("md5sum");
    apply_ln("mkdir");
    apply_ln("mke2fs");
    apply_ln("mkfifo");
    apply_ln("mkfs.ext2");
    apply_ln("mknod");
    apply_ln("mkswap");
    apply_ln("mktemp");
    apply_ln("modprobe");
    apply_ln("more");
    apply_ln("mount");
    apply_ln("mountpoint");
    apply_ln("mv");
    apply_ln("nice");
    apply_ln("nohup");
    apply_ln("od");
    apply_ln("patch");
    apply_ln("pgrep");
    apply_ln("pidof");
    apply_ln("pkill");
    apply_ln("printenv");
    apply_ln("printf");
    apply_ln("ps");
    apply_ln("pwd");
    apply_ln("rdev");
    apply_ln("readlink");
    apply_ln("realpath");
    apply_ln("renice");
    apply_ln("reset");
    apply_ln("rm");
    apply_ln("rmdir");
    apply_ln("rmmod");
    apply_ln("run-parts");
    apply_ln("sed");
    apply_ln("seq");
    apply_ln("setsid");
    apply_ln("sh");
    apply_ln("sha1sum");
    apply_ln("sha256sum");
    apply_ln("sha512sum");
    apply_ln("sleep");
    apply_ln("sort");
    apply_ln("split");
    apply_ln("stat");
    apply_ln("strings");
    apply_ln("stty");
    apply_ln("swapoff");
    apply_ln("swapon");
    apply_ln("sync");
    apply_ln("sysctl");
    apply_ln("tac");
    apply_ln("tail");
    apply_ln("tar");
    apply_ln("tee");
    apply_ln("test");
    apply_ln("time");
    apply_ln("top");
    apply_ln("touch");
    apply_ln("tr");
    apply_ln("true");
    apply_ln("tty");
    apply_ln("tune2fs");
    apply_ln("umount");
    apply_ln("uname");
    apply_ln("uniq");
    apply_ln("unix2dos");
    apply_ln("unlzop");
    apply_ln("unzip");
    apply_ln("uptime");
    apply_ln("usleep");
    apply_ln("uudecode");
    apply_ln("uuencode");
    apply_ln("watch");
    apply_ln("wc");
    apply_ln("which");
    apply_ln("whoami");
    apply_ln("xargs");
    apply_ln("yes");
    apply_ln("zcat");

    ui_print("Copying su binary\n");
    __system("rm /system/bin/su");
    __system("rm /system/xbin/su");
    __system("rm /system/sbin/su");
    __system("rm /vendor/bin/su");
    __system("cat /res/misc/su > /system/xbin/su");
    __system("chown 0.0 /system/xbin/su");
    __system("chmod 4755 /system/xbin/su");

    ui_print("Copying Superuser.apk\n");
    __system("rm /system/app/Superuser.apk");
    __system("rm /data/app/Superuser.apk");
    __system("cat /res/misc/Superuser.apk > /system/app/Superuser.apk");
    __system("chown 0.0 /system/app/Superuser.apk");
    __system("chmod 644 /system/app/Superuser.apk");

  ui_print("Done.\n");
}

void show_root_menu() {
    static char* headers[] = {  "INSTALL/REMOVE ROOT and BUSYBOX",
                                "Be sure to have enough free space",
                                "space (~1Mb) on /system partition, use",
                                "cleanup options to free space if needed...",
                                NULL
    };

    static char* list[] = { "Install ROOT (su, Superuser, busybox)",
                            "Remove ROOT (su, Superuser, busybox)",
                            NULL
    };

    for (;;)
    {
        int chosen_item = get_menu_selection(headers, list, 0, 0);
        if (chosen_item == GO_BACK)
            break;
        switch (chosen_item)
        {
            case 0:
              if (confirm_selection("Confirm install ROOT","Yes - apply ROOT to device")) {
                apply_root_to_device();
              }
              break;
            case 1:
              if (confirm_selection("Confirm remove ROOT","Yes - remove ROOT from device")) {
                remove_root_from_device();
              }
              break;
        }
    }

}

int get_partition_free(const char *partition){
    if (ensure_path_mounted(partition) != 0){
        ui_print("Can't mount %s\n",partition);
        return -1;
    }
    int ret;
    struct statfs s;
    if (0 != (ret = statfs(partition, &s))){
        ui_print("Unable to stat %s\n",partition);
        return(-1);}
    uint64_t bavail = s.f_bavail;
    uint64_t bsize = s.f_bsize;
    uint64_t freecalc = bavail * bsize;
    uint64_t free_mb = freecalc / (uint64_t)(1024 * 1024);
    return free_mb;
    }

int file_exists(const char *filename){
    char tmp[PATH_MAX];
    struct stat file_info;
    sprintf(tmp, "%s", filename);
    return(statfs(tmp, &file_info));
}

int show_file_exists(const char *pre, const char *filename, const char *ui_filename, const char *post, const char *post_no){
    if(0 == file_exists(filename)){
        ui_print("%s %s %s\n",pre,ui_filename,post);
        return 1; 
        }
    ui_print("%s %s %s\n",pre,ui_filename,post_no);
    return 0;
    }

void custom_menu(
    const char* headers[],
    const char* list[],
    const int numtweaks,
    const char* options[],
    const char* conffile,
    int onlyone
    ) {
    int tweaks[numtweaks];
    int i;
    char buf[128];
    ensure_path_mounted("/system");
    for (;;)
    {
        for (i=0;i<numtweaks;i++) tweaks[i]=0;          // set all tweaks disabled
        FILE* f = fopen(conffile,"r");                  // open configfile
        ui_print("\n\nEnabled options:\n");
        if (f) {
          while (fgets(buf,127,f)) {                    // read all enabled options
            ui_print("%s",buf);
            if (onlyone!=1){                            // only if multiple options possible:
                for (i=0; i<numtweaks; i++) {           // enable options in tweaks array
                    if (memcmp(buf,options[i],strlen(options[i]))==0) tweaks[i]=1;
                }
            }
          }
          fclose(f);
        }                                               // done
        
        // start menu
        int chosen_item = get_menu_selection(headers, list, 0, 0);
        if (chosen_item == GO_BACK)
            break;
        tweaks[chosen_item] = tweaks[chosen_item]?0:1;  // toggle selected option

        if (memcmp("LOGGER",options[chosen_item],strlen(options[chosen_item]))==0){
            ensure_path_mounted("/data");
            if(tweaks[chosen_item]==0)
                __system("rm /data/local/logger.ko");        
            else
                __system("cp /lib/modules/logger.ko /data/local/logger.ko");        
        }

        f = fopen(conffile,"w+");                       // write options to config file
        if (f) {
          for (i=0; i<numtweaks; i++) {
            if (tweaks[i]) fprintf(f,"%s\n",options[i]);
          }
          fclose(f);
        } else {
          ui_print("Could not create config file\n");
        }
    }
}

void create_backup_dirs() {
  ensure_path_mounted("/data");
  __system("mkdir -p /data/midnight");
  __system("mkdir -p /data/midnight/backups");
  __system("mkdir -p /data/midnight/backups/sounds");
  __system("mkdir -p /data/midnight/backups/midnight-conf");
}

int apply_appbackup(void)
{
    char path[PATH_MAX] = "";
    char tmp[PATH_MAX] ="";
    char *fname = NULL;
    int numFiles = 0;
    int i = 0;
    unsigned int freemb = 0;
    unsigned int maxmb = 0;
    struct stat filedata;
        
    ui_print("\nStarting backup of /data/app...\n");

    // we need our source packages...
    LOGI("/data/app backup requested, mounting...\n");
    if(0 != ensure_path_mounted("/data")){
        ui_print("Failed to mount /data, exiting...\n");
        return 1;
    }

    // we need temp space to zipalign...
    if(0 != ensure_path_mounted("/sdcard")){
        ui_print("Failed to mount /sdcard, exiting...\n");
        return 1;
    }
            
    // get files
    char** files = gather_files("/data/app/",".apk", &numFiles);
    // bail out if nothing found
    if (numFiles <= 0)
    {
        ui_print("No files found, exiting.\n");
        return 1;
    }

    // create directory...   
    ui_print("Creating dir /sdcard/midnight_appbackup...\n");
    sprintf(tmp,"mkdir -p /sdcard/midnight_appbackup"); 
    if(0 != __system(tmp)){
        ui_print("Failed to to execute %s, exiting...\n",tmp);
        return 1;
    }
    
    char** list = (char**) malloc((numFiles + 1) * sizeof(char*));
    list[numFiles] = NULL;
    
    freemb=get_partition_free("/sdcard")*1048576; // free Mb in byte
    LOGI("Free Mb on /sdcard: %i\n",freemb);
    for (i = 0 ; i < numFiles; i++)
    {
        list[i] = strdup(files[i]);
        if (stat(list[i], &filedata) < 0) {
           LOGE("Error stat'ing %s: %s\n", list[i], strerror(errno));
           return 1;
        }
        LOGI("Filesize: %i\n",filedata.st_size);       
            maxmb = maxmb + filedata.st_size;
    }

    // check available space...
    if(maxmb >= freemb){
        ui_print("Not enough space on /sdcard, please\n");
        ui_print("free at least %i Mb, exiting...\n",(maxmb/1024/1024));
        return 1;        
    }    
        
    // let's go...    
    for (i = 0 ; i < numFiles; i++)
    {
        sprintf(tmp,"cp %s /sdcard/midnight_appbackup/",list[i]);
        fname = &list[i][10];
        ui_print("Backing up %s...\n",fname);
        if(0 != __system(tmp)){
            ui_print("Failed!\n",fname);        
        }
    }

    ui_print("Done.\n");
    free_string_array(list);
    free_string_array(files);
    return 0;
}

int apply_apprestore(void)
{
    char path[PATH_MAX] = "";
    char tmp[PATH_MAX] ="";
    int numFiles = 0;
    int i = 0;
    char *fname = NULL;
    unsigned int freemb = 0;
    unsigned int maxmb = 0;
    struct stat filedata;
        
    ui_print("\nStarting backup of /data/app...\n");

    // we need our target...
    LOGI("/data/app restore requested, mounting...\n");
    if(0 != ensure_path_mounted("/data")){
        ui_print("Failed to mount /data, exiting...\n");
        return 1;
    }

    // we need our packages...
    if(0 != ensure_path_mounted("/sdcard")){
        ui_print("Failed to mount /sdcard, exiting...\n");
        return 1;
    }
            
    // get files
    char** files = gather_files("/sdcard/midnight_appbackup/",".apk", &numFiles);
    // bail out if nothing found
    if (numFiles <= 0)
    {
        ui_print("No files found, exiting.\n");
        return 1;
    }

    // create directory...   
    ui_print("Creating dir /data/app...\n");
    sprintf(tmp,"mkdir -p /data/app"); 
    if(0 != __system(tmp)){
        ui_print("Failed to to execute %s, exiting...\n",tmp);
        return 1;
    }
    
    char** list = (char**) malloc((numFiles + 1) * sizeof(char*));
    list[numFiles] = NULL;
    
    freemb=get_partition_free("/data")*1048576; // free Mb in byte
    LOGI("Free Mb on /data: %i\n",freemb);
    for (i = 0 ; i < numFiles; i++)
    {
        list[i] = strdup(files[i]);
        if (stat(list[i], &filedata) < 0) {
           LOGE("Error stat'ing %s: %s\n", list[i], strerror(errno));
           return 1;
        }
        LOGI("Filesize: %i\n",filedata.st_size);       
            maxmb = maxmb + filedata.st_size;
    }
    
    // check available space...
    if(maxmb >= freemb){
        ui_print("Not enough space on /data, please\n");
        ui_print("free at least %i Mb, exiting...\n",(maxmb/1024/1024));
        return 1;        
    }    
        
    // let's go...    
    for (i = 0 ; i < numFiles; i++)
    {
        sprintf(tmp,"busybox install %s /data/app",list[i]);
        fname = &list[i][26];  
        ui_print("Installing %s...\n",fname);
        if(0 != __system(tmp)){
            ui_print("Failed!\n",fname);        
        }
    }

    ui_print("Done.\n");
    free_string_array(list);
    free_string_array(files);
    return 0;
}


int apply_zipalign(const char* directory)
{
    char path[PATH_MAX] = "";
    char tempdir[PATH_MAX] = "";
    char checkit[PATH_MAX] = "/res/misc/zipalign -c 4 %s";
    char doit[PATH_MAX] = "/res/misc/zipalign -f 4 %s /sdcard/midnight_zipalign%s";
    char copyit[PATH_MAX] = "cp -p /sdcard/midnight_zipalign%s %s";
    char removeit[PATH_MAX] = "rm /sdcard/midnight_zipalign%s";
    char chmodit[PATH_MAX] = "chmod 644 %s";
    char *fname = NULL;
    char cmd_zipalign[PATH_MAX] ="";
    int numFiles = 0;
    int i = 0;
    int zcount = 0;
    unsigned int freemb = 0;
    unsigned int maxmb = 0;
    struct stat filedata;
        
    ui_print("\nStarting zipaligning in %s\n",directory);

    // we need our source packages...
    if(strstr(directory,"/data") !=0 ){
        LOGI("/data zipalign requested, mounting...\n");
        if(0 != ensure_path_mounted("/data")){
            ui_print("Failed to mount /data, exiting...\n");
            return 1;
        }
    }
    if(strstr(directory,"/system") !=0 ){
        LOGI("/system zipalign requested, mounting...\n");
        if(0 != ensure_path_mounted("/system")){
            ui_print("Failed to mount /system, exiting...\n");
            return 1;
        }
    }    
    // we need temp space to zipalign...
    if(0 != ensure_path_mounted("/sdcard")){
        ui_print("Failed to mount /sdcard, exiting...\n");
        return 1;
    }
            
    // get files
    char** files = gather_files(directory,".apk", &numFiles);
    // bail out if nothing found
    if (numFiles <= 0)
    {
        ui_print("No files found, exiting.\n");
        return 1;
    }

    // create temp directory to zipalign...   
    ui_print("Creating temp dir /sdcard/midnight_zipalign...\n");
    sprintf(tempdir,"mkdir -p /sdcard/midnight_zipalign%s",directory); 
    if(0 != __system(tempdir)){
        ui_print("Failed to to execute %s, exiting...\n",tempdir);
        return 1;
    }
    
    char** list = (char**) malloc((numFiles + 1) * sizeof(char*));
    list[numFiles] = NULL;
    
    freemb=get_partition_free("/sdcard")*1048576; // free Mb in byte
    LOGI("Free Mb on /sdcard: %i\n",freemb);
    for (i = 0 ; i < numFiles; i++)
    {
        list[i] = strdup(files[i]);
        if (stat(list[i], &filedata) < 0) {
           LOGE("Error stat'ing %s: %s\n", list[i], strerror(errno));
           return 1;
        }
        LOGI("Filesize: %i\n",filedata.st_size);       
        if(maxmb < filedata.st_size){
            maxmb = filedata.st_size;
            LOGI("New max mb: %i\n",maxmb);
        }
    }
    
    // check available space...
    if(maxmb >= freemb){
        ui_print("Not enough space on /sdcard, please\n");
        ui_print("free at least %i byte, exiting...\n",maxmb);
        return 1;        
    }    
        
    // let's go...    
    for (i = 0 ; i < numFiles; i++)
    {
        sprintf(cmd_zipalign,checkit,list[i]);
        fname = &list[i][strlen(directory)];
        if(0 == __system(cmd_zipalign)){
            //ui_print("Skipping %s\n",list[i]);
            zcount = zcount +1;
        }else{
            ui_print("Zipaligning %s...\n",fname);        
            sprintf(cmd_zipalign,doit,list[i],list[i]);
            if(0 == __system(cmd_zipalign)){
                //ui_print("Align: success\n",list[i]);
                sprintf(cmd_zipalign,copyit,list[i],list[i]);
                if(0 == __system(cmd_zipalign)){
                    //ui_print("Copyback: success %s\n",list[i]);
                    sprintf(cmd_zipalign,chmodit,list[i]);
                    if(0 == __system(cmd_zipalign)){
                        //ui_print("Chmod 644: sucess\n",list[i]);
                    }else{
                        ui_print("Chmod 644 failed...\n");                    
                    }
                }else{
                    ui_print("Copying back failed, skipping...\n");
                }
                sprintf(cmd_zipalign,removeit,list[i]);
                __system(cmd_zipalign);       
            }else{
                ui_print("Zipaligning failed, skipping...\n");
                sprintf(cmd_zipalign,removeit,list[i]);
                __system(cmd_zipalign);       
            }
        }
    }

    ui_print("Removing temp dir /sdcard/midnight_zipalign...\n");
    sprintf(tempdir,"rm -r /sdcard/midnight_zipalign"); 
    if(0 != __system(tempdir)){
        ui_print("Failed to to remove /sdcard/midnight_zipalign, exiting...\n",tempdir);
        return 1;
    }    
    ui_print("Already zipaligned, skipped: %i\n",zcount);
    ui_print("Done.\n");
    free_string_array(list);
    free_string_array(files);
    return 0;
}

void show_zipalign_menu() {
    static char* headers[] = {  "ZIPALIGN",
                                "Zipalign packages in chosen directory",
                                "for faster loading and execution time...",
                                NULL
    };

    static char* list[] = { "Zipalign packages in /system/app",
                            "Zipalign packages in /system/framework",
                            "Zipalign packages in /data/app",
                            NULL
    };

    for (;;)
    {
        int chosen_item = get_menu_selection(headers, list, 0, 0);
        if (chosen_item == GO_BACK)
            break;
        switch (chosen_item)
        {
            case 0:
              if (confirm_selection("Confirm zipaligning in /system/app","Yes - zipalign in /system/app")) {
                apply_zipalign("/system/app/");
              }
              break;
            case 1:
              if (confirm_selection("Confirm zipaligning in /system/framework","Yes - zipalign in /system/framework")) {
                apply_zipalign("/system/framework/");
              }
              break;
            case 2:
              if (confirm_selection("Confirm zipaligning in /data/app","Yes - zipalign in /data/app")) {
                apply_zipalign("/data/app/");
              }
              break;
        }
    }

}

void show_misc_menu() {
    const char* h[]={
        "misc. options",
        "",
        "",
        NULL};
    const char* m[]={
        "init.d/userinit.d         [default:    off]",
        "NOOP IO scheduler         [default:    SIO]",
        "512Kb sdcard readahead    [default:  256Kb]",
        "TouchLED timeout 250ms    [default: 1600ms]",
        "BTHID module",
        "TUN module",
        "Logcat module",
        "enhanced touchscreen sensitivity",
        NULL};
    int num=11;
    const char* cnfv[]={"INITD","NOOP","512","LEDTIMEOUT","BTHID","TUN","LOGGER","TOUCHSCREEN"};
    const char* cnff="/data/local/midnight_options.conf";
    custom_menu(h,m,num,cnfv,cnff,0);
}

void show_uv_menu() {
    const char* h[]={
        "undervolting profiles",
        "",
        "",
        NULL};
    const char* m[]={
        " 0   0   0   0   0 mV [default]",
        " 0   0  25  50  75 mV",
        " 0   0  25  75 100 mV",
        " 0   0  50  75 125 mV",
        NULL};
    int num=4;
    const char* cnfv[]={"DEFAULT","UV1","UV2","UV3"};
    const char* cnff="/data/local/midnight_uv.conf";
    custom_menu(h,m,num,cnfv,cnff,1);
}

void show_gamma_menu() {
    const char* h[]={
        "screen gamma",
        "",
        "",
        NULL};
    const char* m[]={
        "stock/unchanged",
        "- 7 [default]",
        "-11",
        NULL};
    int num=3;
    const char* cnfv[]={"GAM1","GAM2","GAM3"};
    const char* cnff="/data/local/midnight_gamma.conf";
    custom_menu(h,m,num,cnfv,cnff,1);
}

void show_rgb_menu() {
    const char* h[]={
        "screen RGB profiles",
        "",
        "",
        NULL};
    const char* m[]={
        "unchanged [default]",
        "red- , green--",
        "red--, green-",
        NULL};
    int num=3;
    const char* cnfv[]={"RGB1","RGB2","RGB3"};
    const char* cnff="/data/local/midnight_rgb.conf";
    custom_menu(h,m,num,cnfv,cnff,1);
}

void show_governor_menu() {
    const char* h[]={
        "CPU governor",
        "",
        "",
        NULL};
    const char* m[]={
        "Smoove [default]",
        "SmartassV2",
        "Ondemand",
        "Conservative",
        NULL};
    int num=4;
    const char* cnfv[]={"smoove","smartassV2","ondemand","conservative"};
    const char* cnff="/data/local/midnight_gov.conf";
    custom_menu(h,m,num,cnfv,cnff,1);
}

void show_freq_menu() {
    const char* h[]={
        "CPU max. frequency",
        "",
        "",
        NULL};
    const char* m[]={
        " 800Mhz",
        "1000Mhz",
        "1128Mhz",
        NULL};
    int num=3;
    const char* cnfv[]={"800","1000","1128"};
    const char* cnff="/data/local/midnight_freq.conf";
    custom_menu(h,m,num,cnfv,cnff,1);
}

void mngb_menu() {
    static char* headers[] = {  "MNGB kernel settings",
                                "",
                                "",
                                NULL
    };

    static char* list[] = { "Root...",
                            "Zipalign...",
                            "Screen gamma profile",
                            "Screen RGB profile",
                            "CPU undervolting...",
                            "CPU max. frequency...",
                            "CPU governor...",
                            "Options...",
                            NULL
    };

    for (;;)
    {
        int chosen_item = get_menu_selection(headers, list, 0, 0);
        if (chosen_item == GO_BACK)
            break;
        switch (chosen_item)
        {
            case 0:
                show_root_menu();
              break;
            case 1:
                show_zipalign_menu();
              break;
            case 2:
                show_gamma_menu();
              break;
            case 3:
                show_rgb_menu();
              break;
            case 4:
                show_uv_menu();
              break;
            case 5:
                show_freq_menu();
              break;
            case 6:
                show_governor_menu();
              break;
            case 7:
                show_misc_menu();
              break;
        }
    }

}

