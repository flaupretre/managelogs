/*=============================================================================

Copyright 2008 Francois Laupretre (francois@tekwire.net)

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
=============================================================================*/

#ifndef __LOGMANAGER_INT_H
#define __LOGMANAGER_INT_H

#include <apr.h>
#include <apr_file_io.h>
#include <apr_time.h>

#if APR_HAVE_STRING_H
#include <string.h>
#endif

/*----------------------------------------------*/

#include "./compress.h"
#include "./file.h"
#include "./array.h"
#include "./checksum.h"

typedef struct
	{
	char *path;
	char *link;
	TIMESTAMP start;
	TIMESTAMP end;
	apr_off_t size;  /* File (compressed) size */
	apr_off_t osize; /* Original (uncompressed) size */
	CHECKSUM sum;
	} LOGFILE;

typedef struct
	{
	char *base_dir;
	size_t base_dir_len; /* strlen(base_dir) */
	char *base_path;
	size_t base_path_len; /* strlen(base_path) */
	char *log_path;
	size_t log_path_len; /* strlen(log_path) */
	char *log_dir;
	char *status_path;
	char *pid_path;
	unsigned int flags;
	struct
		{
		COMPRESS_HANDLER *handler;
		void *private;
		unsigned long ratio;
		char *level;
		} compress;
	struct
		{
		OFILE *fp;
		LOGFILE *file;
		} active;
	struct
		{
		ARRAY(LOGFILE) files;
		apr_off_t size;
		} backup;
	apr_off_t file_maxsize;
	apr_off_t global_maxsize;
	unsigned int keep_count;
	TIMESTAMP rotate_delay;
	TIMESTAMP purge_delay;
	apr_fileperms_t create_mode;
	char *rotate_cmd;
	struct
		{
		char *buf;
		apr_off_t len;
		} eol_buffer;
	TIMESTAMP last_write_time;
	struct
		{
		char *path;
		OFILE *fp;
		int level;
		} debug;
	struct
		{
		int write_count;
		int write2_count;
		int write3_count;
		int flush_count;
		int link_count;
		int refresh_backup_links_count;
		int refresh_active_link_count;
		int new_active_file_count;
		int rotate_count;
		int remove_oldest_count;
		int dump_count;
		int sync_count;
		} stats;
	} _LOGMANAGER_STRUCT;

typedef _LOGMANAGER_STRUCT LOGMANAGER;

/*----------------------------------------------*/
#endif	/* __LOGMANAGER_INT_H */
