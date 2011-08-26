/*
 * truncate implementation
 *
 * Copyright (C) 2010 Zsolt Sz Sztupak <mail@sztupy.hu>
 *
 * Licensed under GPLv2 or later, see file LICENSE in this tarball for details.
 */

#include "stdio.h"
#include "stdlib.h"
#include "unistd.h"
#include "sys/types.h"
#include "common.h"
#include "lagfixutils.h"

int truncate_main(int argc, char **argv)
{
    char* name;
    char* length;
    int l;

	  if (argc >= 2) {
		  name = argv[1];
		  if (name != NULL) {
        length = argv[2];
        l = atoi(length);
        truncate(name,l);
		  }
      return 0;
	  } else {
        printf("Usage: truncate filename length\n  filename must exist and length must be an integer\n");
        return -1;
    }
}

