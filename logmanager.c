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

#define IS_OPEN() (mp->active.fp)

/*----------------------------------------------*/

static void _create_pid_file(LOGMANAGER *mp);
static void _remove_pid_file(LOGMANAGER *mp);
static void _open_active_file(LOGMANAGER *mp);
static apr_off_t _close_active_file(LOGMANAGER *mp);
static void _new_active_file(LOGMANAGER *mp,TIMESTAMP t);
static void _purge_backups(LOGMANAGER *mp,apr_size_t add);
static void _remove_backup(LOGMANAGER *mp,int num);
static void _remove_oldest_backup(LOGMANAGER *mp);
static void _get_status_from_file(LOGMANAGER *mp);
static void _dump_status_to_file(LOGMANAGER *mp);
static void _refresh_backups(LOGMANAGER *mp);

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
if (!IS_OPEN()) return;

DEBUG1("Flushing %s",mp->active.file->path);

C_HANDLER(flush);
}
	
/*----------------------------------------------*/
/*
* NB: the pool element is set to null by the initial memset() call.
* NB: Status is read at creation time and written at close time (not destroy).
*/

LOGMANAGER *new_logmanager_v1(LOGMANAGER_OPTIONS_V1 *opts,TIMESTAMP t)
{
LOGMANAGER *mp;
int len;

mp=(LOGMANAGER *)allocate(NULL,sizeof(LOGMANAGER));
memset(mp,0,sizeof(*mp));

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

/*-- Populates mp->active and mp->backups */

_get_status_from_file(mp);

/*-- Max sizes and limits */

mp->file_maxsize=opts->file_maxsize;
mp->global_maxsize=opts->global_maxsize;
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
/*-- First, close the current log if not already done */

if (IS_OPEN()) logmanager_close(mp);

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
if (IS_OPEN()) return;

_new_active_file(mp,t);

_open_active_file(mp);
}

/*----------------------------------------------*/

static void _open_active_file(LOGMANAGER *mp)
{
if (IS_OPEN()) return;

mp->active.fp=file_open_for_append(mp->active.file->path,mp->create_mode);

C_HANDLER(start);
}

/*----------------------------------------------*/

static apr_off_t _close_active_file(LOGMANAGER *mp)
{
apr_off_t size;

if (!IS_OPEN()) return (apr_off_t)0;

C_HANDLER(end);
size=mp->active.fp->size;

mp->active.fp=file_close(mp->active.fp);

return size;
}

/*----------------------------------------------*/

void logmanager_close(LOGMANAGER *mp)
{
(void)_close_active_file(mp);

_dump_status_to_file(mp);
}

/*----------------------------------------------*/

static LOGFILE *_new_logfile()
{
LOGFILE *lp;

lp=(LOGFILE *)allocate(NULL,sizeof(LOGFILE));
memset(lp,0,sizeof(LOGFILE));

return lp;
}

/*----------------------------------------------*/

static void _new_active_file(LOGMANAGER *mp,TIMESTAMP t)
{
LOGFILE *lp;
int len;

lp=_new_logfile();

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

mp->active.file=lp;
}

/*----------------------------------------------*/

void logmanager_rotate(LOGMANAGER *mp,TIMESTAMP t)
{
if (!IS_OPEN()) return;

DEBUG1("Starting rotation (%s)",mp->root_path);

mp->backups.size += _close_active_file(mp);
mp->backups.files=(LOGFILE **)allocate(mp->backups.files
	,(mp->backups.nb+1)*sizeof(LOGFILE *));
mp->backups.files[mp->backups.nb]=mp->active.file;
mp->backups.nb++;
mp->active.file=(LOGFILE *)0;

_purge_backups(mp,0);

_new_active_file(mp,t);
_open_active_file(mp);

_dump_status_to_file(mp);
}

/*----------------------------------------------*/
/* C optimizer should have fun here :) */

static void _purge_backups(LOGMANAGER *mp,apr_size_t add)
{
_refresh_backups(mp);

while (((mp->global_maxsize)
		&& (mp->backups.nb)
		&& ((mp->active.fp->size+add+mp->backups.size) > mp->global_maxsize))
	|| ((mp->keep_count) && (mp->backups.nb > (mp->keep_count - 1))))
	{
	_remove_oldest_backup(mp);
	}
}

/*----------------------------------------------*/

static void _remove_backup(LOGMANAGER *mp, int num)
{
int i,j;
LOGFILE **p;

file_delete(mp->backups.files[num]->path,YES);

p=(LOGFILE **)allocate(NULL,(mp->backups.nb-1)*sizeof(LOGFILE *));
if (p)
	{
	for (i=j=0;i<mp->backups.nb;i++)
		{
		if (i==num) continue;
		p[(j++)-((i>num) ? 1 : 0)]=mp->backups.files[i];
		}
	}
(void)allocate(mp->backups.files,0);
mp->backups.files=p;
}

/*----------------------------------------------*/

static void _remove_oldest_backup(LOGMANAGER *mp)
{
int i,num;
TIMESTAMP end;

if (! mp->backups.nb) return; /* Should never happen */

/*-- Find oldest backup */

end=mp->backups.files[num=0]->end;

for (i=0;i<mp->backups.nb;i++)
	{
	if (mp->backups.files[i]->end < end)
		{
		num=i;
		end=mp->backups.files[i]->end;
		}
	}

/*-- Now, remove it */

mp->backups.size -= file_size(mp->backups.files[num]->path);

_remove_backup(mp,num);
}

/*----------------------------------------------*/

void logmanager_write(LOGMANAGER *mp, const char *buf, apr_size_t size
	,unsigned int flags, TIMESTAMP t)
{
apr_size_t csize;

if ((size==0) || (!IS_OPEN())) return;

csize=C_HANDLER1(predict_size,size);
if (!csize) csize=size;

/*-- rotate ? */

if (! (flags & NO_ROTATE))
	{
	if (mp->file_maxsize && ((mp->active.fp->size + csize) > mp->file_maxsize))
		logmanager_rotate(mp,t);
	}

/*-- Purge before write */

_purge_backups(mp,csize);

/*-- Write data */

C_HANDLER2(compress_and_write,buf,size);
mp->active.file->end=t;
}

/*----------------------------------------------*/

static void _get_status_from_file(LOGMANAGER *mp)
{
/*TODO*/

_refresh_backups(mp);
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
		} \
	}

static void _dump_status_to_file(LOGMANAGER *mp)
{
OFILE *fp;
char buf[32];
int i;

fp=file_create(mp->status_path,(apr_int32_t)STATUSFILE_MODE);

DUMP_FILE(mp->active.file,"a");

if (mp->backups.nb)
	{
	for (i=0;i<mp->backups.nb;i++) DUMP_FILE(mp->backups.files[i],"b");
	}

(void)file_close(fp);
}

/*----------------------------------------------*/
/* Purges the backup array from non-existing files and (re)computes the 
* global size.
*/

static void _refresh_backups(LOGMANAGER *mp)
{
apr_off_t gsize;
int i;
char *path;

gsize=0;

if (mp->backups.nb)
	{
	for (i=0;i<mp->backups.nb;)
		{
		path=mp->backups.files[i]->path;
		if (!file_exists(path))
			{
			_remove_backup(mp,i);
			}
		else
			{
			gsize += file_size(path);
			}
		}
	}

mp->backups.size=gsize;
}

/*----------------------------------------------*/
