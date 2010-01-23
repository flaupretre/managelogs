/*=============================================================================

Copyright F. Laupretre (francois@tekwire.net)

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

#include <apr_signal.h>

#include "config.h"
#include "compress.h"
#include "logmanager.h"
#include "time.h"
#include "util.h"

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

#define IS_OPEN(_mp) (_mp->active.fp)

#define NEW_LOGFILE() (LOGFILE *)allocate(NULL,sizeof(LOGFILE))

#define ACTIVE_SIZE(_mp) \
	(_mp->active.fp ? _mp->active.fp->size : _mp->active.file->size)

#define ROTATE_IF_NEEDED(_mp,_add,_t)	{ \
	if (_mp->file_maxsize && ((ACTIVE_SIZE(_mp)+_add) > _mp->file_maxsize)) \
		logmanager_rotate(_mp,_t); \
	} \

#define GLOBAL_SIZE_EXCEEDED(_mp,_add)	((_mp->global_maxsize) \
	&& (_mp->backup.nb) \
	&& ((ACTIVE_SIZE(_mp)+_add+_mp->backup.size) > _mp->global_maxsize))

#define CHECK_MP() { /* Security */ \
	if (!mp) FATAL_ERROR("Received null LOGMANAGER pointer"); \
	}

/*----------------------------------------------*/

static void _create_pid_file(LOGMANAGER *mp);
static void _remove_pid_file(LOGMANAGER *mp);
static void _open_active_file(LOGMANAGER *mp);
static void _close_active_file(LOGMANAGER *mp);
static void _new_active_file(LOGMANAGER *mp,TIMESTAMP t);
static void _purge_backup_files(LOGMANAGER *mp,apr_off_t add);
static void _pack_backup_files(LOGMANAGER *mp);
static void _remove_backup(LOGMANAGER *mp,int num);
static LOGFILE *_remove_logfile(LOGFILE *lp,BOOL fatal);
static void _remove_oldest_backup(LOGMANAGER *mp);
static void _get_status_from_file(LOGMANAGER *mp);
static void _dump_status_to_file(LOGMANAGER *mp);
static void _refresh_logfiles(LOGMANAGER *mp);

/*----------------------------------------------*/

static void _create_pid_file(LOGMANAGER *mp)
{
OFILE *fp;
char buf[32];

fp=file_create(mp->pid_path,(apr_int32_t)PIDFILE_MODE);

(void)snprintf(buf,sizeof(buf),"%lu",(unsigned long)getpid());
file_write_string_nl(fp, buf);

(void)file_close(fp);
}

/*----------------------------------------------*/

static void _remove_pid_file(LOGMANAGER *mp)
{
(void)file_delete(mp->pid_path,NO);
}

/*----------------------------------------------*/

void logmanager_flush(LOGMANAGER *mp)
{
CHECK_MP();

if (!IS_OPEN(mp)) return;

DEBUG1("Flushing %s",mp->active.file->path);

C_HANDLER(flush);
}
	
/*----------------------------------------------*/
/*
* NB: the pool element is set to null by allocate().
* NB: Status is read at creation time and written at close time (not destroy).
*/

LOGMANAGER *new_logmanager_v1(LOGMANAGER_OPTIONS_V1 *opts,TIMESTAMP t)
{
LOGMANAGER *mp;
int len;

mp=(LOGMANAGER *)allocate(NULL,sizeof(LOGMANAGER));

/*-- API version */

mp->api_version=1;

/*-- Root, PID, Status paths */

mp->root_path=duplicate(opts->root_path);

mp->pid_path=allocate(NULL,len=(strlen(mp->root_path)+5));
(void)snprintf(mp->pid_path,len,"%s.pid",mp->root_path);
_create_pid_file(mp);

mp->status_path=allocate(NULL,len=(strlen(mp->root_path)+8));
(void)snprintf(mp->status_path,len,"%s.status",mp->root_path);

/*-- Flags */

mp->flags=opts->flags;

/*-- Compress engine */

C_HANDLER1(init_v1,opts);

/*-- Populates mp->active and mp->backup */

_get_status_from_file(mp);
ROTATE_IF_NEEDED(mp,0,t);
_purge_backup_files(mp,0);

/*-- Max sizes and limits */
/* Ensures that file limit is lower than global limit. If global limit is set
* and file limit is not, file limit is set to global limit / 2 */

mp->file_maxsize=opts->file_maxsize;
mp->global_maxsize=opts->global_maxsize;

if (mp->global_maxsize)
	{
	if (mp->global_maxsize < LOWER_LIMIT)
		FATAL_ERROR1("Global limit cannot be less than %d",LOWER_LIMIT);
	if (! mp->file_maxsize) mp->file_maxsize=mp->global_maxsize/2;
	if (mp->global_maxsize < mp->file_maxsize)
		FATAL_ERROR("Global limit cannot be less than file limit");
	}

mp->keep_count=opts->keep_count;

/*-- File creation mode */

mp->create_mode=opts->create_mode;
if (!mp->create_mode) mp->create_mode=LOGFILE_MODE;

/*-- Create PID file */

_create_pid_file(mp);

/*--*/

return mp;
}

/*----------------------------------------------*/

void logmanager_destroy(LOGMANAGER *mp)
{
CHECK_MP();

/*-- First, close the current log if not already done */

if (IS_OPEN(mp)) logmanager_close(mp);

/*-- Remove the PID file */

_remove_pid_file(mp);

/*-- Destroy compress handler */

C_HANDLER(destroy);

/* Free paths */

(void)allocate(mp->root_path,0);
(void)allocate(mp->pid_path,0);
(void)allocate(mp->status_path,0);

/* Last, free the structure */

(void)allocate(mp,0);
}

/*----------------------------------------------*/

void logmanager_open(LOGMANAGER *mp,TIMESTAMP t)
{
if (IS_OPEN(mp)) return;

if (!mp->active.file) _new_active_file(mp,t);

_open_active_file(mp);
}

/*----------------------------------------------*/

static void _open_active_file(LOGMANAGER *mp)
{
if (IS_OPEN(mp)) return;

mp->active.fp=file_open_for_append(mp->active.file->path,mp->create_mode);

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

void logmanager_close(LOGMANAGER *mp)
{
CHECK_MP();

(void)_close_active_file(mp);

_dump_status_to_file(mp);
}

/*----------------------------------------------*/

static void _new_active_file(LOGMANAGER *mp,TIMESTAMP t)
{
LOGFILE *lp;
int len;

lp=mp->active.file=NEW_LOGFILE();

len=strlen(mp->root_path)+12;
if (mp->compress.handler->suffix) len+=(strlen(mp->compress.handler->suffix)+1);

lp->path=allocate(NULL,len);

if (mp->compress.handler->suffix)
	{
	(void)snprintf(lp->path,len,"%s.%010lu.%s",mp->root_path,t
		,mp->compress.handler->suffix);
	}
else
	{
	(void)snprintf(lp->path,len,"%s.%010lu",mp->root_path,t);
	}

lp->start=lp->end=t;
}

/*----------------------------------------------*/

void logmanager_rotate(LOGMANAGER *mp,TIMESTAMP t)
{
CHECK_MP();

DEBUG1("Starting rotation (%s)",mp->root_path);

if (IS_OPEN(mp)) _close_active_file(mp);

mp->backup.size += ACTIVE_SIZE(mp);
mp->backup.files=(LOGFILE **)allocate(mp->backup.files
	,(mp->backup.nb+1)*sizeof(LOGFILE *));
mp->backup.files[mp->backup.nb]=mp->active.file;
mp->backup.nb++;
mp->active.file=(LOGFILE *)0;

_purge_backup_files(mp,0);

_new_active_file(mp,t);
_open_active_file(mp);

_dump_status_to_file(mp);
}

/*----------------------------------------------*/

static void _purge_backup_files(LOGMANAGER *mp,apr_off_t add)
{
_refresh_logfiles(mp);

while (GLOBAL_SIZE_EXCEEDED(mp,add)
	|| ((mp->keep_count) && (mp->backup.nb > (mp->keep_count - 1))))
	{
	_remove_oldest_backup(mp);
	}
}

/*----------------------------------------------*/
/* Remove NULL entries and recompute global size */

static void _pack_backup_files(LOGMANAGER *mp)
{
LOGFILE **oldp,**newp;
int i,j,offset;
apr_off_t gsize;

oldp=mp->backup.files;
newp=(LOGFILE **)0;
gsize=0;

if (mp->backup.nb)
	{
	for (i=j=offset=0;i<mp->backup.nb;i++)
		{
		if (oldp[i])
			{
			newp=allocate(newp,(i-offset+1)*sizeof(LOGFILE *));
			newp[i-offset]=oldp[i];
			gsize += oldp[i]->size;
			}
		else offset++;
		}
	}

(void)allocate(oldp,0);
mp->backup.files=newp;
mp->backup.size=gsize;
}

/*----------------------------------------------*/

static void _remove_backup(LOGMANAGER *mp, int num)
{
mp->backup.files[num]=_remove_logfile(mp->backup.files[num],NO);

_pack_backup_files(mp);
}

/*----------------------------------------------*/

static void _remove_oldest_backup(LOGMANAGER *mp)
{
int i,num;
TIMESTAMP end;

if (! mp->backup.nb) return; /* Should never happen */

/*-- Find oldest backup */

end=mp->backup.files[num=0]->end;

for (i=0;i<mp->backup.nb;i++)
	{
	if (mp->backup.files[i]->end < end)
		{
		num=i;
		end=mp->backup.files[i]->end;
		}
	}

/*-- Now, remove it */

_remove_backup(mp,num);
}

/*----------------------------------------------*/

void logmanager_write(LOGMANAGER *mp, const char *buf, apr_off_t size
	,unsigned int flags, TIMESTAMP t)
{
apr_off_t csize;

CHECK_MP();

if ((size==0) || (!IS_OPEN(mp))) return;

csize=C_HANDLER1(predict_size,size);
if (!csize) csize=size;

/*-- rotate/purge ? (before writing) */

if (! (flags & CANNOT_ROTATE)) ROTATE_IF_NEEDED(mp,csize,t);
_purge_backup_files(mp,csize);

/*-- Write data */

C_HANDLER2(compress_and_write,buf,size);

/*-- Update end timestamp */

mp->active.file->end=t;
}

/*----------------------------------------------*/

static void _get_status_from_file(LOGMANAGER *mp)
{
char *buf,*p,*p2,*val;
apr_off_t bufsize;
LOGFILE *lp;

lp=(LOGFILE *)0; /* Just to remove warning at compile time */

mp->active.fp=(OFILE *)0;
mp->active.file=(LOGFILE *)0;
mp->backup.files=(LOGFILE **)0;
mp->backup.nb=0;
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
			lp=mp->active.file=NEW_LOGFILE();
			lp->path=duplicate(val);
			break;

		case 'b':
			lp=mp->active.file=NEW_LOGFILE();
			lp->path=duplicate(val);
			break;

		case 's':
			if (!lp) break;	/* Security against invalid file */
			lp->start=strval_to_time(val);
			break;

		case 'e':
			if (!lp) break;	/* Security against invalid file */
			lp->end=strval_to_time(val);
			break;

		case 'S':;
			if (!lp) break;	/* Security against invalid file */
			lp->size=convert_size_string(val);
			break;
		/* Ignore other values */
		}
	p=p2+1;
	}

(void)allocate(buf,0);
}

/*----------------------------------------------*/

#define DUMP_FILE(_lp,_type)	{ \
	if (_lp) \
		{ \
		file_write_string(fp,_type " ");	/* Path */ \
		file_write_string_nl(fp,_lp->path); \
		file_write_string(fp,"s ");			/* Start */ \
		(void)snprintf(buf,sizeof(buf),"%lu",_lp->start); \
		file_write_string_nl(fp,buf); \
		file_write_string(fp,"e ");			/* End */ \
		(void)snprintf(buf,sizeof(buf),"%lu",_lp->end); \
		file_write_string_nl(fp,buf); \
		file_write_string(fp,"S ");			/* Size */ \
		(void)snprintf(buf,sizeof(buf),"%lu",_lp->size); \
		file_write_string_nl(fp,buf); \
		} \
	}

static void _dump_status_to_file(LOGMANAGER *mp)
{
OFILE *fp;
char buf[32];
int i;

fp=file_create(mp->status_path,(apr_int32_t)STATUSFILE_MODE);

file_write_string_nl(fp,"V " MANAGELOGS_VERSION);

file_write_string(fp,"A ");
(void)snprintf(buf,sizeof(buf),"%d",mp->api_version);
file_write_string_nl(fp,buf);

file_write_string(fp,"D ");
(void)snprintf(buf,sizeof(buf),"%lu",time_now());
file_write_string_nl(fp,buf);

DUMP_FILE(mp->active.file,"a");

if (mp->backup.nb)
	{
	for (i=0;i<mp->backup.nb;i++) DUMP_FILE(mp->backup.files[i],"b");
	}

(void)file_close(fp);
}

/*----------------------------------------------*/

static LOGFILE *_remove_logfile(LOGFILE *lp,BOOL fatal)
{
if (file_exists(lp->path)) file_delete(lp->path,fatal);

(void)allocate(lp,0);

return (LOGFILE *)0;
}

/*----------------------------------------------*/
/* Purges the backup array from non-existing files and (re)computes the 
* individual and global sizes.
* NB: Don't check open active file.
*/

static void _refresh_logfiles(LOGMANAGER *mp)
{
apr_off_t gsize;
int i;

gsize=0;

if ((!mp->active.fp) && (!file_exists(mp->active.file->path)))
	mp->active.file=_remove_logfile(mp->active.file,NO);

if (mp->backup.nb)
	{
	for (i=0;i<mp->backup.nb;i++)
		{
		if (!file_exists(mp->backup.files[i]->path))
			mp->backup.files[i]=_remove_logfile(mp->backup.files[i],NO);
		}
	}

_pack_backup_files(mp);
}

/*----------------------------------------------*/
