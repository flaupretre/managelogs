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

#ifndef __LOGMANAGER_H
#define __LOGMANAGER_H

#include <apr.h>
#include <apr_file_io.h>

#include "util.h"
#include "time.h"
#include "compress.h"

/*----------------------------------------------*/

#ifndef _WIN32
#define SYMLINK_SUPPORT
#define HARDLINK_SUPPORT
#endif

/*----------------------------------------------*/

#define LOGMANAGER_VERSION	"1.0rc1"

/* logmanager_write() flags */

#define LMGRW_CANNOT_ROTATE	0x1

/* LOGMANAGER flags */

#define LMGR_ACTIVE_LINK	0x1
#define LMGR_BACKUP_LINKS	0x2
#define LMGR_HARD_LINKS		0x4
#define LMGR_IGNORE_EOL		0x8

/*----------------------------------------------*/

typedef struct
	{
	char *root_path;
	unsigned int flags;
	char *compress_string;
	apr_off_t file_maxsize;
	apr_off_t global_maxsize;
	unsigned int keep_count;
	apr_fileperms_t create_mode;
	char *debug_file;
	int debug_level;
	} LOGMANAGER_OPTIONS_V1;

/*----------------------------------------------*/

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
	int api_version;
	char *root_path;
	char *pid_path;
	char *status_path;
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
		LOGFILE **files;
		unsigned int count;
		apr_off_t size;
		} backup;
	apr_off_t file_maxsize;
	apr_off_t global_maxsize;
	unsigned int keep_count;
	apr_fileperms_t create_mode;
	TIMESTAMP last_time;
	struct
		{
		char *buf;
		apr_off_t len;
		} eol_buffer;
	struct
		{
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
	} LOGMANAGER;

/*----------------------------------------------*/

extern LOGMANAGER *new_logmanager_v1(LOGMANAGER_OPTIONS_V1 *opts,TIMESTAMP t);
extern void logmanager_destroy(LOGMANAGER *mp,TIMESTAMP t);
extern void logmanager_open(LOGMANAGER *mp,TIMESTAMP t);
extern void logmanager_close(LOGMANAGER *mp,TIMESTAMP t);
extern void logmanager_write(LOGMANAGER *mp, const char *buf, apr_off_t size
	,unsigned int flags, TIMESTAMP t);
extern void logmanager_flush(LOGMANAGER *mp,TIMESTAMP t);
extern void logmanager_rotate(LOGMANAGER *mp,TIMESTAMP t);
extern char *logmanager_compression_list(void);
extern char *logmanager_version(void);
extern void logmanager_display_stats(LOGMANAGER *mp);

/*----------------------------------------------*/
#endif	/* __LOGMANAGER_H */
