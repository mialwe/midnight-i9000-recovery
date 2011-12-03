/*
 * Copyright (C) 2010 Zsolt Sz Sztupák
 * Modified by HardCORE (Rodney Clinton Chua)
 * Modified and renamed (orig. lagfixutils.h) by mialwe (Michael Weingärtner)
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

#ifndef _MIDNIGHT_H
#define _MIDNIGHT_H

#include "roots.h"
void remove_root_from_device(void);
void apply_root_to_device(void);
void show_root_menu(void);
void show_advanced_menu(void);
int get_partition_free(const char *partition);
int file_exists(const char *filename);
int show_file_exists(const char *pre, const char *filename, const char *ui_filename, const char *post, const char *post_no);

#endif //_MIDNIGHT_H
