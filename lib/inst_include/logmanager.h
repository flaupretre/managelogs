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

/* The data exposed here is everything a program needs to use the logmanager
* library */

#ifndef __LOGMANAGER_H
#define __LOGMANAGER_H

#include <apr.h>
#include <apr_file_io.h>

/*----------------------------------------------*/

#ifndef _WIN32
#define SYMLINK_SUPPORT
#define HARDLINK_SUPPORT
#endif

/*----------------------------------------------*/

#ifndef NOW
typedef unsigned long TIMESTAMP;

#define NOW	(TIMESTAMP)0
#endif

/*----------------------------------------------*/
/* logmanager_write() flags */

#define LMGRW_CANNOT_ROTATE	0x1

/* LOGMANAGER option flags */

#define LMGR_ACTIVE_LINK	0x01
#define LMGR_BACKUP_LINKS	0x02
#define LMGR_HARD_LINKS		0x04
#define LMGR_IGNORE_EOL		0x08
#define LMGR_FAIL_ENOSPC	0x10

/* Max size of compress method */

#define LMGR_COMPRESS_SIZE	5
#define LMGR_COMPRESS_STRING_SIZE	(LMGR_COMPRESS_SIZE+5)

/*----------------------------------------------*/

#define LOGMANAGER_API_VERSION	2

/*----------------------------------------------*/

typedef struct
	{
	unsigned int api_version;
	char *base_path;
	unsigned int flags;
	char compress_string[LMGR_COMPRESS_STRING_SIZE+1];
	apr_off_t file_maxsize;
	apr_off_t global_maxsize;
	unsigned int keep_count;
	apr_fileperms_t create_mode;
	char *debug_file;
	int debug_level;
	char *rotate_cmd;
	/* API version 1 stops here */
	TIMESTAMP rotate_delay;
	TIMESTAMP purge_delay;
	} LOGMANAGER_OPTIONS;

/*----------------------------------------------*/

#ifdef IN_LMGR_LIB

#include "../include/compress.h"
#include "../include/file.h"
#include "../include/array.h"

typedef struct
	{
	char *path;
	char *link;
	TIMESTAMP start;
	TIMESTAMP end;
	apr_off_t size; /* Invalid for active file when open */
	} LOGFILE;

typedef struct
	{
	char *root_dir;
	char *base_path;
	char *status_path;
	char *pid_path;
	unsigned int flags;
	struct
		{
		COMPRESS_HANDLER *handler;
		void *private;
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

typedef _LOGMANAGER_STRUCT * LOGMANAGER;

#else
typedef void * LOGMANAGER; /* Opaque to client */
#endif

/*----------------------------------------------*/
/* Functions */

extern LOGMANAGER new_logmanager(LOGMANAGER_OPTIONS *opts);
extern void logmanager_destroy(LOGMANAGER mp);
extern void logmanager_open(LOGMANAGER mp,TIMESTAMP t);
extern void logmanager_close(LOGMANAGER mp);
extern void logmanager_write(LOGMANAGER mp, const char *buf, apr_off_t size
	,unsigned int flags, TIMESTAMP t);
extern void logmanager_flush(LOGMANAGER mp);
extern void logmanager_rotate(LOGMANAGER mp,TIMESTAMP t);
extern char *logmanager_compression_list(void);
extern char *logmanager_version(void);
extern void logmanager_display_stats(LOGMANAGER mp);

/*----------------------------------------------*/
#endif	/* __LOGMANAGER_H */
