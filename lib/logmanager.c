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
#include <apr_strings.h>

#define APR_WANT_STRFUNC
#define APR_WANT_STDIO

#include "apr_want.h"

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

#include "../config.h"

#include "../common/global.h"
#include "../common/alloc.h"
#include "../common/convert.h"
#include "../common/path.h"
#include "inst_include/logmanager.h"
#include "include/config.h"
#include "../common/time.h"
#include "include/status.h"
#include "include/compress.h"
#include "include/plain_handler.h"
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
#include "include/debug.h"
#include "include/checksum.h"

/*----------------------------------------------*/

#define IS_OPEN(_mp) ((_mp)->active.fp)

#define NEW_LOGFILE() NEW_STRUCT(LOGFILE)

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
		FREE_P((_lp)->path); \
		FREE_P((_lp)->link); \
		} \
	FREE_P(_lp); \
	} \

#define SYNC_LOGFILE_FROM_DISK(_lp)	{ \
	if (_lp) \
		{ \
		if (file_exists((_lp)->path)) (_lp)->size=file_size((_lp)->path); \
		else DELETE_LOGFILE(_lp); \
		} \
	}

#define ACTIVE_SIZE(_mp) \
	((_mp)->active.file ? (_mp)->active.file->size : 0)

/* For added security */

#define CHECK_MP(_mp) {  \
	if (!(_mp)) FATAL_ERROR("Received null LOGMANAGER pointer"); \
	}

/*----------------------------------------------*/

static APR_INLINE BOOL should_rotate(LOGMANAGER *mp,apr_off_t add,TIMESTAMP t);
static APR_INLINE BOOL global_conditions_exceeded(LOGMANAGER *mp,apr_off_t add,TIMESTAMP t);
static void init_logmanager_paths(LOGMANAGER *mp,LOGMANAGER_OPTIONS *opts);
static void _open_active_file(LOGMANAGER *mp);
static void _close_active_file(LOGMANAGER *mp);
static void _new_active_file(LOGMANAGER *mp,TIMESTAMP t);
static void _sync_logfiles_from_disk(LOGMANAGER *mp);
static void free_logmanager_struct(LOGMANAGER *mp);

/*-----------*/
/* Other source files (non exported symbols) */
/* Files are included here because they can refer to the declarations above */

#include "../common/alloc.c"
#include "../common/convert.c"
#include "../common/path.c"
#include "../common/time.c"
#include "file.c"
#include "gzip_handler.c"
#include "plain_handler.c"
#include "bzip2_handler.c"
#include "compress.c"
#include "status.c"
#include "backup.c"
#include "stats.c"
#include "write.c"
#include "pid.c"
#include "link.c"
#include "cmd.c"
#include "debug.c"
#include "checksum.c"

/*----------------------------------------------*/

static APR_INLINE BOOL should_rotate(LOGMANAGER *mp,apr_off_t add,TIMESTAMP t)
{
apr_off_t future_size;

if (mp->file_maxsize && ACTIVE_SIZE(mp))
	{
	future_size=FUTURE_ACTIVE_SIZE(mp,add);
	if (future_size > mp->file_maxsize)
		{
		DEBUG3(mp,1,"Should rotate on size (add=%" APR_OFF_T_FMT ", future=%" APR_OFF_T_FMT ", limit=%" APR_OFF_T_FMT ")"
			,add,future_size,mp->file_maxsize);
		DEBUG1(mp,1,"Additional info : current=%" APR_OFF_T_FMT,ACTIVE_SIZE(mp));
		return YES;
		}
	}

if  (mp->rotate_delay
	&& mp->active.file
	&& (mp->active.file->start < (t - mp->rotate_delay)))
	{
	DEBUG(mp,1,"Should rotate on delay");
	return YES;
	}

return NO;
}

/*----------------------------------------------*/

static APR_INLINE BOOL global_conditions_exceeded(LOGMANAGER *mp,apr_off_t add,TIMESTAMP t)
{
apr_off_t future_size;

if ((mp->global_maxsize) && BACKUP_COUNT(mp))
	{
	future_size=FUTURE_ACTIVE_SIZE(mp,add) + BACKUP_SIZE(mp);
	if (future_size > mp->global_maxsize)
		{
		/* Purge on global size */
		DEBUG3(mp,1,"Global size conditions exceeded (add=%" APR_OFF_T_FMT ",future=%" APR_OFF_T_FMT ", limit=%" APR_OFF_T_FMT ")"
			,add,future_size,mp->global_maxsize);
		return YES;
		}
	}

if (mp->keep_count && (BACKUP_COUNT(mp) > (mp->keep_count - 1)))
	{
	DEBUG1(mp,1,"Global keep count exceeded (%d)",mp->keep_count);
	return YES;
	}

if (mp->purge_delay && BACKUP_COUNT(mp)
	&& (OLDEST_BACKUP_FILE(mp)->end < (t - mp->purge_delay)))
	{
	DEBUG(mp,1,"Purge delay exceeded");
	return YES;
	}

return NO;
}

/*----------------------------------------------*/

void logmanager_flush(LOGMANAGER *mp)
{
CHECK_MP(mp);

if (!IS_OPEN(mp)) return;

DEBUG1(mp,1,"Flushing %s",mp->active.file->path);
INCR_STAT_COUNT(mp,flush);

C_VOID_HANDLER(mp,flush);
}
	
/*----------------------------------------------*/
/*-- Root, PID, Status paths */

LIB_INTERNAL void init_logmanager_paths(LOGMANAGER *mp,LOGMANAGER_OPTIONS *opts)
{
mp->base_path=mk_abs_path(NULL,opts->base_path);
mp->base_dir=ut_dirname(mp->base_path);
mp->status_path=combine_strings(mp->base_path,".status");
if (opts->flags & LMGR_PID_FILE)
	mp->pid_path=combine_strings(mp->base_path,".pid");;
}

/*----------------------------------------------*/
/* NB: Status is read at creation time and written at close time (not destroy).
* This function must not write anything to the file system as it can be used
* for analysis (modifications can start when the manager is open()ed).
*/

LOGMANAGER *new_logmanager(LOGMANAGER_OPTIONS *opts)
{
LOGMANAGER *mp;

mp=NEW_STRUCT(LOGMANAGER);

init_logmanager_paths(mp,opts);

/*-- Flags */

mp->flags=opts->flags;

/*-- Compress engine */

mp->compress.handler=compress_handler_from_string(opts->compress.type);
if (! mp->compress.handler)
	FATAL_ERROR1("Invalid compression : %s",opts->compress.type);

DUP_P(mp->compress.level,opts->compress.level);

mp->compress.ratio=mp->compress.handler->default_ratio;

mp->compress.private=C_HANDLER3(mp,init,mp->compress.level,write_level3,(void *)mp);

/*-- Populates mp->active and mp->backup */

get_status_from_file(mp);
_sync_logfiles_from_disk(mp);

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

/*-- Debug info */

mp->debug.path=mk_abs_path(NULL,opts->debug_file);
mp->debug.level=opts->debug_level;

/* Rotate command */

mp->rotate_cmd=mk_abs_path(NULL,opts->rotate_cmd);

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

void logmanager_open(LOGMANAGER *mp,TIMESTAMP t)
{
CHECK_MP(mp);
NORMALIZE_TIMESTAMP(t);

if (IS_OPEN(mp)) return;

DEBUG(mp,1,"Opening log manager");

/*-- Create PID file */

create_pid_file(mp);

/* Open active file */

if (!mp->active.file)
	{
	_new_active_file(mp,t);
	dump_status_to_file(mp);
	}

_open_active_file(mp);

/*-- If options have changed, we can have to rotate and/or purge backups */

if (should_rotate(mp,0,t)) logmanager_rotate(mp,t);
else
	{
	purge_backup_files(mp,0,t);
	refresh_backup_links(mp);
	}
}

/*----------------------------------------------*/

static void free_logmanager_struct(LOGMANAGER *mp)
{
unsigned int i;

CHECK_MP(mp);

FREE_LOGFILE(mp->active.file);	/*-- Free the LOGFILE structs */

if (BACKUP_COUNT(mp))	/* Free backup array */
	{
	for (i=0;i<BACKUP_COUNT(mp);i++) FREE_LOGFILE(BACKUP_FILES(mp)[i]);
	ARRAY_CLEAR(mp->backup.files);
	}

FREE_P(mp->compress.private);

/* Free strings */

FREE_P(mp->base_path);
FREE_P(mp->base_dir);
FREE_P(mp->status_path);
FREE_P(mp->pid_path);
FREE_P(mp->debug.path);
FREE_P(mp->rotate_cmd);
FREE_P(mp->compress.level);

FREE_P(mp);	/* Last, free the envelope */
}

/*----------------------------------------------*/

void logmanager_destroy(LOGMANAGER *mp)
{
CHECK_MP(mp);

DEBUG(mp,1,"Destroying log manager");

if (IS_OPEN(mp)) logmanager_close(mp);	/*-- First, close the current log */

remove_pid_file(mp);	/*-- Remove the PID file */

C_VOID_HANDLER(mp,destroy);	/*-- Destroy compress handler */

debug_close(mp);	/*-- Close debug file */

free_logmanager_struct(mp);	/* Now, free whole struct*/
}

/*----------------------------------------------*/

static void _open_active_file(LOGMANAGER *mp)
{
if (IS_OPEN(mp)) return;

DEBUG1(mp,1,"Opening active file (%s)",mp->active.file->path);

mp->active.fp=file_open_for_append(mp->active.file->path,mp->create_mode);

refresh_active_link(mp);

C_VOID_HANDLER(mp,start);
}

/*----------------------------------------------*/

static void _close_active_file(LOGMANAGER *mp)
{
if (!IS_OPEN(mp)) return;

C_VOID_HANDLER(mp,stop);

/* Recompute compression ratio */
/* If original size is too small, compression ratio is not representative */

if (mp->active.file->osize > 10000)
	{
	mp->compress.ratio=mp->active.file->osize / mp->active.file->size;
	if (mp->compress.ratio == 0) mp->compress.ratio=1;	/* Security */
	}

mp->active.fp=file_close(mp->active.fp);
}

/*----------------------------------------------*/

void logmanager_close(LOGMANAGER *mp)
{
CHECK_MP(mp);

DEBUG(mp,1,"Closing logmanager");

write_end(mp);
_close_active_file(mp);

dump_status_to_file(mp);
}

/*----------------------------------------------*/

static void _new_active_file(LOGMANAGER *mp,TIMESTAMP t)
{
LOGFILE *lp;
int len;
char *path,*ep;
int i;

INCR_STAT_COUNT(mp,new_active_file);

lp=mp->active.file=NEW_LOGFILE();

len =64; /* should be enough for any suffix  (compression + number) */

len += strlen(mp->base_path)+11;
path=allocate(NULL,len);
(void)apr_snprintf(path,len,"%s._%08lX",mp->base_path,(unsigned long)t);

for (i=0,ep=path+strlen(path);;i++)
	{
	if (i) (void)apr_snprintf(ep,10,((i < 1000) ? "_%03d" : "_%d"),i);
	if (mp->compress.handler->suffix[0])
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

void logmanager_rotate(LOGMANAGER *mp,TIMESTAMP t)
{
CHECK_MP(mp);
NORMALIZE_TIMESTAMP(t);

DEBUG1(mp,1,"Starting rotation (%s)",mp->base_path);
INCR_STAT_COUNT(mp,rotate);

if (IS_OPEN(mp)) _close_active_file(mp);

/* Insert new backup first */

ARRAY_SHIFT(mp->backup.files);			/* Shift array */
BACKUP_FILES(mp)[0]=mp->active.file;	/* Fill first slot */
mp->backup.size += ACTIVE_SIZE(mp);		/* Add size to backup size */
mp->active.file=(LOGFILE *)0;			/* Clear active file */

if (mp->rotate_cmd) run_bg_cmd(mp,mp->rotate_cmd,BACKUP_FILES(mp)[0]->path,t);

purge_backup_files(mp,0,t);
refresh_backup_links(mp);

_new_active_file(mp,t);
_open_active_file(mp);

dump_status_to_file(mp);
}

/*----------------------------------------------*/
/* Purges the structure from non-existing files and (re)computes the
* individual and global sizes.
* NB: Don't check active file if it is open.
*/

static void _sync_logfiles_from_disk(LOGMANAGER *mp)
{
unsigned int i;
LOGFILE **lpp;

DEBUG(mp,1,"Syncing log files from disk");
INCR_STAT_COUNT(mp,sync);

if (! IS_OPEN(mp)) SYNC_LOGFILE_FROM_DISK(mp->active.file);

BACKUP_SIZE(mp)=0;

if (BACKUP_COUNT(mp))
	{
	for (i=0,lpp=BACKUP_FILES(mp);i<BACKUP_COUNT(mp);i++,lpp++)
		{
		SYNC_LOGFILE_FROM_DISK(*lpp);
		if (*lpp) BACKUP_SIZE(mp) += (*lpp)->size; /* TODO: corruption check here */
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
