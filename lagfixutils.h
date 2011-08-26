/*
 * Copyright (C) 2010 Zsolt Sz Sztupák
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

#ifndef _LAGFIXUTILS_H
#define _LAGFIXUTILS_H

#include "roots.h"

void show_advanced_lfs_menu();
int graphsh_main(int argc, char** argv);
int graphchoice_main(int argc, char** argv);
int truncate_main(int argc, char** argv);
void apply_root_to_device();
void lagfix_menu();
int ensure_lagfix_mount_points(const RootInfo *info);
int ensure_lagfix_unmount_points(const RootInfo *info);
int ensure_lagfix_formatted(const RootInfo *info);
int lagfixer_main(int argc, char** argv);

#endif //_LAGFIXUTILS_H
