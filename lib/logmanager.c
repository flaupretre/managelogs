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

#include <apr.h>
#include <apr_signal.h>

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


#include "include/logmanager.h"
#include "include/config.h"
#include "include/time.h"
#include "include/util.h"

/*----------------------------------------------*/

#define C_HANDLER(_action) \
	((mp->compress.handler->_action) \
		? mp->compress.handler->_action(mp) : 0)

#define C_HANDLER1(_action,_arg1) \
	((mp->compress.handler->_action) \
		? mp->compress.handler->_action(mp,_arg1) : 0)

#define C_HANDLER2(_action,_arg1,_arg2) \
	((mp->compress.handler->_action) \
		? mp->compress.handler->_action(mp,_arg1,_arg2) : 0)

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
		else FREE_LOGFILE(_lp); \
		} \
	}

#define ACTIVE_SIZE(_mp) \
	((_mp)->active.fp ? (_mp)->active.fp->size : \
		((_mp)->active.file ? (_mp)->active.file->size : 0))

#define ROTATE_IF_NEEDED(_mp,_add,_t)	{ \
	if ((_mp)->file_maxsize \
		&& (ACTIVE_SIZE(_mp)) \
		&& ((ACTIVE_SIZE(_mp)+_add) > (_mp)->file_maxsize)) \
		logmanager_rotate((_mp),_t); \
	} \

#define SHOULD_ROTATE(_mp,_add)	((_mp)->file_maxsize \
		&& (ACTIVE_SIZE(_mp)) \
		&& ((ACTIVE_SIZE(_mp)+_add) > (_mp)->file_maxsize))

#define GLOBAL_SIZE_EXCEEDED(_mp,_add)	(((_mp)->global_maxsize) \
	&& ((_mp)->backup.count) \
	&& ((ACTIVE_SIZE(_mp)+_add+(_mp)->backup.size) > (_mp)->global_maxsize))

#define CHECK_MP() { /* Security */ \
	if (!mp) FATAL_ERROR("Received null LOGMANAGER pointer"); \
	}

#define CORRECT_TIME(_t)	{ if (!t) t=time_now(); }

#define CHECK_TIME(_mp,_t)	{ \
	CORRECT_TIME(t); \
	(_mp)->last_time=_t; \
	}

#define STAT_COUNT_ITEM(_item)	mp->stats._item ## _count

#define INCR_STAT_COUNT(_item)	{ STAT_COUNT_ITEM(_item)++; }

/*----------------------------------------------*/

static void _create_pid_file(LOGMANAGER *mp);
static void _remove_pid_file(LOGMANAGER *mp);
static void _open_active_file(LOGMANAGER *mp);
static void _close_active_file(LOGMANAGER *mp);
static void _new_active_file(LOGMANAGER *mp,TIMESTAMP t);
static void _purge_backup_files(LOGMANAGER *mp,apr_off_t add);
static void _remove_oldest_backup(LOGMANAGER *mp);
static void _get_status_from_file(LOGMANAGER *mp);
static void _dump_status_to_file(LOGMANAGER *mp,TIMESTAMP t);
static void _sync_logfiles_from_disk(LOGMANAGER *mp);
static char *_link_name(LOGMANAGER *mp, int num);
static void _refresh_backup_links(LOGMANAGER *mp);
static void _refresh_active_link(LOGMANAGER *mp);
static void _clear_logfile_link(LOGMANAGER *mp, LOGFILE *lp);
static void _create_logfile_link(LOGMANAGER *mp, LOGFILE *lp,int num);
static void _write_end(LOGMANAGER *mp, TIMESTAMP t);
static void _write_level2(LOGMANAGER *mp, const char *buf, apr_off_t size
	,unsigned int flags, TIMESTAMP t);

/*----------------------------------------------*/

static void _create_pid_file(LOGMANAGER *mp)
{
OFILE *fp;
char buf[32];

DEBUG1(mp,1,"Creating PID file (%s)",mp->pid_path);

fp=file_create(mp->pid_path,(apr_int32_t)PIDFILE_MODE);

(void)snprintf(buf,sizeof(buf),"%lu",(unsigned long)getpid());
file_write_string_nl(fp, buf);

(void)file_close(fp);
}

/*----------------------------------------------*/

static void _remove_pid_file(LOGMANAGER *mp)
{
DEBUG(mp,1,"Removing PID file");

(void)file_delete(mp->pid_path,NO);
}

/*----------------------------------------------*/

void logmanager_flush(LOGMANAGER *mp,TIMESTAMP t)
{
CHECK_MP();

if (!IS_OPEN(mp)) return;

DEBUG1(mp,1,"Flushing %s",mp->active.file->path);
INCR_STAT_COUNT(flush);

C_HANDLER(flush);
}
	
/*----------------------------------------------*/
/* NB: Status is read at creation time and written at close time (not destroy).
* This function must not write anything to the file system as it can be used
* for analysis (modifications can start when the manager is open()ed).
*/

LOGMANAGER *new_logmanager_v1(LOGMANAGER_OPTIONS_V1 *opts,TIMESTAMP t)
{
LOGMANAGER *mp;
int len;

mp=(LOGMANAGER *)allocate(NULL,sizeof(LOGMANAGER));

/*-- API version */

mp->api_version=1;

/*-- Initial timestamp */

CORRECT_TIME(t);
mp->last_time=t;

/*-- Root, PID, Status paths */

mp->root_path=duplicate(opts->root_path);

mp->pid_path=allocate(NULL,len=(strlen(mp->root_path)+5));
(void)snprintf(mp->pid_path,len,"%s.pid",mp->root_path);

mp->status_path=allocate(NULL,len=(strlen(mp->root_path)+8));
(void)snprintf(mp->status_path,len,"%s.status",mp->root_path);

/*-- Flags */

mp->flags=opts->flags;

/*-- Compress engine */

init_compress_handler_from_string(mp,opts->compress_string);

/*-- Populates mp->active and mp->backup */

_get_status_from_file(mp);

/*-- Max sizes and limits */
/* Ensures that file limit is lower than global limit. If global limit is set
* and file limit is not, file limit is set to global limit / 2 */

mp->keep_count=opts->keep_count;
mp->file_maxsize=opts->file_maxsize;
mp->global_maxsize=opts->global_maxsize;

if (mp->file_maxsize == 1) mp->file_maxsize=FILE_LOWER_LIMIT;
if (mp->file_maxsize && (mp->file_maxsize < FILE_LOWER_LIMIT))
	FATAL_ERROR2("File limit cannot be less than %d (is %d)"
		,FILE_LOWER_LIMIT,mp->file_maxsize);

if (mp->global_maxsize)
	{
	if (mp->global_maxsize == 1) mp->global_maxsize=GLOBAL_LOWER_LIMIT;
	if (mp->global_maxsize < GLOBAL_LOWER_LIMIT)
		FATAL_ERROR2("Global limit cannot be less than %d (is %d)"
			,GLOBAL_LOWER_LIMIT,mp->global_maxsize);
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

/*--*/

return mp;
}

/*----------------------------------------------*/

void logmanager_open(LOGMANAGER *mp,TIMESTAMP t)
{
CHECK_MP();
CHECK_TIME(mp,t);

if (IS_OPEN(mp)) return;

DEBUG(mp,1,"Opening log manager");

/*-- Create PID file */

_create_pid_file(mp);

/* Open active file */

if (!mp->active.file) _new_active_file(mp,t);
_open_active_file(mp);

/*-- If options have changed, we can have to rotate and/or purge backups */

if (SHOULD_ROTATE(mp,0)) logmanager_rotate(mp,t);
else
	{
	_purge_backup_files(mp,0);
	_refresh_backup_links(mp);
	}
}

/*----------------------------------------------*/

void logmanager_destroy(LOGMANAGER *mp,TIMESTAMP t)
{
int i;

CHECK_MP();

DEBUG(mp,1,"Destroying log manager");

/*-- First, close the current log if not already done */

if (IS_OPEN(mp)) logmanager_close(mp,t);

/*-- Remove the PID file */

_remove_pid_file(mp);

/*-- Destroy compress handler */

C_HANDLER(destroy);

/*-- Free the LOGFILE structs */

FREE_LOGFILE(mp->active.file);

if (mp->backup.count)
	{
	for (i=0;i<mp->backup.count;i++) FREE_LOGFILE(mp->backup.files[i]);
	}

/*-- Close debug file */

if (mp->debug.fp) mp->debug.fp=file_close(mp->debug.fp);

/* Free paths */

(void)allocate(mp->root_path,0);
(void)allocate(mp->pid_path,0);
(void)allocate(mp->status_path,0);

/* Last, free the envelope */

(void)allocate(mp,0);
}

/*----------------------------------------------*/

static char *_link_name(LOGMANAGER *mp, int num)
{
int len;
char buf[32],*p;

p=allocate(NULL,len=strlen(mp->root_path)+1);
strcpy(p,mp->root_path);

if (num)
	{
	snprintf(buf,sizeof(buf),".%d",num);
	p=allocate(p,len += strlen(buf));
	strcat(p,buf);
	}

if (mp->compress.handler->suffix)
	{
	p=allocate(p,len += strlen(mp->compress.handler->suffix));
	strcat(p,".");
	strcat(p,mp->compress.handler->suffix);
	}

return p;
}

/*----------------------------------------------*/

static void _clear_logfile_link(LOGMANAGER *mp, LOGFILE *lp)
{
if (lp && lp->link)
	{
	file_delete(lp->link,NO);
	lp->link=allocate(lp->link,0);
	}
}

/*----------------------------------------------*/

static void _create_logfile_link(LOGMANAGER *mp, LOGFILE *lp,int num)
{
char *lname;

if (!lp) return;

if (((num==0) && (mp->flags & LMGR_ACTIVE_LINK))
	|| ((num!=0) && (mp->flags & LMGR_BACKUP_LINKS)))
	{
	INCR_STAT_COUNT(link);
	lname=_link_name(mp,num);
	if (mp->flags & LMGR_HARD_LINKS)
		{
		file_delete(lname,NO);
#ifdef HARDLINK_SUPPORT
		(void)link(lp->path,lp->link=lname);
#endif
		}
	else
		{
		file_delete(lname,NO);
#ifdef SYMLINK_SUPPORT
		(void)symlink(lp->path,lp->link=lname);
#endif
		}
	}
else
	{
	lp->link=NULL;
	}
}

/*----------------------------------------------*/
/* We must proceed in 2 steps : first clear all, then (re)create all.
* If we don't do that, clear can destroy new links
*/

static void _refresh_backup_links(LOGMANAGER *mp)
{
int i;

DEBUG(mp,1,"Refreshing backup links");
INCR_STAT_COUNT(refresh_backup_links);

if (mp->backup.count)
	{
	for (i=0;i<mp->backup.count;i++)
		{
		_clear_logfile_link(mp,mp->backup.files[i]);
		}

	for (i=0;i<mp->backup.count;i++)
		{
		_create_logfile_link(mp,mp->backup.files[i],i+1);
		}
	}
}

/*----------------------------------------------*/

static void _refresh_active_link(LOGMANAGER *mp)
{
DEBUG(mp,1,"Refreshing active link");
INCR_STAT_COUNT(refresh_active_link);

_clear_logfile_link(mp,mp->active.file);
_create_logfile_link(mp,mp->active.file,0);
}

/*----------------------------------------------*/

static void _open_active_file(LOGMANAGER *mp)
{
if (IS_OPEN(mp)) return;

DEBUG1(mp,1,"Opening active file (%s)",mp->active.file->path);

mp->active.fp=file_open_for_append(mp->active.file->path,mp->create_mode);

_refresh_active_link(mp);

C_HANDLER(start);
}

/*----------------------------------------------*/

static void _close_active_file(LOGMANAGER *mp)
{
if (!IS_OPEN(mp)) return;

C_HANDLER(end);
mp->active.file->size=mp->active.fp->size;

mp->active.fp=file_close(mp->active.fp);
}

/*----------------------------------------------*/

void logmanager_close(LOGMANAGER *mp,TIMESTAMP t)
{
CHECK_MP();
CHECK_TIME(mp,t);

DEBUG(mp,1,"Closing logmanager");

_write_end(mp,t);
_close_active_file(mp);

_dump_status_to_file(mp,t);
}

/*----------------------------------------------*/

static void _new_active_file(LOGMANAGER *mp,TIMESTAMP t)
{
LOGFILE *lp;
int len;
char *path;

INCR_STAT_COUNT(new_active_file);

lp=mp->active.file=NEW_LOGFILE();

len=strlen(mp->root_path)+12;
if (mp->compress.handler->suffix) len+=(strlen(mp->compress.handler->suffix)+1);

for (path=NULL;;t++)
	{
	path=allocate(path,len=(strlen(mp->root_path)+11));
	(void)snprintf(path,len,"%s._%08lX",mp->root_path,t);
	if (mp->compress.handler->suffix)
		{
		path=allocate(path,len += (strlen(mp->compress.handler->suffix)+1));
		strcat(path,".");
		strcat(path,mp->compress.handler->suffix);
		}
	if (!file_exists(path)) break;
	}

lp->path=path;
lp->start=lp->end=t;
}

/*----------------------------------------------*/

void logmanager_rotate(LOGMANAGER *mp,TIMESTAMP t)
{
int i;

CHECK_MP();
CHECK_TIME(mp,t);

DEBUG1(mp,1,"Starting rotation (%s)",mp->root_path);
INCR_STAT_COUNT(rotate);

if (IS_OPEN(mp)) _close_active_file(mp);

/* Insert new backup first */

mp->backup.size += ACTIVE_SIZE(mp);
mp->backup.files=(LOGFILE **)allocate(mp->backup.files
	,(mp->backup.count+1)*sizeof(LOGFILE *));
if (mp->backup.count)	/* Shift */
	{
	for (i=mp->backup.count;i>0;i--) mp->backup.files[i]=mp->backup.files[i-1];
	}

mp->backup.count++;
mp->backup.files[0]=mp->active.file;
mp->active.file=(LOGFILE *)0;

_purge_backup_files(mp,0);
_refresh_backup_links(mp);

_new_active_file(mp,t);
_open_active_file(mp);

_dump_status_to_file(mp,t);
}

/*----------------------------------------------*/

static void _purge_backup_files(LOGMANAGER *mp,apr_off_t add)
{
while (GLOBAL_SIZE_EXCEEDED(mp,add)
	|| ((mp->keep_count) && (mp->backup.count > (mp->keep_count - 1))))
	{
	_remove_oldest_backup(mp);
	}
}

/*----------------------------------------------*/

static void _remove_oldest_backup(LOGMANAGER *mp)
{
if (! mp->backup.count) return; /* Should never happen */

DEBUG1(mp,1,"Removing oldest backup file (%s)"
	,mp->backup.files[mp->backup.count-1]->path);
INCR_STAT_COUNT(remove_oldest);

mp->backup.count--;
mp->backup.size -= mp->backup.files[mp->backup.count]->size;

DELETE_LOGFILE(mp->backup.files[mp->backup.count]);

mp->backup.files=allocate(mp->backup.files,mp->backup.count*sizeof(LOGFILE *));
}

/*----------------------------------------------*/
/* Called when the manager is closed. Even if we don't have an EOL, we
* must write the buffer */

static void _write_end(LOGMANAGER *mp, TIMESTAMP t)
{
_write_level2(mp,mp->eol_buffer.buf,mp->eol_buffer.len,0,t);
mp->eol_buffer.buf=allocate(mp->eol_buffer.buf,mp->eol_buffer.len=0);
}

/*----------------------------------------------*/
/* Buffer output so that the files are cut on line boundaries ('\n' char) */

void logmanager_write(LOGMANAGER *mp, const char *buf, apr_off_t size
	,unsigned int flags, TIMESTAMP t)
{
int i;

/*DEBUG1(mp,2,"Starting logmanager_write (size=%lu)",size);*/
INCR_STAT_COUNT(write);

if ((!buf) || (!size)) return;

if (mp->flags & LMGR_IGNORE_EOL)
	{
	_write_level2(mp,buf,size,flags,t);
	return;
	}

/* 1. If eol_buffer contains some data from a previous write, search a '\n'
from the beginning. If found, output the buffer and input data up to \n,
truncate data. If not found, append data to the buffer */

if (mp->eol_buffer.buf)
	{
	for (i=0;i<size;i++)
		{
		if (buf[i]=='\n')
			{
			DEBUG1(mp,2,"Flushing %lu bytes from eol buffer",mp->eol_buffer.len);
			_write_level2(mp,mp->eol_buffer.buf,mp->eol_buffer.len,flags,t);
			mp->eol_buffer.buf=allocate(mp->eol_buffer.buf,mp->eol_buffer.len=0);
			_write_level2(mp,buf,i+1,flags|LMGRW_CANNOT_ROTATE,t);
			buf += (i+1);
			size -= (i+1);
			break;
			}
		}
	if (mp->eol_buffer.buf) /* if not found, append to eol_buffer.buf */
		{
		DEBUG1(mp,2,"Appending %lu bytes to eol buffer",size);
		mp->eol_buffer.buf=allocate(mp->eol_buffer.buf
			,mp->eol_buffer.len+size);
		memcpy(&(mp->eol_buffer.buf[mp->eol_buffer.len]),buf,size);
		mp->eol_buffer.len += size;
		buf += size;
		size=0;
		}
	}

/* 2. Search last \n. If found, move trailing data to eol_buffer and truncate.
If not found, put everything in eol_buffer and return.
When entering this section, eol_buffer is always empty. */

if (!size) return;

for (i=size-1;;i--)
	{
	if ((i<0) || (buf[i]=='\n'))
		{
		mp->eol_buffer.len=(size-i-1);
		if (mp->eol_buffer.len)
			{
			DEBUG1(mp,2,"Storing %lu bytes in eol buffer",mp->eol_buffer.len);
			size=i+1;
			mp->eol_buffer.buf=duplicate_mem(&(buf[size]),mp->eol_buffer.len);
			}
		break;
		}
	}

/* 2.If something remains, write it */

if (size) _write_level2(mp,buf,size,flags,t);
}

/*----------------------------------------------*/

static void _write_level2(LOGMANAGER *mp, const char *buf, apr_off_t size
	,unsigned int flags, TIMESTAMP t)
{
apr_off_t csize;

CHECK_MP();
CHECK_TIME(mp,t);

INCR_STAT_COUNT(write2);

if ((!buf) || (size==0) || (!IS_OPEN(mp))) return;

csize=C_HANDLER1(predict_size,size);
if (!csize) csize=size;

/*-- rotate/purge ? (before writing) */

if ((!(flags & LMGRW_CANNOT_ROTATE)) && SHOULD_ROTATE(mp,csize))
	{
	logmanager_rotate(mp,t); /* includes a purge */
	}
else _purge_backup_files(mp,csize);

/*-- Write data */

C_HANDLER2(compress_and_write,buf,size);

/*-- Update end timestamp */

mp->active.file->end=t;
}

/*----------------------------------------------*/
/* NB: the order of backup files must be preserved from file to mem */

static void _get_status_from_file(LOGMANAGER *mp)
{
char *buf,*p,*p2,*val;
apr_off_t bufsize;
LOGFILE *lp;

DEBUG1(mp,1,"Reading status from file (%s)",mp->status_path);

lp=(LOGFILE *)0; /* Just to remove a warning at compile time */

mp->active.fp=(OFILE *)0;
mp->active.file=(LOGFILE *)0;
mp->backup.files=(LOGFILE **)0;
mp->backup.count=0;
mp->backup.size=0;

if (!file_exists(mp->status_path)) return;

buf=file_get_contents(mp->status_path,&bufsize);

p=buf;
while ((p2=strchr(p,'\n'))!=NULL)
	{
	(*p2)='\0';
	val=p+2;
	switch (*p)
		{
		case 'a':
			lp=NEW_LOGFILE();
			lp->path=duplicate(val);
			mp->active.file=lp;
			break;

		case 'b':
			lp=NEW_LOGFILE();
			lp->path=duplicate(val);
			mp->backup.files=allocate(mp->backup.files
				,(++mp->backup.count)*sizeof(LOGFILE *));
			mp->backup.files[mp->backup.count-1]=lp;
			break;

		case 'L':
			if (!lp) break;	/* Security against invalid file */
			lp->link=duplicate(val);
			break;

		case 'C':
			if (strcmp(val,compression_name(mp->compress.handler)))
				FATAL_ERROR2("Cannot continue from another compression engine (previous: %s ; new: %s)"
					,val,compression_name(mp->compress.handler));
			break;

		case 's':
			if (!lp) break;	/* Security against invalid file */
			lp->start=strval_to_time(val);
			break;

		case 'e':
			if (!lp) break;	/* Security against invalid file */
			lp->end=strval_to_time(val);
			break;
		/* Ignore other values */
		}
	p=p2+1;
	}

(void)allocate(buf,0);

_sync_logfiles_from_disk(mp);
}

/*----------------------------------------------*/
/* Dump info about active and backup log files to a status file.
* Sizes are not dumped as they will be retrieved from actual file size.
*/

#define DUMP_FILE(_lp,_type)	{ \
	if (_lp) \
		{ \
		file_write_string(fp,_type " ");	/* Path */ \
		file_write_string_nl(fp,(_lp)->path); \
		file_write_string(fp,"s ");			/* Start */ \
		(void)snprintf(buf,sizeof(buf),"%lu",(_lp)->start); \
		file_write_string_nl(fp,buf); \
		file_write_string(fp,"e ");			/* End */ \
		(void)snprintf(buf,sizeof(buf),"%lu",(_lp)->end); \
		file_write_string_nl(fp,buf); \
		if ((_lp)->link) \
			{ \
			file_write_string(fp,"L ");	/* Link */ \
			file_write_string_nl(fp,(_lp)->link); \
			} \
		} \
	}

static void _dump_status_to_file(LOGMANAGER *mp, TIMESTAMP t)
{
OFILE *fp;
char buf[32];
int i;

DEBUG1(mp,1,"Writing status to file (%s)",mp->status_path);
INCR_STAT_COUNT(dump);

fp=file_create(mp->status_path,(apr_int32_t)STATUSFILE_MODE);

file_write_string_nl(fp,"I === Managelogs status data ===");

file_write_string(fp,"A ");
(void)snprintf(buf,sizeof(buf),"%d",mp->api_version);
file_write_string_nl(fp,buf);

file_write_string_nl(fp,"V " LOGMANAGER_VERSION);

file_write_string(fp,"D ");
(void)snprintf(buf,sizeof(buf),"%lu",t);
file_write_string_nl(fp,buf);

file_write_string(fp,"C "); /* Compression type */
file_write_string_nl(fp,compression_name(mp->compress.handler));

DUMP_FILE(mp->active.file,"a");

if (mp->backup.count)
	{
	for (i=0;i<mp->backup.count;i++) DUMP_FILE(mp->backup.files[i],"b");
	}

(void)file_close(fp);
}

/*----------------------------------------------*/
/* Purges the structure from non-existing files and (re)computes the
* individual and global sizes.
* NB: Don't check active file if it is open.
*/

static void _sync_logfiles_from_disk(LOGMANAGER *mp)
{
int i,offset;
LOGFILE **lpp;

DEBUG(mp,1,"Syncing log files from disk");
INCR_STAT_COUNT(sync);

if (! IS_OPEN(mp)) SYNC_LOGFILE_FROM_DISK(mp->active.file);

mp->backup.size=0;

if (mp->backup.count)
	{
	for (i=0,offset=0,lpp=mp->backup.files;i<mp->backup.count;i++,lpp++)
		{
		SYNC_LOGFILE_FROM_DISK(*lpp);
		if (*lpp)
			{
			mp->backup.size += (*lpp)->size;
			if (offset) lpp[-offset]=(*lpp);
			}
		else offset++;
		}
	if (offset)
		{
		mp->backup.count -= offset;
		mp->backup.files=allocate(mp->backup.files
			,mp->backup.count*sizeof(LOGFILE *));
		}
	}
}

/*----------------------------------------------*/

char *logmanager_compression_list()
{
return compress_handler_list();
}

/*----------------------------------------------*/

char *logmanager_version()
{
return duplicate(LOGMANAGER_VERSION);
}

/*----------------------------------------------*/

#define DISPLAY_COUNT(_item)	{ \
	file_write_string(fp,#_item " count : "); \
	snprintf(buf,sizeof(buf),"%d",STAT_COUNT_ITEM(_item)); \
	file_write_string_nl(fp,buf); \
	}

void logmanager_display_stats(LOGMANAGER *mp)
{
OFILE *fp;
char buf[32];

fp=(mp->debug.fp ? mp->debug.fp : file_open_for_append("stdout",0));

file_write_string_nl(fp,"================== logmanager statistics ==================");

DISPLAY_COUNT(write);
DISPLAY_COUNT(write2);
DISPLAY_COUNT(flush);
DISPLAY_COUNT(link);
DISPLAY_COUNT(refresh_backup_links);
DISPLAY_COUNT(refresh_active_link);
DISPLAY_COUNT(new_active_file);
DISPLAY_COUNT(rotate);
DISPLAY_COUNT(remove_oldest);
DISPLAY_COUNT(dump);
DISPLAY_COUNT(sync);

file_write_string_nl(fp,"===========================================================");
}

/*----------------------------------------------*/
