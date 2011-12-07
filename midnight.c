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

void lmk_preset_menu() {
    const char* h[]={   
        "CONFIGURE LOWMEMORYKILLER VALUES",
        "",
        "",
        NULL};
    const char* m[]={   
        "[0] 8,16,46,55,60,70Mb (MIDNIGHT)",
        "[1] 8,12,16,24,28,32Mb (NEXUS S)",
		"[2] 8,16,24,28,36,42Mb",
		"[3] 8,16,24,36,48,56Mb",
		"[4] 8,16,44,48,56,68Mb",
        "[5] 8,16,46,56,68,88Mb",
        "[6] 8,16,46,62,74,96Mb",
        NULL};
    int num=7;
    const char* cnfv[]={"LMK0","LMK1","LMK2","LMK3","LMK4","LMK5","LMK6" };
    const char* cnff="/system/etc/midnight_lmk.conf";
    custom_menu(h,m,num,cnfv,cnff,1);
}

void lmk_slot_mb_menu(int slot) {
    int num=20;
    const char* cnfv[]={"001","002","003","004","005","006","007","008","009","010","020","030","040","050","060","070","080","090","100","NOOVERRIDE"};
    const char* m[]={   
        "001",
        "002",
        "003",
        "004",
        "005",
        "006",
        "007",
        "008",
        "009",
        "010",
        "020",
        "030",
        "040",
        "050",
        "060",
        "070",
        "080",
        "090",
        "100",
        "don't override PRESET value",        
        NULL};

    if (slot == 1){
        const char* h[]={   
            "SLOT1: SELECT LOWMEMORYKILLER VALUE",
            "multiple selections possible, values will be",
            "added (e.g. 5+20=25Mb)...",
            NULL};
        const char* cnff="/system/etc/midnight_lmk_slot1.conf";
        custom_menu(h,m,num,cnfv,cnff,0);
    }
    if (slot == 2){
        const char* h[]={   
            "SLOT2: SELECT LOWMEMORYKILLER VALUE",
            "multiple selections possible, values will be",
            "added (e.g. 5+20=25Mb)...",
            NULL};
        const char* cnff="/system/etc/midnight_lmk_slot2.conf";
        custom_menu(h,m,num,cnfv,cnff,0);
    }
    if (slot == 3){
        const char* h[]={   
            "SLOT3: SELECT LOWMEMORYKILLER VALUE",
            "multiple selections possible, values will be",
            "added (e.g. 5+20=25Mb)...",
            NULL};
        const char* cnff="/system/etc/midnight_lmk_slot3.conf";
        custom_menu(h,m,num,cnfv,cnff,0);
    }
    if (slot == 4){
        const char* h[]={   
            "SLOT4: SELECT LOWMEMORYKILLER VALUE",
            "multiple selections possible, values will be",
            "added (e.g. 5+20=25Mb)...",
            NULL};
        const char* cnff="/system/etc/midnight_lmk_slot4.conf";
        custom_menu(h,m,num,cnfv,cnff,0);
    }
    if (slot == 5){
        const char* h[]={   
            "SLOT5: SELECT LOWMEMORYKILLER VALUE",
            "multiple selections possible, values will be",
            "added (e.g. 5+20=25Mb)...",
            NULL};
        const char* cnff="/system/etc/midnight_lmk_slot5.conf";
        custom_menu(h,m,num,cnfv,cnff,0);
    }
    if (slot == 6){
        const char* h[]={   
            "SLOT6: SELECT LOWMEMORYKILLER VALUE",
            "multiple selections possible, values will be",
            "added (e.g. 5+20=25Mb)...",
            NULL};
        const char* cnff="/system/etc/midnight_lmk_slot6.conf";
        custom_menu(h,m,num,cnfv,cnff,0);
    }
}

void modules_menu() {
    const char* h[]={   
        "CONFIGURE MISC./MODULE LOADING",
        "",
        "",
        NULL};
    const char* m[]={   
        "Toggle CIFS module loading (filesystem)",
		"Toggle LOGGER module loading (logcat)",
		"Toggle TUN module loading (VPN)",
        "Toggle IPv4/IPv6 security",
        "Toggle init.d support",
        "Toggle touchscreen sensitivity +",
        "Toggle NO-autoROOT",
        "Toggle NO-MidnightTweaks",
        NULL};
    int num=8;
    const char* cnfv[]={"CIFS","ANDROIDLOGGER","TUN", "IPV6PRIVACY", "INIT_D", "TOUCH", "NOAUTOROOT", "NOTWEAKS" };
    const char* cnff="/system/etc/midnight_misc.conf";
    custom_menu(h,m,num,cnfv,cnff,0);
}

void gfx_menu() {
    const char* h[]={   
        "CONFIGURE GFX OPTIONS",
        "",
        "",
        NULL};
    const char* m[]={   
        "Enable color mode MIDNIGHT [default]",
        "Enable color mode COLD",
        "Enable color mode WARM",
        NULL};
    int num=3;
    const char* cnfv[]={"MIDNIGHT","COLD","WARM"};
    const char* cnff="/system/etc/midnight_gfx.conf";
    custom_menu(h,m,num,cnfv,cnff,1);
}

void readahead_menu() {
    const char* h[]={   
        "SELECT SDCARD READ_AHEAD VALUE",
        "Default=512kB, higher values may result",
        "in faster sdcard read actions.",
        NULL};
    const char* m[]={   
        "[0] 64kB",
        "[1] 128kB",
        "[2] 256kB",
        "[3] 512kB [default]",
        "[4] 1024kB",
        "[5] 2048kB",
        "[6] 3064kB",
        "[7] 4096kB",
        NULL};
    int num=8;
    const char* cnfv[]={"READAHEAD_64","READAHEAD_128","READAHEAD_256","READAHEAD_512","READAHEAD_1024","READAHEAD_2048","READAHEAD_3064","READAHEAD_4096"};
    const char* cnff="/system/etc/midnight_rh.conf";
    custom_menu(h,m,num,cnfv,cnff,1);
}
void create_backup_dirs() {
  ensure_path_mounted("/data");
  __system("mkdir -p /data/midnight");
  __system("mkdir -p /data/midnight/backups");
  __system("mkdir -p /data/midnight/backups/initd");
  __system("mkdir -p /data/midnight/backups/sounds");
  __system("mkdir -p /data/midnight/backups/theme");
  __system("mkdir -p /data/midnight/backups/midnight-conf");
  __system("mkdir -p /data/midnight/backups/bootanimation");
  __system("mkdir -p /data/midnight/backups/bootanimation/local");
  __system("mkdir -p /data/midnight/backups/bootanimation/system");
  //__system("mkdir -p /data/midnight/backups/voltage-control");
  //__system("mkdir -p /data/midnight/backups/localprop");
}

void cpu_max_menu() {
    const char* h[]={   
        "SELECT CPU MAX FREQUENCY",
        "Limit your CPU frequency to preserve",
        "battery life. Brackets = (arm.volt/internal volt).",
        NULL};
    const char* m[]={   
        "400Mhz",
        "800Mhz",
        "1000Mhz [default]",
        "1200Mhz",
        NULL};
    int num=4;
    const char* cnfv[]={"CPU_MAX_400","CPU_MAX_800","CPU_MAX_1000","CPU_MAX_1200"};
    const char* cnff="/system/etc/midnight_cpu_max.conf";
    custom_menu(h,m,num,cnfv,cnff,1);
}

void cpu_gov_menu() {
    const char* h[]={   
        "SELECT CPU GOVERNOR",
        "Change CPU frequency scaling method...",
        "",
        NULL};
    const char* m[]={   
        "CONSERVATIVE -> less battery [default]",
        "ONDEMAND     -> more responsive",
        NULL};
    int num=2;
    const char* cnfv[]={"CPU_GOV_CONSERVATIVE","CPU_GOV_ONDEMAND"};
    const char* cnff="/system/etc/midnight_cpu_gov.conf";
    custom_menu(h,m,num,cnfv,cnff,1);
}

void IO_sched_menu() {
    const char* h[]={   
        "SELECT IO SCHEDULER",
        "",
        "",
        NULL};
    const char* m[]={   
        "NOOP [default]",
        "SIO",
        "VR",
        "CFQ",
        "DEADLINE",
        NULL};
    int num=5;
    const char* cnfv[]={"IO_SCHED_NOOP","IO_SCHED_SIO","IO_SCHED_VR","IO_SCHED_CFQ","IO_SCHED_DEADLINE"};
    const char* cnff="/system/etc/midnight_io_sched.conf";
    custom_menu(h,m,num,cnfv,cnff,1);
}

void cpu_uv_freq_menu(int freq) {
    int num=13;
    const char* cnfv[]={"000","005","010","020","030","040","050","060","070","080","090","100","NOOVERRIDE"};
    const char* m[]={   
        "000 [default]",
        "005",
        "010",
        "020",
        "030",
        "040",
        "050",
        "060",
        "070",
        "080",
        "090",
        "100",
        "don't override PRESET value",        
        NULL};

    if (freq == 100){
        const char* h[]={   
            "100MHZ: SELECT CPU UNDERVOLTING VALUES",
            "multiple selections possible, values will be",
            "added (e.g. (-5)+(-20)=-25mV...",
            NULL};
        const char* cnff="/system/etc/midnight_cpu_uv_100.conf";
        custom_menu(h,m,num,cnfv,cnff,0);
    }
    if (freq == 200){
        const char* h[]={   
            "200MHZ: SELECT CPU UNDERVOLTING VALUES",
            "multiple selections possible, values will be",
            "added (e.g. (-5)+(-20)=-25mV...",
            NULL};
        const char* cnff="/system/etc/midnight_cpu_uv_200.conf";
        custom_menu(h,m,num,cnfv,cnff,0);
    }
    if (freq == 400){
        const char* h[]={   
            "400MHZ: SELECT CPU UNDERVOLTING VALUES",
            "multiple selections possible, values will be",
            "added (e.g. (-5)+(-20)=-25mV...",
            NULL};
        const char* cnff="/system/etc/midnight_cpu_uv_400.conf";
        custom_menu(h,m,num,cnfv,cnff,0);
    }
    if (freq == 800){
        const char* h[]={   
            "800MHZ: SELECT CPU UNDERVOLTING VALUES",
            "multiple selections possible, values will be",
            "added (e.g. (-5)+(-20)=-25mV...",
            NULL};
        const char* cnff="/system/etc/midnight_cpu_uv_800.conf";
        custom_menu(h,m,num,cnfv,cnff,0);
    }
    if (freq == 1000){
        const char* h[]={   
            "1000Mhz: SELECT CPU UNDERVOLTING VALUES",
            "multiple selections possible, values will be",
            "added (e.g. (-5)+(-20)=-25mV...",
            NULL};
        const char* cnff="/system/etc/midnight_cpu_uv_1000.conf";
        custom_menu(h,m,num,cnfv,cnff,0);
    }
    if (freq == 1200){
        const char* h[]={   
            "1200Mhz: SELECT CPU UNDERVOLTING VALUES",
            "multiple selections possible, values will be",
            "added (e.g. (-5)+(-20)=-25mV...",
            NULL};
        const char* cnff="/system/etc/midnight_cpu_uv_1200.conf";
        custom_menu(h,m,num,cnfv,cnff,0);
    }
}

void cpu_uv_menu() {
    const char* h[]={   
        "SELECT CPU UNDERVOLTING mV VALUES",
        "Lower values can save battery, too low",
        "values can cause system instability...",
        "Values:1200/1000/800/400/200/100Mhz.",
        "MANUAL SETTINGS OVERRIDE PRESETS.",
        NULL};
    const char* m[]={   
        "[ 0]  0/ 0/ 0/  0/  0/  0",
        "[ 1]  0/ 0/ 0/ 25/ 25/ 50",
        "[ 2]  0/ 0/25/ 25/ 50/ 50",
        "[ 3]  0/ 0/25/ 25/ 50/100",
        "[ 4]  0/ 0/25/ 50/100/100",
        "[ 5]  0/ 0/25/ 75/100/125",
        "[ 6]  0/ 0/25/ 50/100/125",
        "[ 7]  0/ 0/25/ 50/125/125",
        "[ 8]  0/ 0/25/100/125/150",
        "[ 9]  0/ 0/50/100/125/150",
        "[10]  0/15/50/ 50/100/125",
        "[11]  5/25/50/ 75/125/150",
        "[12] 10/15/50/ 75/125/150",
        "[13] 15/25/50/ 75/125/150",
        "[14] 10/15/50/ 75/125/175",
        "[15] 15/25/75/100/150/175",
        NULL};
    int num=16;
    const char* cnfv[]={"CPU_UV_0","CPU_UV_1","CPU_UV_2","CPU_UV_3","CPU_UV_4","CPU_UV_5","CPU_UV_6","CPU_UV_7","CPU_UV_8","CPU_UV_9","CPU_UV_10","CPU_UV_11","CPU_UV_12","CPU_UV_13","CPU_UV_14","CPU_UV_15"};
    const char* cnff="/system/etc/midnight_cpu_uv.conf";
    custom_menu(h,m,num,cnfv,cnff,1);
}

void cpu_menu() {
    static char* headers[] = {  "CPU OPTIONS",
                                "Configure CPU max. frequency, governor and",
                                "customize undervolting values for better "
                                "battery life and device temperature...",
                                NULL
    };
    static char* list[] = { "Select CPU max. frequency...",
                            "Select CPU governor...",
                            "Select CPU undervolting preset...",
                            "Setup CPU undervolting 1200Mhz...",
                            "Setup CPU undervolting 1000Mhz...",
                            "Setup CPU undervolting 800Mhz...",
                            "Setup CPU undervolting 400Mhz...",
                            "Setup CPU undervolting 200Mhz...",
                            "Setup CPU undervolting 100Mhz...",
                            "Reset CPU undervolting defaults...",
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
              {
                cpu_max_menu();
                break;
              }
              case 1:
              {
                cpu_gov_menu();
                break;
              }
              case 2:
              {
                cpu_uv_menu();
                break;
              }
              case 3:
              {
                cpu_uv_freq_menu(1200);
                break;
              }
              case 4:
              {
                cpu_uv_freq_menu(1000);
                break;
              }
              case 5:
              {
                cpu_uv_freq_menu(800);
                break;
              }
              case 6:
              {
                cpu_uv_freq_menu(400);
                break;
              }
              case 7:
              {
                cpu_uv_freq_menu(200);
                break;
              }
               case 8:
              {
                cpu_uv_freq_menu(100);
                break;
              }
               case 9:
              {
                if (confirm_selection("Confirm deleting undervolting settings","Yes - delete ALL undervolting settings")) {  
                  ensure_path_mounted("/system");
                  ui_print("\nDeleting stored preset...\n");
                  __system("rm -rf /system/etc/midnight_cpu_uv.conf");                
                  ui_print("Deleting stored 1300Mhz value...\n");
                  __system("rm -rf /system/etc/midnight_cpu_uv_1300.conf");                
                  ui_print("Deleting stored 1200Mhz value...\n");
                  __system("rm -rf /system/etc/midnight_cpu_uv_1200.conf");                
                  ui_print("Deleting stored 1000Mhz value...\n");
                  __system("rm -rf /system/etc/midnight_cpu_uv_1000.conf");                
                  ui_print("Deleting stored 800Mhz value...\n");
                  __system("rm -rf /system/etc/midnight_cpu_uv_800.conf");                
                  ui_print("Deleting stored 400Mhz value...\n");
                  __system("rm -rf /system/etc/midnight_cpu_uv_400.conf");                
                  ui_print("Deleting stored 200Mhz value...\n");
                  __system("rm -rf /system/etc/midnight_cpu_uv_200.conf");                
                  ui_print("Deleting stored 100Mhz value...\n");
                  __system("rm -rf /system/etc/midnight_cpu_uv_100.conf");                
                  ui_print("Done.\n");
                }
                break;
              }
       }
    }
}
void lmk_menu() {
    static char* headers[] = {  "LOWMEMORYKILLER OPTIONS",
                                "Select LMK preset or build own value which",
                                "may override preset individual values...",
                                NULL
    };
    static char* list[] = { "Select LMK preset",
                            "Setup LMK Slot 1 value (Mb)",
                            "Setup LMK Slot 2 value (Mb)",
                            "Setup LMK Slot 3 value (Mb)",
                            "Setup LMK Slot 4 value (Mb)",
                            "Setup LMK Slot 5 value (Mb)",
                            "Setup LMK Slot 6 value (Mb)",
                            "Reset LMK default settings",
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
              {
				lmk_preset_menu();
                break;
              }
              case 1:
              {
			    lmk_slot_mb_menu(1);
                break;
              }
              case 2:
              {
			    lmk_slot_mb_menu(2);
                break;
              }
              case 3:
              {
			    lmk_slot_mb_menu(3);
                break;
              }
              case 4:
              {
			    lmk_slot_mb_menu(4);
                break;
              }
              case 5:
              {
			    lmk_slot_mb_menu(5);
                break;
              }
              case 6:
              {
			    lmk_slot_mb_menu(6);
                break;
              }
               case 7:
              {
                if (confirm_selection("Confirm deleting LMK settings","Yes - delete ALL LMK settings")) {  
                  ensure_path_mounted("/system");
                  ui_print("\nDeleting stored preset...\n");
                  __system("rm -rf /system/etc/midnight_lmk.conf");                
                  ui_print("Deleting stored slot1 value...\n");
                  __system("rm -rf /system/etc/midnight_lmk_slot1.conf");                
                  ui_print("Deleting stored slot2 value...\n");
                  __system("rm -rf /system/etc/midnight_lmk_slot2.conf");                
                  ui_print("Deleting stored slot3 value...\n");
                  __system("rm -rf /system/etc/midnight_lmk_slot3.conf");                
                  ui_print("Deleting stored slot4 value...\n");
                  __system("rm -rf /system/etc/midnight_lmk_slot4.conf");                
                  ui_print("Deleting stored slot5 value...\n");
                  __system("rm -rf /system/etc/midnight_lmk_slot5.conf");                
                  ui_print("Deleting stored slot6 value...\n");
                  __system("rm -rf /system/etc/midnight_lmk_slot6.conf");                
                  ui_print("Done.\n");
                }
                break;
              }

        }
    }
}

void show_advanced_menu() {
    static char* headers[] = {  "MIDNIGHT ADVANCED OPTIONS",
                                NULL
    };
    static char* list[] = { "CPU options...",
                            "IO scheduler options...",
                            "READ_AHEAD options...",
                            "Lowmemorykiller options...",
                            "Video driver options...",
                            "Misc. options/modules loading...",
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
              {
                cpu_menu();
                break;
              }
              case 1:
              {
                IO_sched_menu();
                break;
              }
              case 2:
              {
                readahead_menu();
                break;
              }
              case 3:
              {
                lmk_menu();
                break;
              }
              case 4:
              {
                gfx_menu(1300);
                break;
              }
              case 5:
              {
                modules_menu(1200);
                break;
              }
       }
    }
}
