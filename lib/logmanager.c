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

#define IN_LMGR_LIB

#include <apr.h>
#include <apr_signal.h>
#include <apr_time.h>
#include <apr_file_io.h>
#include <apr_env.h>
#include <apr_errno.h>

#if APR_HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif

#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif

#if APR_HAVE_UNISTD_H
#include <unistd.h>
#endif

#if APR_HAVE_STDLIB_H
#include <stdlib.h>
#endif

#if APR_HAVE_STRING_H
#include <string.h>
#endif

#if APR_HAVE_STRINGS_H
#include <strings.h>
#endif

#if APR_HAVE_STDIO_H
#include <stdio.h>
#endif

#include "../config.h"

#include "../util/util.h"
#include "inst_include/logmanager.h"
#include "include/config.h"
#include "include/time.h"
#include "include/status.h"
#include "include/compress.h"
#include "include/gzip_handler.h"
#include "include/bzip2_handler.h"
#include "include/file.h"
#include "include/array.h"
#include "include/backup.h"
#include "include/stats.h"
#include "include/write.h"
#include "include/pid.h"
#include "include/link.h"
#include "include/cmd.h"

/*----------------------------------------------*/

#define IS_OPEN(_mp) ((_mp)->active.fp)

#define NEW_LOGFILE() (LOGFILE *)allocate(NULL,sizeof(LOGFILE))

#define DELETE_LOGFILE(_lp)	{ \
	if (_lp) \
		{ \
		if ((_lp)->path) file_delete((_lp)->path,NO); \
		if ((_lp)->link) file_delete((_lp)->link,NO); \
		} \
	FREE_LOGFILE(_lp); \
	}

#define FREE_LOGFILE(_lp)	{ \
	if (_lp) \
		{ \
		if ((_lp)->path) (void)allocate((_lp)->path,0); \
		if ((_lp)->link) (void)allocate((_lp)->link,0); \
		} \
	(_lp)=allocate((_lp),0); \
	} \

#define SYNC_LOGFILE_FROM_DISK(_lp)	{ \
	if (_lp) \
		{ \
		if (file_exists((_lp)->path)) (_lp)->size=file_size((_lp)->path); \
		else DELETE_LOGFILE(_lp); \
		} \
	}

#define ACTIVE_SIZE(_mp) \
	((_mp)->active.fp ? (_mp)->active.fp->size : \
		((_mp)->active.file ? (_mp)->active.file->size : 0))

#define SHOULD_ROTATE(_mp,_add,_t)	( \
		((_mp)->file_maxsize \
			&& (ACTIVE_SIZE(_mp)) \
			&& (FUTURE_ACTIVE_SIZE(_mp,_add) > (_mp)->file_maxsize)) \
	||	((_mp)->rotate_delay && (_mp)->active.file \
			&& ((_mp)->active.file->start < ((_t)-(_mp)->rotate_delay))) \
	)

#define GLOBAL_CONDITIONS_EXCEEDED(_mp,_add,_t)	(\
		(((_mp)->global_maxsize) \
			&& (BACKUP_COUNT(_mp)) \
			&& ((FUTURE_ACTIVE_SIZE((_mp),_add) + BACKUP_SIZE(_mp)) \
				> (_mp)->global_maxsize)) \
	|| (_mp->keep_count \
		&& (BACKUP_COUNT(_mp) > (_mp->keep_count - 1))) \
	|| (_mp->purge_delay && BACKUP_COUNT(_mp) \
		&& (OLDEST_BACKUP_FILE(_mp)->end < ((_t)-(_mp)->purge_delay))) \
	)

/* For added security */

#define CHECK_MP(_mp) {  \
	if (!(_mp)) FATAL_ERROR("Received null LOGMANAGER pointer"); \
	}

/*----------------------------------------------*/

static void _open_active_file(LOGMANAGER mp);
static void _close_active_file(LOGMANAGER mp);
static void _new_active_file(LOGMANAGER mp,TIMESTAMP t);
static void _sync_logfiles_from_disk(LOGMANAGER mp);
static char *_absolute_path(LOGMANAGER mp, const char *str);
static char *_dirname(const char *path);
static const char *_basename(const char *path);

/*-----------*/
/* Other source files (non exported symbols) */
/* Files are included here because they can refer to the declarations above */

#include "../util/util.c"
#include "file.c"
#include "time.c"
#include "compress.c"
#include "gzip_handler.c"
#include "bzip2_handler.c"
#include "status.c"
#include "backup.c"
#include "stats.c"
#include "write.c"
#include "pid.c"
#include "link.c"
#include "cmd.c"

/*----------------------------------------------*/

void logmanager_flush(LOGMANAGER mp,TIMESTAMP t)
{
CHECK_MP(mp);

if (!IS_OPEN(mp)) return;

DEBUG1(mp,1,"Flushing %s",mp->active.file->path);
INCR_STAT_COUNT(flush);

C_HANDLER(mp,flush);
}
	
/*----------------------------------------------*/
/* NB: Status is read at creation time and written at close time (not destroy).
* This function must not write anything to the file system as it can be used
* for analysis (modifications can start when the manager is open()ed).
*/

LOGMANAGER new_logmanager(LOGMANAGER_OPTIONS *opts,TIMESTAMP t)
{
LOGMANAGER mp;

mp=(LOGMANAGER )allocate(NULL,sizeof(*mp));

/*-- Initial timestamp */

NORMALIZE_TIMESTAMP(t);
mp->last_time=t;

/*-- Root, PID, Status paths */

mp->base_path=duplicate(opts->base_path);
mp->root_dir=_dirname(mp->base_path);
mp->status_path=status_path(mp);
mp->pid_path=pid_path(mp);

/*-- Flags */

mp->flags=opts->flags;

/*-- Compress engine */

init_compress_handler_from_string(mp,opts->compress_string);

/*-- Populates mp->active and mp->backup */

get_status_from_file(mp);

/*-- Max sizes and limits */
/* Ensures that file limit is lower than global limit. If global limit is set
* and file limit is not, file limit is set to global limit / 2 */

mp->keep_count=opts->keep_count;
mp->file_maxsize=opts->file_maxsize;
mp->global_maxsize=opts->global_maxsize;

if (mp->file_maxsize == 1) mp->file_maxsize=FILE_LOWER_LIMIT;
if (mp->file_maxsize && (mp->file_maxsize < FILE_LOWER_LIMIT))
	FATAL_ERROR2("File limit cannot be less than %d (is %d)"
		,FILE_LOWER_LIMIT,(int)(mp->file_maxsize));

if (mp->global_maxsize)
	{
	if (mp->global_maxsize == 1) mp->global_maxsize=GLOBAL_LOWER_LIMIT;
	if (mp->global_maxsize < GLOBAL_LOWER_LIMIT)
		FATAL_ERROR2("Global limit cannot be less than %d (is %d)"
			,GLOBAL_LOWER_LIMIT,(int)(mp->global_maxsize));
	if (! mp->file_maxsize) mp->file_maxsize=mp->global_maxsize/2;
	if (mp->global_maxsize < mp->file_maxsize)
		FATAL_ERROR("Global limit cannot be less than file limit");
	}

/* Write back actual limits to the options struct */

opts->file_maxsize=mp->file_maxsize;
opts->global_maxsize=mp->global_maxsize;

/*-- File creation mode */

mp->create_mode=opts->create_mode;
if (!mp->create_mode) mp->create_mode=0x644;

/*-- Open debug file */

if (opts->debug_file)
	{
	mp->debug.fp=file_open_for_append(opts->debug_file,(apr_int32_t)PIDFILE_MODE);
	mp->debug.level=opts->debug_level;
	}

/* Rotate command */

mp->rotate_cmd=duplicate(opts->rotate_cmd);

/* V 2+ specific options */

if (opts->api_version >= 2)
	{
	mp->rotate_delay=opts->rotate_delay;
	mp->purge_delay=opts->purge_delay;
	}

/*--*/

return mp;
}

/*----------------------------------------------*/

void logmanager_open(LOGMANAGER mp,TIMESTAMP t)
{
CHECK_MP(mp);
CHECK_TIME(mp,t);

if (IS_OPEN(mp)) return;

DEBUG(mp,1,"Opening log manager");

/*-- Create PID file */

create_pid_file(mp);

/* Open active file */

if (!mp->active.file)
	{
	_new_active_file(mp,t);
	dump_status_to_file(mp,t);
	}

_open_active_file(mp);

/*-- If options have changed, we can have to rotate and/or purge backups */

if (SHOULD_ROTATE(mp,0,t)) logmanager_rotate(mp,t);
else
	{
	purge_backup_files(mp,0,t);
	refresh_backup_links(mp);
	}
}

/*----------------------------------------------*/
/* Note: Don't remove the pid file if it has been overwritten by another 
log manager (happens with error_log when apache starts) */

void logmanager_destroy(LOGMANAGER mp,TIMESTAMP t)
{
int i;

CHECK_MP(mp);

DEBUG(mp,1,"Destroying log manager");

/*-- First, close the current log if not already done */

if (IS_OPEN(mp)) logmanager_close(mp,t);

/*-- Remove the PID file */

if (!pid_file_is_overwritten(mp)) remove_pid_file(mp);

/*-- Destroy compress handler */

C_HANDLER(mp,destroy);

/*-- Free the LOGFILE structs */

FREE_LOGFILE(mp->active.file);

if (BACKUP_COUNT(mp))
	{
	for (i=0;i<BACKUP_COUNT(mp);i++) FREE_LOGFILE(BACKUP_FILES(mp)[i]);
	ARRAY_CLEAR(mp->backup.files);
	}

/*-- Close debug file */

if (mp->debug.fp) mp->debug.fp=file_close(mp->debug.fp);

/* Free paths */

(void)allocate(mp->base_path,0);
(void)allocate(mp->root_dir,0);
(void)allocate(mp->status_path,0);
(void)allocate(mp->pid_path,0);

/* Last, free the envelope */

(void)allocate(mp,0);
}

/*----------------------------------------------*/
/* Return a pointer to the char after the last separator. If a separator
is not found, return a pointer to the full string.
Warning : the output is not duplicated : it is a pointer inside the input */

static const char *_basename(const char *path)
{
const char *p;
char c;
int i;

for (i=strlen(path);;i--)
	{
	if (!i) break;
	c=(*(p=path+i));
	if (!c) continue; /* First char of non-empty string */
	if ((c=='/')||(c=='\\')) return (p+1);
	}
return path;
}

/*----------------------------------------------*/
/* Return a duplicate of the dirname of a path (with the trailing separator) */
/* An empty string and a string without separator return a null pointer */

static char *_dirname(const char *path)
{
const char *p;
char *p2,c;
int i;

for (i=strlen(path)-1;;i--)
	{
	if (i < 0) return NULL;
	c=(*(p=&(path[i])));
	if ((c=='/')||(c=='\\'))
		{
		p2=duplicate_mem(path,i+2);
		p2[i+1]='\0';
		return p2;
		}
	}
}

/*----------------------------------------------*/

static void _open_active_file(LOGMANAGER mp)
{
if (IS_OPEN(mp)) return;

DEBUG1(mp,1,"Opening active file (%s)",mp->active.file->path);

mp->active.fp=file_open_for_append(mp->active.file->path,mp->create_mode);

refresh_active_link(mp);

C_HANDLER(mp,start);
}

/*----------------------------------------------*/

static void _close_active_file(LOGMANAGER mp)
{
if (!IS_OPEN(mp)) return;

C_HANDLER(mp,end);
mp->active.file->size=mp->active.fp->size;

mp->active.fp=file_close(mp->active.fp);
}

/*----------------------------------------------*/

void logmanager_close(LOGMANAGER mp,TIMESTAMP t)
{
CHECK_MP(mp);
CHECK_TIME(mp,t);

DEBUG(mp,1,"Closing logmanager");

write_end(mp,t);
_close_active_file(mp);
purge_backup_files(mp,0,t);

dump_status_to_file(mp,t);
}

/*----------------------------------------------*/

static void _new_active_file(LOGMANAGER mp,TIMESTAMP t)
{
LOGFILE *lp;
int len;
char *path,*ep;
int i;

INCR_STAT_COUNT(new_active_file);

lp=mp->active.file=NEW_LOGFILE();

len =64; /* should be enough for any suffix  (compression + number) */

len += strlen(mp->base_path)+11;
path=allocate(NULL,len);
(void)snprintf(path,len,"%s._%08lX",mp->base_path,t);

for (i=0,ep=path+strlen(path);;i++)
	{
	if (i) sprintf(ep,((i < 1000) ? ".%03d" : ".%d"),i);
	if (mp->compress.handler->suffix)
		{
		strcat(ep,".");
		strcat(ep,mp->compress.handler->suffix);
		}
	if (!file_exists(path)) break;
	}

lp->path=path;
lp->start=lp->end=t;
}

/*----------------------------------------------*/

void logmanager_rotate(LOGMANAGER mp,TIMESTAMP t)
{
CHECK_MP(mp);
CHECK_TIME(mp,t);

DEBUG1(mp,1,"Starting rotation (%s)",mp->base_path);
INCR_STAT_COUNT(rotate);

if (IS_OPEN(mp)) _close_active_file(mp);

/* Insert new backup first */

ARRAY_SHIFT(mp->backup.files);			/* Shift array */
BACKUP_FILES(mp)[0]=mp->active.file;	/* Fill first slot */
mp->backup.size += ACTIVE_SIZE(mp);		/* Add size to backup size */
mp->active.file=(LOGFILE *)0;			/* Clear active file */

run_bg_cmd(mp,mp->rotate_cmd,BACKUP_FILES(mp)[0],t);

purge_backup_files(mp,0,t);
refresh_backup_links(mp);

_new_active_file(mp,t);
_open_active_file(mp);

dump_status_to_file(mp,t);
}

/*----------------------------------------------*/

static char *_absolute_path(LOGMANAGER mp, const char *str)
{
char *p;
int len;

if (!mp->root_dir) return duplicate(str);

p=allocate(NULL,len=strlen(mp->root_dir)+strlen(str)+2);
snprintf(p,len,"%s%s",mp->root_dir,str);
return p;
}

/*----------------------------------------------*/
/* Purges the structure from non-existing files and (re)computes the
* individual and global sizes.
* NB: Don't check active file if it is open.
*/

static void _sync_logfiles_from_disk(LOGMANAGER mp)
{
int i;
LOGFILE **lpp;

DEBUG(mp,1,"Syncing log files from disk");
INCR_STAT_COUNT(sync);

if (! IS_OPEN(mp)) SYNC_LOGFILE_FROM_DISK(mp->active.file);

BACKUP_SIZE(mp)=0;

if (BACKUP_COUNT(mp))
	{
	for (i=0,lpp=BACKUP_FILES(mp);i<BACKUP_COUNT(mp);i++,lpp++)
		{
		SYNC_LOGFILE_FROM_DISK(*lpp);
		if (*lpp) BACKUP_SIZE(mp) += (*lpp)->size;
		}
	ARRAY_PACK(mp->backup.files);
	}
}

/*----------------------------------------------*/

char *logmanager_version()
{
return duplicate(PACKAGE_VERSION);
}

/*----------------------------------------------*/
