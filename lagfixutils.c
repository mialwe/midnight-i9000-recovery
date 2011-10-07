/* Copyright (C) 2010 Zsolt Sz Sztupák
 * Modified by HardCORE (Rodney Clinton Chua)
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

#include "bootloader.h"
#include "common.h"
#include "cutils/properties.h"
#include "install.h"
#include "minui/minui.h"
#include "minzip/DirUtil.h"
#include "roots.h"
#include "recovery_ui.h"

#include "extendedcommands.h"
#include "commands.h"
#include "lagfixutils.h"
#include "nandroid.h"

extern char **environ;

void apply_ln(char* name) {
  char tmp[128];
  sprintf(tmp,"ln -s /system/xbin/busybox /system/xbin/%s",name);
  __system(tmp);
}

void apply_rm(char* name) {
  char tmp[128];
  sprintf(tmp,"/system/xbin/rm /system/bin/%s",name);
  __system(tmp);
}

void remove_root_from_device(void){
  ensure_root_path_mounted("SYSTEM:");
  ensure_root_path_mounted("DATA:");

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

    }
void apply_root_to_device(int mode) {
  ensure_root_path_mounted("SYSTEM:");
  ui_print("Copying Superuser and Busybox files to /system/xbin\n");

  ui_print("Copying su binary\n");
  __system("rm /system/bin/su");
  __system("rm /system/xbin/su");
  __system("rm /system/sbin/su");
  __system("rm /vendor/bin/su");
  __system("cp /res/misc/su /system/xbin/su");
  __system("chown 0.0 /system/xbin/su");
  __system("chmod 4755 /system/xbin/su");

  ui_print("Copying Superuser.apk\n");
  __system("rm /system/app/Superuser.apk");
  __system("rm /data/app/Superuser.apk");
  __system("cp /res/misc/Superuser.apk /system/app/Superuser.apk");
  __system("chown 0.0 /system/app/Superuser.apk");
  __system("chmod 644 /system/app/Superuser.apk");

  ui_print("Copying Busybox executable\n");
  __system("rm /system/xbin/busybox");
  __system("rm /system/bin/busybox");
  __system("rm /vendor/bin/busybox");
  __system("rm /system/sbin/busybox");
  __system("cp /sbin/recovery /system/xbin/busybox");
  __system("chmod 755 /system/xbin/busybox");

  ui_print("Creating symlinks to commands\n");

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

  if (mode>0) {
    ui_print("Removing some toolbox commands");
    apply_rm("cat");
    apply_rm("chmod");
    apply_rm("chown");
    apply_rm("dd");
    apply_rm("ln");
    apply_rm("ls");
    apply_rm("mkdir");
    apply_rm("mv");
    apply_rm("rm");
    apply_rm("rmdir");
    if (mode>1) {
      ui_print("Removing even more toolbox commands");
      apply_rm("cmp");
      apply_rm("date");
      apply_rm("df");
      apply_rm("dmesg");
      apply_rm("id");
      apply_rm("insmod");
      apply_rm("kill");
      apply_rm("lsmod");
      apply_rm("mount");
      apply_rm("printenv");
      apply_rm("ps");
      apply_rm("renice");
      apply_rm("sleep");
      apply_rm("sync");
      apply_rm("top");
      apply_rm("umount");
    }
  }
  ui_print("Done!\n");
}

static char* startval[] = {"DATA_FS=","CACHE_FS=","DBDATA_FS=","DATA_LOOP=","CACHE_LOOP=","DBDATA_LOOP=","BIND_DATA_TO_DBDATA=", NULL};
static int catvals[] = { 3,6,7,999 };
static char* categories[][8] = {
    {"rfs","jfs","ext4nj","ext4","ext2","ext3",NULL,NULL},
    {"false","ext2",NULL,NULL},
    {"false","data",NULL,NULL},
    {NULL,NULL,NULL,NULL}
  };


void lagfix_system_menu() {
    static char* headers[] = {  "Convert /system partition?",
								"Conversion will be done at next boot",
                                "",
                                NULL
    };

    static char* list[] = { "YES",
							"NO",
                            NULL
    };

    for (;;)
    {
		int chosen_item = get_menu_selection(headers, list, 0);
		if (chosen_item == GO_BACK) {
			break;
		} else {
			switch (chosen_item)
			{
				case 0:
					__system("touch /system/etc/lagfixsystem.conf");
					ui_print("\nSYSTEM conversion ON\n/system will be converted on next boot\n");
					//sync();
					//__reboot(LINUX_REBOOT_MAGIC1, LINUX_REBOOT_MAGIC2, LINUX_REBOOT_CMD_RESTART2, "recovery");
				break;
				case 1:
					__system("rm -f /system/etc/lagfixsystem.conf");
					ui_print("\nSYSTEM conversion OFF\n/system will not be converted\n");
				break;
			}
			break; //go back
		}
	}
}

int searchfor_in_config_file(const char* searchfor, int category) {
  int res = -1;
  char buf[64]; 
  FILE *f = fopen("/system/etc/lagfix.conf.old","r");
  if (!f) return -1;
  while (fgets(buf,63,f)) {
    if (memcmp(searchfor,buf,strlen(searchfor))==0) {
      res++;
      while (categories[category][res] && (memcmp(buf+strlen(searchfor),categories[category][res],strlen(categories[category][res]))!=0)) {
        res++;
      }
      if (!categories[category][res]) res=-1;
    }
  }
  fclose(f);
  return res;
}

int get_loop_options(const char * name) {
  char searchfor[64];
  sprintf(searchfor,"%s_LOOP=",name);
  return searchfor_in_config_file(searchfor,1);
}

int get_fs_options(const char * name) {
  char searchfor[64];
  sprintf(searchfor,"%s_FS=",name);
  return searchfor_in_config_file(searchfor,0);
}

int dont_use_bind_options = 0;
int get_bind_options() {
  if (dont_use_bind_options) return 0;
  return searchfor_in_config_file("BIND_DATA_TO_DBDATA=",2);
}

void mount_block(const char* name, const char* blockname, const char* loopblock, const char* destnoloop, const char* destloop) {
  int getfsopts,getloopopts,bindopts;
  getfsopts = get_fs_options(name);
  getloopopts = get_loop_options(name);
  char tmp[256];
  if (getloopopts>0) {
    sprintf(tmp,"mkdir %s",destloop);__system(tmp);
    sprintf(tmp,"chmod 700 %s",destloop);__system(tmp);
    if (getfsopts==0) {
      sprintf(tmp,"mount -t rfs -o nosuid,nodev,check=no %s %s",blockname,destloop);
    } else if (getfsopts==1) {
      sprintf(tmp,"mount -t jfs -o noatime %s %s",blockname,destloop);
    } else if (getfsopts==2) {
      sprintf(tmp,"mount -t ext4 -o noatime %s %s",blockname,destloop);
    } else if (getfsopts==3) {
      sprintf(tmp,"mount -t ext4 -o noatime %s %s",blockname,destloop);
	} else if (getfsopts==4) {
      sprintf(tmp,"mount -t ext2 -o noatime %s %s",blockname,destloop);
	} else if (getfsopts==5) {
      sprintf(tmp,"mount -t ext3 -o noatime %s %s",blockname,destloop);
    } else if (getfsopts==-1) {
      // rfs can't autodetect itself
      sprintf(tmp,"mount -t rfs -o nosuid,nodev,check=no %s %s",blockname,destnoloop);
      sprintf(tmp,"mount %s %s",blockname,destloop);
    }
    __system(tmp);
    sprintf(tmp,"losetup %s %s/.extfs",loopblock,destloop);__system(tmp);
    sprintf(tmp,"mount -t ext2 %s %s",loopblock,destnoloop);__system(tmp);
  } else {
    if (getfsopts==0) {
      sprintf(tmp,"mount -t rfs -o nosuid,nodev,check=no %s %s",blockname,destnoloop);
    } else if (getfsopts==1) {
      sprintf(tmp,"mount -t jfs -o noatime %s %s",blockname,destnoloop);
    } else if (getfsopts==2) {
      sprintf(tmp,"mount -t ext4 -o noatime %s %s",blockname,destnoloop);
    } else if (getfsopts==3) {
      sprintf(tmp,"mount -t ext4 -o noatime %s %s",blockname,destnoloop);
    } else if (getfsopts==4) {
      sprintf(tmp,"mount -t ext2 -o noatime %s %s",blockname,destnoloop);
    } else if (getfsopts==5) {
      sprintf(tmp,"mount -t ext3 -o noatime %s %s",blockname,destnoloop);
    } else if (getfsopts==-1) {
      // rfs can't autodetect itself
      sprintf(tmp,"mount -t rfs -o nosuid,nodev,check=no %s %s",blockname,destnoloop);
      sprintf(tmp,"mount %s %s",blockname,destnoloop);
    }
    __system(tmp);
  }
}

// there should be some checks whether the action actually succeded
int ensure_lagfix_mount_points(const RootInfo *info) {
  int bindopts;
  bindopts = get_bind_options();
  if (strcmp(info->name,"DATA:")==0) {
    mount_block("DATA","/dev/block/mmcblk0p2","/dev/block/loop1","/data","/res/odata");
    if (bindopts>0) {
      ensure_root_path_mounted("DATADATA:");
      __system("mkdir -p /dbdata/.data/data");
      __system("mkdir -p /data/data");
      __system("mount -o bind /dbdata/.data/data /data/data");
    }
  } else if (strcmp(info->name,"DATADATA:")==0) {
    mount_block("DBDATA","/dev/block/stl10","/dev/block/loop2","/dbdata","/res/odbdata");
  } else if (strcmp(info->name,"CACHE:")==0) {
    mount_block("CACHE","/dev/block/stl11","/dev/block/loop3","/cache","/res/ocache");
  } else {
    return 1;
  }
  return 0;
}

// not implemented
int ensure_lagfix_unmount_points(const RootInfo *info) {
  if (strcmp(info->name,"DATA:")==0) {
    return -1;
  } else if (strcmp(info->name,"DATADATA:")==0) {
    return -1;
  } else if (strcmp(info->name,"CACHE:")==0) {
    return -1;
  } else return 1;
}

int ensure_lagfix_formatted(const RootInfo *info) {
  // we won't remove hidden files in root yet
  if (strcmp(info->name,"DATA:")==0) {
    __system("rm -rf /data/*");
    __system("rm -rf /data/.*");
    return 0;
  } else if (strcmp(info->name,"DATADATA:")==0) {
    __system("rm -rf /dbdata/*");
    if (!get_bind_options()) {
      __system("rm -rf /dbdata/.*");
    }
    return 0;
  } else if (strcmp(info->name,"CACHE:")==0) {
    __system("rm -rf /cache/*");
    __system("rm -rf /cache/.*");
    return 0;
  } else return 1;
}

int unmount_all_lagfixed() {
  sync();
  __system("umount -f /data/data");
  __system("umount -f /data/dalvik-cache");
  __system("umount -f -d /dev/block/loop3");
  __system("umount -f -d /dev/block/loop2");
  __system("umount -f -d /dev/block/loop1");
  __system("umount -f /res/ocache");
  __system("umount -f /res/odata");
  __system("umount -f /res/odbdata");
  __system("umount -f /cache");
  __system("umount -f /data");
  __system("umount -f /dbdata");
  return 0;
}

int create_lagfix_partition(int id) {
  char loopname[64],blockname[64];
  char looppos[64],blockpos[64];
  char name[64];
  int loopsize;
  if (id==0) {
    strcpy(loopname,"/dev/block/loop1");
    strcpy(blockname,"/dev/block/mmcblk0p2");
    strcpy(looppos,"/res/odata");
    strcpy(blockpos,"/data");
    strcpy(name,"DATA");
    loopsize = 1831632896;
  } else if (id==1) {
    strcpy(loopname,"/dev/block/loop2");
    strcpy(blockname,"/dev/block/stl10");
    strcpy(looppos,"/res/odbdata");
    strcpy(blockpos,"/dbdata");
    strcpy(name,"DBDATA");
    loopsize = 104857600; //reduced for 513 pit
  } else {
    strcpy(loopname,"/dev/block/loop3");
    strcpy(blockname,"/dev/block/stl11");
    strcpy(looppos,"/res/ocache");
    strcpy(blockpos,"/cache");
    strcpy(name,"CACHE");
    loopsize = 29720576;
  }
  int ft = get_fs_options(name);
  int loop = get_loop_options(name);
  char tmp[256];
  char inodes[64];
  if (id==0) {
	strcpy(inodes,"50000");
  } else if (id==1) {
	strcpy(inodes,"20000");
  } else if (id==2) {
	strcpy(inodes,"2000");
  }
  if (ft==0) {
    if (id==0) {
      sprintf(tmp,"/sbin/fat.format -S 4096 -s 4 -F 32 %s",blockname);
      // we can't create small partitions that are valid as rfs with fat.format, so we'll use some compressed pre-made valid rfs images
    } else {
      // EDIT: it seems we can
      sprintf(tmp,"/sbin/fat.format -S 4096 -s 1 -F 16 %s",blockname);
    }
    __system(tmp);
  } else if (ft==1) {
    sprintf(tmp,"/sbin/mkfs.jfs -L %s %s",name,blockname);
    __system(tmp);
  } else if (ft==2) {
    sprintf(tmp,"/sbin/mkfs.ext4 -O ^has_journal -L %s -N %s -b 4096 -m 0 -F %s",name,inodes,blockname);
    __system(tmp);
  } else if (ft==3) {
    sprintf(tmp,"/sbin/mkfs.ext4 -L %s -b 4096 -N %s -m 0 -F %s",name,inodes,blockname);
    __system(tmp);
  } else if (ft==4) {
    sprintf(tmp,"/sbin/mkfs.ext2 -L %s -b 4096 -m 0 -F %s",name,blockname);
    __system(tmp);
  } else if (ft==5) {
    sprintf(tmp,"/sbin/mkfs.ext3 -L %s -b 4096 -m 0 -F %s",name,blockname);
    __system(tmp);
  }  

  if (loop) {
    if (ft==0) {
		sprintf(tmp,"mount -t rfs -o nosuid,nodev,check=no %s %s",blockname,looppos);
    } else {
		sprintf(tmp,"mount %s %s",blockname,looppos);
	}
	__system(tmp);
    sprintf(tmp,"%s/.extfs",looppos);
    FILE*f = fopen(tmp,"w+");fclose(f);
    truncate(tmp,loopsize);
    sprintf(tmp,"losetup /dev/block/loop0 %s/.extfs",looppos);
    __system(tmp);
    __systemscript("/sbin/mkfs.ext2 -b 4096 -m 0 -F /dev/block/loop0");
    __system("losetup -d /dev/block/loop0");
    sprintf(tmp,"umount %s",blockname);
    __system(tmp);
  }
  return 0;
}

void ui_print_file(char* name) {
  FILE *f = fopen(name,"r");
  char tmp[64];
  if (f) {
    while (fgets(tmp,63,f)) {
      ui_print(tmp);
    }
    fclose(f);
  }
}

int do_lagfix(int do_fr) {
  ui_print("old config was:\n");
  ui_print_file("/system/etc/lagfix.conf.old");
  ui_print("new config is:\n");
  ui_print_file("/system/etc/lagfix.conf");
  ui_print("checking mounts available\n");
  if (ensure_root_path_mounted("DATA:")!=0) return -1;
  if (ensure_root_path_mounted("DATADATA:")!=0) return -1;
  if (ensure_root_path_mounted("CACHE:")!=0) return -1;
  if (ensure_root_path_mounted("SDCARD:")!=0) return -1;

  char tmp[PATH_MAX];
  nandroid_generate_timestamp_path(tmp);
  int backup_ok = 1; //detect if backup was ok
  if (do_fr!=2) {
    ui_print("Creating a nandroid backup at %s\n",tmp);
    //if (nandroid_backup_flags(tmp,DONT_BACKUP_SYSTEM)!=0) return -1;
	if (nandroid_backup_flags(tmp,DONT_BACKUP_SYSTEM)!=0) backup_ok = 0;
  } else {
    ui_print("Not creating a backup\n");
  }
  if (backup_ok==0) { //backup failed
	ui_print("Backup failed!\nYou may need more free space on int SD\nAborting lagfix\n");
	return -1; //abort
  }

  ui_print("Backup completed, recreating file systems\n");

  ui_print("Unmounting\n");
  unmount_all_lagfixed();

  ui_print("Switching to new config\n");
  __system("cp /system/etc/lagfix.conf /system/etc/lagfix.conf.old");

  ui_print("Creating /data\n");
  create_lagfix_partition(0);
  ui_print("Creating /dbdata\n");
  create_lagfix_partition(1);
  ui_print("Creating /cache\n");
  create_lagfix_partition(2);

  ui_print("Mounting to test\n");
  dont_use_bind_options = 1;
  if (ensure_root_path_mounted("DATA:")!=0) return -1;
  if (ensure_root_path_mounted("DATADATA:")!=0) return -1;
  if (ensure_root_path_mounted("CACHE:")!=0) return -1;
  dont_use_bind_options = 0;
  __system("mount"); // for debug purposes
  if (get_bind_options()) {
    ui_print("Creating bind directories\n");
    __system("mkdir -p /dbdata/.data/data");
    __system("mkdir -p /data/data");
  }

  ui_print("Unmounting again\n");
  unmount_all_lagfixed();

  if (do_fr) {
    ui_print("Factory reset was requested, not restoring backed up data\n");
  } else {
    ui_print("Restoring data\n");
    nandroid_restore(tmp,0,0,1,1,0);
    // restore might have brought some .data into dbdata, clear them
    if (!get_bind_options()) {
      if (ensure_root_path_mounted("DATADATA:")!=0) return -1;
      __system("rm -rf /dbdata/.data");
    }
  }

  __system("mount");
  ui_print("Unmounting again to be sure\n");
  sync();
  sleep(5);
  unmount_all_lagfixed();
  sync();
  return 0;
}

int do_lagfixsys(int do_fr) {
  ui_print("Going to convert /system:\n");
  
		__system("mkdir /mnt");
		__system("mkdir /mnt/sdcard");
		__system("ln -s /mnt/sdcard /sdcard");
		__system("mount /dev/block/mmcblk0p1 /mnt/sdcard");
  
		ui_print("\nStep 1: Backup partitions...\nThis may take a while...\n");
		__system("tar cvf /sdcard/system-backup.tar /system");		
		
		char tmp[PATH_MAX];
		
		ui_print("Unmounting /system\n");
		sync();
		__systemscript("/sbin/pre-init-cpsystemtoram.sh");
		
		ui_print("\nStep 2: Format partitions...\n");
		
		ui_print("Formatting SYSTEM:...\n");
		
		if (do_fr == 0) {
		sprintf(tmp,"/system/bin/mkfs.ext4 -O ^has_journal -b 4096 -m 0 -N 2000 -F /dev/block/stl9");
		} else if (do_fr == 1) {
		sprintf(tmp,"/system/bin/fat.format -S 4096 -s 1 -F 32 /dev/block/stl9");
		}
		__systemscript(tmp);

		ui_print("Mount /system again\n");
		if (do_fr == 0) {
		sprintf(tmp,"mount -t ext4 /dev/block/stl9 /system");
		} else if (do_fr == 1) {
		sprintf(tmp,"mount -t rfs -o nosuid,nodev,check=no /dev/block/stl9 /system");
		}  
		__system(tmp);
		
		ui_print("\nStep 3: Restore partitions...\nThis may take a while...\n");
		
		__system("tar xvf /sdcard/system-backup.tar");
		//nandroid_restore(backup_path,0,1,0,0,0); //restore SYSTEM

		ui_print("Deleting backup\n");
		__system("rm -f /sdcard/system-backup.tar");
		
		//sprintf(tmp,"rm -rf %s",backup_path);
		//__system(tmp);
		
		sync();
		ui_print("\nConversion done!\n");

  //better reboot now
  return 0;
}

int lagfixer_main(int argc, char** argv) {
  ui_init();
  ui_print(EXPAND(RECOVERY_VERSION)"\nFilesystem Converter\n");
  create_fstab();
  ui_set_show_text(1);

  int res;
  int opts = 0;
  int syslf = 0;
  if ((argc>=2)&&(strcmp(argv[1],"fr")==0)) {
    opts=2;
  } else if ((argc>=2)&&(strcmp(argv[1],"b")==0)) {
    opts=1;
  } else if ((argc>=2)&&(strcmp(argv[1],"sysext4")==0)) {
    syslf=1;
	opts=0; //ext4
  } else if ((argc>=2)&&(strcmp(argv[1],"sysrfs")==0)) {
    syslf=1;
	opts=1; //rfs
  }
  if (syslf == 0) {
	res = do_lagfix(opts);
  } else {
    res = do_lagfixsys(opts);
  }
  if (res) {
    ui_print("Something went wrong while doing the lagfix, sorry.\n");
  } else {
    ui_print("Done. Your device will reboot soon or enter recovery mode to debug.\n");
  }
  sleep(5);

  gr_exit();
  ev_exit();
  return 0;
}

//**********************************************************************
void show_partition_sizes(void) {
    int i;
    char buf[128];
    ensure_root_path_mounted("DATA:");
    __systemscript("/sbin/partitions.sh");
    FILE* f = fopen("/data/midnight/partitions.txt","r");
    ui_print("\n\nPARTITIONS/FREE SPACE:\n");
    if (f) {
      while (fgets(buf,127,f)) {                    
        ui_print(buf);
      }
      fclose(f);
    }
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
    for (;;)
    {
        for (i=0;i<numtweaks;i++) tweaks[i]=0;          // set all tweaks disabled
        FILE* f = fopen(conffile,"r");                  // open configfile
        ui_print("\n\nEnabled options:\n");
        if (f) {
          while (fgets(buf,127,f)) {                    // read all enabled options
            ui_print(buf);
            if (onlyone!=1){                            // only if multiple options possible:
                for (i=0; i<numtweaks; i++) {           // enable options in tweaks array
                    if (memcmp(buf,options[i],strlen(options[i]))==0) tweaks[i]=1;
                }
            }
          }
          fclose(f);
        }                                               // done
        
        // start menu
        int chosen_item = get_menu_selection(headers, list, 0);
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
        "[0] 8,16,24,55,60,80Mb (MIDNIGHT)",
        "[1] 8,12,16,24,28,32Mb (NEXUS S)",
		"[2] 8,16,24,28,36,42Mb",
		"[3] 8,16,24,36,48,56Mb",
		"[4] 8,16,24,42,56,68Mb",
        "[5] 8,16,24,56,68,88Mb",
        "[6] 8,16,24,62,74,96Mb",
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
        "Toggle IPv6 privacy (security)",
        "Toggle init.d support",
        NULL};
    int num=5;
    const char* cnfv[]={"CIFS","ANDROIDLOGGER","TUN", "IPV6PRIVACY", "INIT_D" };
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
    int num=2;
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
        "[0] 512kB [default]",
        "[1] 1024kB",
        "[2] 2048kB",
        "[3] 3064kB",
        "[4] 4096kB",
        NULL};
    int num=5;
    const char* cnfv[]={"READAHEAD_512","READAHEAD_1024","READAHEAD_2048","READAHEAD_3064","READAHEAD_4096"};
    const char* cnff="/system/etc/midnight_rh.conf";
    custom_menu(h,m,num,cnfv,cnff,1);
}
void create_backup_dirs() {
  ensure_root_path_mounted("DATA:");
  __system("mkdir -p /data/midnight");
  __system("mkdir -p /data/midnight/backups");
  __system("mkdir -p /data/midnight/backups/initd");
  __system("mkdir -p /data/midnight/backups/sounds");
  __system("mkdir -p /data/midnight/backups/voltage-control");
  __system("mkdir -p /data/midnight/backups/localprop");
  __system("mkdir -p /data/midnight/backups/midnight-conf");
  __system("mkdir -p /data/midnight/backups/bootanimation");
  __system("mkdir -p /data/midnight/backups/bootanimation/local");
  __system("mkdir -p /data/midnight/backups/bootanimation/system");
}


void cleanup_submenu(int m) {
    create_backup_dirs();
    if (m==0){
        static char* headers[] = {  "BACKUP/DELETE/RESTORE ALL CUSTOM BOOTANIMATIONS",
                                    "Backup your files (overwriting previously backuped",
                                    "files) to /data/midnight, delete or restore them.",
                                    NULL};
        static char* list[] = { "Backup custom bootanimations",
                                "Delete custom bootanimations",
                                "Restore custom bootanimations",
                                NULL
        };
        for (;;)
        {
            int chosen_item = get_menu_selection(headers, list, 0);
            if (chosen_item == GO_BACK)
                break;        
            switch (chosen_item)
            {
                  case 0:
                  {
                    if (confirm_selection("Confirm backup custom bootanimations","Yes - backup custom bootanimations")) {  
                      ensure_root_path_mounted("SYSTEM:");
                      ensure_root_path_mounted("DATA:");
                      ui_print("Cleaning backup directory...");
                      __system("rm -r /data/midnight/backups/bootanimation/local/*");                
                      __system("rm -r /data/midnight/backups/bootanimation/system/*");                
                      ui_print("Backup to /data/midnight/backups/bootanimation...\n");
                      __system("cp /data/local/bootanimation.zip /data/midnight/backups/bootanimation/local");                
                      __system("cp /data/local/sanim.zip /data/midnight/backups/bootanimation/local");                
                      __system("cp /data/local/bootanimation.bin /data/midnight/backups/bootanimation/local");                
                      __system("cp /system/media/bootanimation.zip /data/midnight/backups/bootanimation/system");                
                      __system("cp /system/media/sanim.zip /data/midnight/backups/bootanimation/system");                
                      ui_print("Done.\n");
                      ui_print("Backup files:\n");
                      show_dir_contents("/data/midnight/backups/bootanimation/local");
                      show_dir_contents("/data/midnight/backups/bootanimation/system");
                    }
                    break;
                  }
                case 1:
                  {
                    if (confirm_selection("Confirm deleting custom bootanimations","Yes - delete custom bootanimations")) {  
                      ensure_root_path_mounted("SYSTEM:");
                      ensure_root_path_mounted("DATA:");
                      ui_print("Deleting bootanimations...\n");
                      __system("rm -f /data/local/bootanimation.zip");                
                      __system("rm -f /data/local/sanim.zip");                
                      __system("rm -f /data/local/bootanimation.bin");                
                      __system("rm -f /system/media/bootanimation.zip");                
                      __system("rm -f /system/media/sanim.zip");                
                      ui_print("Done.\n");
                    }
                    break;
                  }
                  case 2:
                  {
                    if (confirm_selection("Confirm restoring custom bootanimations","Yes - restore bootanimations")) {  
                      ensure_root_path_mounted("SYSTEM:");
                      ensure_root_path_mounted("DATA:");
                      ui_print("Restoring files...\n");
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
        }
    }
    if (m==1){
        static char* headers[] = {  "BACKUP/DELETE/RESTORE /SYSTEM/ETC/INIT.D CONTENT",
                                    "Backup your files (overwriting previously backuped",
                                    "files) to /data/midnight, delete or restore them.",
                                    NULL};
        static char* list[] = { "Backup /system/etc/init.d",
                                "Delete /system/etc/init.d",
                                "Restore /system/etc/init.d",
                                NULL
        };
        for (;;)
        {
            int chosen_item = get_menu_selection(headers, list, 0);
            if (chosen_item == GO_BACK)
                break;        
            switch (chosen_item)
            {
                  case 0:
                  {
                    if (confirm_selection("Confirm backup /system/etc/init.d","Yes - backup init.d")) {  
                      ensure_root_path_mounted("SYSTEM:");
                      ensure_root_path_mounted("DATA:");
                      ui_print("Cleaning backup directory...");
                      __system("rm -r /data/midnight/backups/initd/*");                
                      ui_print("Backup to /data/midnight/initd...\n");
                      __system("cp /system/etc/init.d/* /data/midnight/backups/initd");                
                      ui_print("Done.\n");
                      ui_print("Backup files:\n");
                      show_dir_contents("/data/midnight/backups/initd");
                    }
                    break;
                  }
                case 1:
                  {
                    if (confirm_selection("Confirm cleaning /system/etc/init.d","Yes - clean init.d")) {  
                      ensure_root_path_mounted("SYSTEM:");
                      ensure_root_path_mounted("DATA:");
                      ui_print("Cleaning /system/etc/init.d...\n");
                      __system("rm -rf /system/etc/init.d/*");                
                      ui_print("Done.\n");
                    }
                    break;
                  }
                  case 2:
                  {
                    if (confirm_selection("Confirm restoring /system/etc/init.d","Yes - restore init.d content")) {  
                      ensure_root_path_mounted("SYSTEM:");
                      ensure_root_path_mounted("DATA:");
                      ui_print("Restoring files...\n");
                      __system("cp /data/midnight/backups/initd/* /system/etc/init.d");                
                      ui_print("Done.\n");
                    }
                    break;
                  }
            }
        }
    }
    if (m==2){
        static char* headers[] = {  "BACKUP/REMOVE/RESTORE BOOT/SHUTDOWN SOUNDS",
                                    "Backup your files (overwriting previously backuped",
                                    "files) to /data/midnight, delete or restore them.",
                                    NULL};
        static char* list[] = { "Backup boot/shutdown sounds",
                                "Remove boot/shutdown sounds",
                                "Restore boot/shutdown sounds",
                                NULL
        };
        for (;;)
        {
            int chosen_item = get_menu_selection(headers, list, 0);
            if (chosen_item == GO_BACK)
                break;        
            switch (chosen_item)
            {
                  case 0:
                  {
                    if (confirm_selection("Confirm backup start/shutdown sounds","Yes - backup start/shutdown sounds")) {  
                      ensure_root_path_mounted("SYSTEM:");
                      ensure_root_path_mounted("DATA:");
                      ui_print("Cleaning backup directory...");
                      __system("rm -r /data/midnight/backups/sounds/*");                
                      ui_print("Backup to /data/midnight/backups/sounds...\n");
                      __system("cp /system/etc/PowerOn.snd /data/midnight/backups/sounds");                
                      __system("cp /system/etc/PowerOn.wav /data/midnight/backups/sounds");                
                      __system("cp /system/media/audio/ui/shutdown.ogg /data/midnight/backups/sounds");                
                      ui_print("Done.\n");
                      ui_print("Backup files:\n");
                      show_dir_contents("/data/midnight/backups/sounds");

                    }
                    break;
                  }

                  case 1:
                  {
                    if (confirm_selection("Confirm deleting start/shutdown sounds","Yes - delete start/shutdown sounds")) {  
                      ensure_root_path_mounted("SYSTEM:");
                      ensure_root_path_mounted("DATA:");
                      ui_print("Deleting startup/shutdown sounds...\n");
                      __system("rm -f /system/etc/PowerOn.snd");                
                      __system("rm -f /system/etc/PowerOn.wav");                
                      __system("rm -f /system/media/audio/ui/shutdown.ogg");                
                      ui_print("Done.\n");
                    }
                    break;
                  }
                  case 2:
                  {
                    if (confirm_selection("Confirm restoring boot/shutdown sounds","Yes - restore sounds")) {  
                      ensure_root_path_mounted("SYSTEM:");
                      ensure_root_path_mounted("DATA:");
                      ui_print("Restoring files...\n");
                      __system("cp /data/midnight/backups/sounds/PowerOn.snd /system/etc");                
                      __system("cp /data/midnight/backups/sounds/PowerOn.wav /system/etc");                
                      __system("cp /data/midnight/backups/sounds/shutdown.ogg /system/media/audio/ui");                
                      ui_print("Done.\n");
                    }
                    break;
                  }
            }
        }
    }

    if (m==3){
        static char* headers[] = {  "BACKUP/REMOVE/RESTORE /data/local.prop",
                                    "Backup your files (overwriting previously backuped",
                                    "files) to /data/midnight, delete or restore them.",
                                    NULL};
        static char* list[] = { "Backup /data/local.prop",
                                "Remove /data/local.prop",
                                "Restore /data/local.prop",
                                NULL
        };
        for (;;)
        {
            int chosen_item = get_menu_selection(headers, list, 0);
            if (chosen_item == GO_BACK)
                break;        
            switch (chosen_item)
            {
                  case 0:
                  {
                    if (confirm_selection("Confirm backup /data/local.prop","Yes - backup /data/local.prop")) {  
                      ensure_root_path_mounted("DATA:");
                      ui_print("Cleaning backup directory...");
                      __system("rm -r /data/midnight/backups/localprop/*");                                      
                      ui_print("Backup files to /data/midnight/backups/localprop...\n");
                      __system("cp /data/local.prop /data/midnight/backups/localprop");
                      ui_print("Done.\n");
                      ui_print("Backup files:\n");
                      show_dir_contents("/data/midnight/backups/localprop");
                    }
                    break;
                  }
                  case 1:
                  {
                    if (confirm_selection("Confirm deleting /data/local.prop","Yes - delete /data/local.prop")) {  
                      ensure_root_path_mounted("DATA:");
                      ui_print("Deleting /data/local.prop...\n");
                      __system("rm -rf /data/local.prop");                
                      ui_print("Done.\n");
                    }
                    break;
                  }
                  case 2:
                  {
                    if (confirm_selection("Confirm restoring /data/local.prop","Yes - restore /data/local.prop")) {  
                      ensure_root_path_mounted("SYSTEM:");
                      ensure_root_path_mounted("DATA:");
                      ui_print("Restoring files...\n");
                      __system("cp /data/midnight/backups/localprop/local.prop /data");
                      ui_print("Done.\n");
                    }
                    break;
                  }
            }
        }
    }
    
    if (m==4){
        static char* headers[] = {  "BACKUP/REMOVE/RESTORE INIT.D/S_VOLT_SCHEDULER",
                                    "Backup your files (overwriting previously backuped",
                                    "files) to /data/midnight, delete or restore them.",
                                    NULL};
        static char* list[] = { "Backup init.d/S_volt_scheduler",
                                "Remove init.d/S_volt_scheduler",
                                "Restore init.d/S_volt_scheduler",
                                NULL
        };
        for (;;)
        {
            int chosen_item = get_menu_selection(headers, list, 0);
            if (chosen_item == GO_BACK)
                break;        
            switch (chosen_item)
            {
                  case 0:
                  {
                    if (confirm_selection("Confirm backup init.d/S_volt_scheduler","Yes - backup init.d/S_volt_scheduler")) {  
                      ensure_root_path_mounted("SYSTEM:");
                      ui_print("Cleaning backup directory...");
                      __system("rm -r /data/midnight/backups/voltage-control/*");                                      
                      ui_print("Backup files to /data/midnight/backups/voltage-control...\n");
                      __system("cp /system/etc/init.d/S_volt_scheduler /data/midnight/backups/voltage-control");
                      ui_print("Done.\n");
                      ui_print("Backup files:\n");
                      show_dir_contents("/data/midnight/backups/voltage-control");
                    }
                    break;
                  }
                  case 1:
                  {
                    if (confirm_selection("Confirm deleting init.d/S_volt_scheduler","Yes - delete init.d/S_volt_scheduler")) {  
                      ensure_root_path_mounted("SYSTEM:");
                      ui_print("Deleting /system/etc/init.d/S_volt_scheduler...\n");
                      __system("rm -rf /system/etc/init.d/S_volt_scheduler");                
                      ui_print("Done.\n");
                      ui_print("Found backup files:\n");
                      show_dir_contents("/data/midnight/backups/voltage-control");
                    }
                    break;
                  }
                  case 2:
                  {
                    if (confirm_selection("Confirm restoring init.d/S_volt_scheduler","Yes - restore init.d/S_volt_scheduler")) {  
                      ensure_root_path_mounted("SYSTEM:");
                      ensure_root_path_mounted("DATA:");
                      ui_print("Restoring files...\n");
                      __system("cp /data/midnight/backups/voltage-control/S_volt_scheduler /system/etc/init.d");
                      ui_print("Done.\n");
                    }
                    break;
                  }
            }
        }
    }
    
    if (m==5){
        static char* headers[] = {  "BACKUP/DELETE/RESTORE MIDNIGHT CONFIG FILES",
                                    "Backup your files (overwriting previously backuped",
                                    "files) to /data/midnight, delete or restore them.",
                                    NULL};
        static char* list[] = { "Backup Midnight config files",
                                "Delete Midnight config files",
                                "Restore Midnight config files",
                                NULL
        };
        for (;;)
        {
            int chosen_item = get_menu_selection(headers, list, 0);
            if (chosen_item == GO_BACK)
                break;        
            switch (chosen_item)
            {
                  case 0:
                  {
                    if (confirm_selection("Confirm backup Midnight conf files","Yes - backup Midnight config files")) {  
                      ensure_root_path_mounted("SYSTEM:");
                      ui_print("Cleaning backup directory...");
                      __system("rm -r /data/midnight/backups/midnight-conf/*");                                      
                      ui_print("Backup to /data/midnight/backups/midnight-conf...\n");
                      __system("cp /system/etc/midnight_*.conf /data/midnight/backups/midnight-conf");
                      ui_print("Done.\n");
                      ui_print("Backup files:\n");
                      show_dir_contents("/data/midnight/backups/midnight-conf");
                    }
                    break;
                  }
                  case 1:
                  {
                    if (confirm_selection("Confirm deleting Midnight conf files","Yes - delete ALL Midnight config files")) {  
                      ensure_root_path_mounted("SYSTEM:");
                      ui_print("Deleting /system/etc/midnight_*.conf\n");
                      __system("rm -rf /system/etc/midnight_*.conf");                
                      ui_print("Done.\n");
                      ui_print("Found backup files:\n");
                      show_dir_contents("/data/midnight/backups/midnight-conf");
                    }
                    break;
                  }
                  case 2:
                  {
                    if (confirm_selection("Confirm restoring Midnight config files","Yes - restore Midnight config files")) {  
                      ensure_root_path_mounted("SYSTEM:");
                      ensure_root_path_mounted("DATA:");
                      ui_print("Restoring files...\n");
                      __system("cp /data/midnight/backups/midnight-conf/midnight_*.conf /system/etc");
                      ui_print("Done.\n");
                    }
                    break;
                  }
            }
        }
    }
}

void cleanup_menu() {
    create_backup_dirs();
    static char* headers[] = {  "MISC. CLEANUP OPTIONS",
                                "Backup, delete and restore files...",
                                "",
                                NULL
    };
    static char* list[] = { "Custom bootanimations (@system,@data)",
                            "Initscripts in init.d (@system)",
                            "Startup/shutdown sounds (@system)",
                            "local.prop (@data)",
                            "VoltageControl initscript (@system)",
                            "Midnight config files (@system)",
                            "Superuser, su, busybox (@system,@data)",
                            NULL
    };
    for (;;)
    {
        int chosen_item = get_menu_selection(headers, list, 0);
        if (chosen_item == GO_BACK)
            break;        
        switch (chosen_item)
        {
              case 0:
              {
                ui_print("Checking if file(s) installed:\n");
                show_dir_contents("/data/local/bootanimation.*");                  
                show_dir_contents("/data/local/sanim.zip");                  
                show_dir_contents("/system/media/bootanimation.*");                  
                show_dir_contents("/system/media/sanim.zip");                  
                cleanup_submenu(0);    
                break;
              }
              case 1:
              {
                ui_print("Checking if file(s) installed:\n");
                show_dir_contents("/system/etc/init.d");                  
                cleanup_submenu(1);
                break;
              }
              case 2:
              {
                ui_print("Checking if file(s) installed:\n");
                show_dir_contents("/system/etc/PowerOn.*");                  
                show_dir_contents("/system/media/audio/ui/shutdown.ogg");                  
                cleanup_submenu(2);
                break;
              } 
              case 3:
              {
                ui_print("Checking if file(s) installed:\n");
                show_dir_contents("'/data/local.prop'");                  
                cleanup_submenu(3);
                break;
              }
              case 4:
              {
                ui_print("Checking if file(s) installed:\n");
                show_dir_contents("'/system/etc/init.d/S_volt_scheduler'");                  
                cleanup_submenu(4);
                break;
              }
              case 5:
              {
                ui_print("Checking if file(s) installed:\n");
                show_dir_contents("'/system/etc/midnight_*.conf'");                  
                cleanup_submenu(5);
                break;
              }
              case 6:
              {
                if (confirm_selection("Confirm deleting su/superuser/busybox","Yes - delete su/superuser/busybox")) {  
                    remove_root_from_device();
                }
                break;
              }

        }
    }

}


void touch_menu() {
    const char* h[]={   
        "SELECT TOUCHSCREEN SENSITIVITY",
        "",
        "",
        NULL};
    const char* m[]={   
        "Stock values [default]",
        "Sensitivity +",
        "Sensitivity ++",
        "Sensitivity +++",
        NULL};
    int num=4;
    const char* cnfv[]={"TOUCH_DEFAULT","TOUCH_PLUS1","TOUCH_PLUS2","TOUCH_PLUS3"};
    const char* cnff="/system/etc/midnight_touch.conf";
    custom_menu(h,m,num,cnfv,cnff,1);
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
        "VR",
        "CFQ",
        "DEADLINE",
        NULL};
    int num=4;
    const char* cnfv[]={"IO_SCHED_NOOP","IO_SCHED_VR","IO_SCHED_CFQ","IO_SCHED_DEADLINE"};
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
        "Values:1300/1200/1000/800/400/200/100Mhz.",
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

void cpu_gov_settings_conservative_up_menu() {
    const char* h[]={   
        "SELECT CONSERVATIVE GOVERNOR UP_THRESHOLD",
        "Higher values will make the CPU scale up",
        "later and save more battery [60 DEFAULT].",
        NULL};
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
        "don't override DEFAULT value",        
        NULL};
    const char* cnff="/system/etc/midnight_cpu_gov_cons_up.conf";
    custom_menu(h,m,num,cnfv,cnff,0);
}

void cpu_gov_settings_conservative_down_menu() {
    const char* h[]={   
        "SELECT CONSERVATIVE GOVERNOR DOWN_THRESHOLD",
        "Higher values will make the CPU scale down",
        "later and save more battery [45 DEFAULT].",
        NULL};
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
        "don't override DEFAULT value",        
        NULL};
    const char* cnff="/system/etc/midnight_cpu_gov_cons_down.conf";
    custom_menu(h,m,num,cnfv,cnff,0);
}

void cpu_gov_settings_ondemand_up_menu() {
    const char* h[]={   
        "SELECT ONDEMAND GOVERNOR UP_THESHOLD",
        "Higher values will make the CPU switch",
        "to max. frequency later and save more",
        "battery [85 DEFAULT].",
        NULL};
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
        "don't override DEFAULT value",        
        NULL};
    const char* cnff="/system/etc/midnight_cpu_gov_ond_up.conf";
    custom_menu(h,m,num,cnfv,cnff,0);
}

void cpu_gov_settings_menu() {
    static char* headers[] = {  "CPU GOVERNOR OPTIONS",
                                "Configure CPU governor tunables to",
                                "customize up/downscaling behaviour...",
                                NULL
    };
    static char* list[] = { "Select conservative UP_THRESHOLD...",
                            "Select conservative DOWN_THRESHOLD...",
                            "Select ondemand UP_THRESHOLD...",
                            NULL
    };
    for (;;)
    {
        int chosen_item = get_menu_selection(headers, list, 0);
        if (chosen_item == GO_BACK)
            break;
        switch (chosen_item)
        {
              case 0:
              {
                cpu_gov_settings_conservative_up_menu();
                break;
              }
              case 1:
              {
                cpu_gov_settings_conservative_down_menu();
                break;
              }
              case 2:
              {
                cpu_gov_settings_ondemand_up_menu();
                break;
              }
       }
    }
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
                            "Adjust CPU governor thresholds...",
                            "Select CPU undervolting preset...",
                            "Setup CPU undervolting 1300Mhz...",
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
        int chosen_item = get_menu_selection(headers, list, 0);
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
                cpu_gov_settings_menu();
                break;
              }
              case 3:
              {
                cpu_uv_menu();
                break;
              }
              case 4:
              {
                cpu_uv_freq_menu(1300);
                break;
              }
              case 5:
              {
                cpu_uv_freq_menu(1200);
                break;
              }
              case 6:
              {
                cpu_uv_freq_menu(1000);
                break;
              }
              case 7:
              {
                cpu_uv_freq_menu(800);
                break;
              }
              case 8:
              {
                cpu_uv_freq_menu(400);
                break;
              }
              case 9:
              {
                cpu_uv_freq_menu(200);
                break;
              }
               case 10:
              {
                cpu_uv_freq_menu(100);
                break;
              }
               case 11:
              {
                if (confirm_selection("Confirm deleting undervolting settings","Yes - delete ALL undervolting settings")) {  
                  ensure_root_path_mounted("SYSTEM:");
                  ui_print("Deleting stored preset...\n");
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
        int chosen_item = get_menu_selection(headers, list, 0);
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
                  ensure_root_path_mounted("SYSTEM:");
                  ui_print("Deleting stored preset...\n");
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

void filesystem_menu() {
    static char* headers[] = {  "FILESYSTEM OPTIONS",
                                "Convert or defragment filesystems and",
                                "select IO scheduler...",
                                NULL
    };
    static char* list[] = { "Convert /DATA, /DBDATA, /CACHE -> RFS",
                            "Convert /DATA, /DBDATA, /CACHE -> EXT4",
                            "Convert /SYSTEM -> RFS/EXT4",
                            NULL
    };
    for (;;)
    {
        int chosen_item = get_menu_selection(headers, list, 0);
        if (chosen_item == GO_BACK)
            break;
        switch (chosen_item)
        {
              case 0:
              {
				convertfs_rfs_menu();
                break;
              }
              case 1:
              {
			    convertfs_ext4_menu();
                break;
              }
			  case 2:
              {
			    lagfix_system_menu();
                break;
              }
        }
    }
}

//**********************************************************************
void convertfs_ext4_menu() {
    static char* headers[] = {  "CONVERT FILESYSTEM TO EXT4?",
								"If fs is already EXT4, it will be unfragmented.",
								"",
								"/data /dbdata and /cache will be converted",
								"by performing a backup, format and restore.",
								"",
								"Make sure to have enough free space on int SD",								
                                "",
                                NULL
    };

    static char* list[] = { "YES",
							"NO",
                            NULL
    };

    for (;;)
    {
		int chosen_item = get_menu_selection(headers, list, 0);
		if (chosen_item == GO_BACK) {
			break;
		} else {
			switch (chosen_item)
			{
				case 0:
					ui_print("\nConvert to EXT4 requested\nThis will take some time\n");

					ui_print("\nStep 1: Backup partitions...\n");
					
					char tmp[PATH_MAX];
					nandroid_generate_timestamp_path(tmp);
					int backup_ok = 1; //detect if backup was ok
					ui_print("Creating a nandroid backup at %s\n",tmp);
					if (nandroid_backup_flags(tmp,DONT_BACKUP_SYSTEM)!=0) backup_ok = 0;
					//if (nandroid_backup_flags(tmp,0)!=0) backup_ok = 0; //backup SYSTEM also
					if (backup_ok==0) { //backup failed
						ui_print("Backup failed!\nYou may need more free space on int SD\nAborting\n");
						return; //abort
					}
					
					ui_print("\nStep 2: Format partitions...\n");
					
					ui_print("Formatting DATA:...\n");
					__system("umount -f /dev/block/mmcblk0p2");
					__systemscript("/sbin/mkfs.ext4 -L DATA -b 4096 -N 50000 -m 0 -F /dev/block/mmcblk0p2");
					__system("mount -t ext4 -o noatime,data=ordered,nodelalloc /dev/block/mmcblk0p2 /data");
  
					ui_print("Formatting DBDATA:...\n");
					__system("umount -f /dev/block/stl10");
					__systemscript("/sbin/mkfs.ext4 -L DBDATA -b 4096 -N 20000 -m 0 -F /dev/block/stl10");
					__system("mount -t ext4 -o noatime,data=ordered,nodelalloc /dev/block/stl10 /dbdata");
  
					ui_print("Formatting CACHE:...\n");
					__system("umount -f /dev/block/stl11");
					__systemscript("/sbin/mkfs.ext4 -O ^has_journal -L CACHE -N 2000 -b 4096 -m 0 -F /dev/block/stl11");
					__system("mount -t ext4 -o noatime,data=ordered,nodelalloc /dev/block/stl11 /cache");
										
					/*ui_print("Formatting SYSTEM:...\n");
					__system("umount -f /dev/block/stl9");
					__systemscript("/sbin/mkfs.ext4 -O ^has_journal -L CACHE -N 2000 -b 4096 -m 0 -F /dev/block/stl9");
					__system("mount -t ext4 -o noatime,data=ordered,nodelalloc /dev/block/stl9 /system");
					*/
					ui_print("\nStep 3: Restore partitions...\n");
					nandroid_restore(tmp,0,0,1,1,0); //restore DATA + CACHE
					//nandroid_restore(tmp,0,1,1,1,0); //restore SYSTEM + DATA + CACHE
					
					ui_print("\nConvert to EXT4 done!\n");
					break;
				case 1:
					ui_print("\nFS conversion not requested\n");
				break;
			}
			break; //go back
		}
	}
}

void convertfs_rfs_menu() {
    static char* headers[] = {  "CONVERT FILESYSTEM TO RFS?",
								"If fs is already RFS, it will be unfragmented.",
								"",
								"/data /dbdata and /cache will be converted",
								"by performing a backup, format and restore.",
								"",
								"Make sure to have enough free space on int SD",								
                                "",
                                NULL
    };

    static char* list[] = { "YES",
							"NO",
                            NULL
    };

    for (;;)
    {
		int chosen_item = get_menu_selection(headers, list, 0);
		if (chosen_item == GO_BACK) {
			break;
		} else {
			switch (chosen_item)
			{
				case 0:
					ui_print("\nConvert to RFS requested\nThis will take some time\n");

					ui_print("\nStep 1: Backup partitions...\n");
					
					char tmp[PATH_MAX];
					nandroid_generate_timestamp_path(tmp);
					int backup_ok = 1; //detect if backup was ok
					ui_print("Creating a nandroid backup at %s\n",tmp);
					if (nandroid_backup_flags(tmp,DONT_BACKUP_SYSTEM)!=0) backup_ok = 0;
					if (backup_ok==0) { //backup failed
						ui_print("Backup failed!\nYou may need more free space on int SD\nAborting\n");
						return; //abort
					}
					
					ui_print("\nStep 2: Format partitions...\n");
					
					ui_print("Formatting DATA:...\n");
					__system("umount -f /dev/block/mmcblk0p2");
					__systemscript("/sbin/fat.format -S 4096 -s 4 -F 32 /dev/block/mmcblk0p2");
					__system("mount -t rfs -o nosuid,nodev,check=no /dev/block/mmcblk0p2 /data");
  
					ui_print("Formatting DBDATA:...\n");
					__system("umount -f /dev/block/stl10");
					__systemscript("/sbin/fat.format -S 4096 -s 1 -F 16 /dev/block/stl10");
					__system("mount -t rfs -o nosuid,nodev,check=no /dev/block/stl10 /dbdata");
  
					ui_print("Formatting CACHE:...\n");
					__system("umount -f /dev/block/stl11");
					__systemscript("/sbin/fat.format -S 4096 -s 1 -F 16 /dev/block/stl11");
					__system("mount -t rfs -o nosuid,nodev,check=no /dev/block/stl11 /cache");
					
					ui_print("\nStep 3: Restore partitions...\n");
					nandroid_restore(tmp,0,0,1,1,0);
					
					ui_print("\nConvert to RFS done!\n");
					break;
				case 1:
					ui_print("\nFS conversion not requested\n");
				break;
			}
			break; //go back
		}
	}
}

void apply_root_menu() {
    static char* headers[] = {  "INSTALL ROOT",
                                "Be sure to have enough free space",
                                "space (~1Mb) on /system partition, use",
                                "cleanup options to free space if needed...",
                                NULL
    };

    static char* list[] = { "Install ROOT (su, Superuser, busybox)",
                            "Install ROOT and remove some toolbox cmds",
                            "Install ROOT and remove most toolbox cmds",
                            NULL
    };

    for (;;)
    {
        int chosen_item = get_menu_selection(headers, list, 0);
        if (chosen_item == GO_BACK)
            break;
        switch (chosen_item)
        {
            case 0:
              if (confirm_selection("Confirm ROOT","Yes - apply ROOT to device")) {
                apply_root_to_device(0);
              }
              break;
            case 1:
              if (confirm_selection("Confirm ROOT","Yes - apply ROOT to device")) {
                apply_root_to_device(1);
              }
              break;
            case 2:
              if (confirm_selection("Confirm ROOT","Yes - apply ROOT to device")) {
                apply_root_to_device(2);
              }
              break;
        }
    }

}


void show_advanced_lfs_menu() {
    static char* headers[] = {  "ADVANCED OPTIONS MENU",
                                "",
                                "",
                                NULL
    };

    static char* list[] = { "Reboot into Recovery",
                            "Reboot into Download",
                            "Shutdown",
                            "Switch to Recovery3e",
                            "Install ROOT and Superuser",
                            "Convert filesystems to RFS/EXT4",
                            "Configure misc./modules",
                            "Configure video driver options",
                            "Configure sdcard READ_AHEAD value",
                            "Configure IO scheduler",
                            "Configure lowmemorykiller values",
                            "Configure CPU",
                            "Configure touchscreen sensitivity",
                            "Cleanup options",
                            NULL
    };

    static char* headersno[] = { "Universal Lagfix Kernel Not Found",
                                 "These options only work if ULK is used",
                                 "",
                                 NULL };
    static char* listno[] = { "Okay", NULL };

    struct stat ss;
    // we check for the pre-init.log, which this kernel creates each run
    /*if (stat("/res/pre-init.log",&ss)!=0) {
      get_menu_selection(headersno,listno,0);
    } else {*/
      for (;;)
      {
          int chosen_item = get_menu_selection(headers, list, 0);
          if (chosen_item == GO_BACK)
              break;
          switch (chosen_item)
          {
              case 0:
                  __reboot(LINUX_REBOOT_MAGIC1, LINUX_REBOOT_MAGIC2, LINUX_REBOOT_CMD_RESTART2, "recovery");
                  break;
              case 1:
                  __reboot(LINUX_REBOOT_MAGIC1, LINUX_REBOOT_MAGIC2, LINUX_REBOOT_CMD_RESTART2, "download");
                  break;
              case 2:
                  __reboot(LINUX_REBOOT_MAGIC1, LINUX_REBOOT_MAGIC2, LINUX_REBOOT_CMD_POWER_OFF, NULL);
                  break;
              case 3: {
                  gr_exit();
                  ev_exit();
                  ensure_root_path_mounted("SYSTEM:");
                  ensure_root_path_mounted("DATA:");
                  ensure_root_path_mounted("DATADATA:");
                  ensure_root_path_mounted("CACHE:");
                  __systemscript("/sbin/rec3e");
                  // should not happen
                  ui_init();
                  ui_print("Back from rec3e");
              }
                  break;
              case 4:
              {
                show_partition_sizes();  
                apply_root_menu();
                break;
              }
              case 5:
              {
                show_partition_sizes();  
                filesystem_menu();
                break;
              }
              case 6:
              {
                modules_menu();
                break;
              }
              case 7:
              {
                gfx_menu();  
                break;
              }
              case 8:
              {
                readahead_menu();  
                break;
              }
			  case 9:
              {
			    IO_sched_menu();
                break;
              }
              case 10:
              {
                lmk_menu(); 
                break;
              }
              case 11:
              {
                cpu_menu(); 
                break;
              }
              case 12:
              {
                touch_menu(); 
                break;
              }
              case 13:
              {
                show_partition_sizes();  
                cleanup_menu(); 
                break;
              }
          }
      }
}
