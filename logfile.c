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
#include "logfile.h"
#include "util.h"

/*----------------------------------------------*/
/* Interrupt system - Delays signals until they can be handled, so that
  everything remain in a consistent state */

#define INTR_INIT()	{ \
			intr_init_done=YES; \
			NO_INTR_START(); \
			(void)apr_signal(SIGHUP,sighup_handler); \
			(void)apr_signal(SIGUSR1,sigusr1_handler); \
			(void)apr_signal(SIGTERM,sig_terminate_handler); \
			NO_INTR_END_DISCARD(); \
			}

#define INTR_SHUTDOWN()	{ \
			intr_init_done=NO; \
			(void)apr_signal(SIGHUP,SIG_IGN); \
			(void)apr_signal(SIGUSR1,SIG_IGN); \
			(void)apr_signal(SIGTERM,SIG_IGN); \
			RESET_PENDING_ACTIONS(); \
			}

#define NO_INTR_START()	{ \
			if (intr_init_done) intr_count++; \
			}

#define NO_INTR_END()	{ \
			if (intr_init_done) \
				{ \
				intr_count--; \
				CHECK_PENDING_ACTIONS(); \
				} \
			}

#define CHECK_PENDING_ACTIONS()	{ \
			if (intr_init_done && (intr_count==0)) \
				{ \
				NO_INTR_START(); \
				if (terminate_requested) \
					logfile_do_terminate(); \
				else if (rotate_requested) \
					logfile_do_rotate(); \
				else if (flush_requested) \
					logfile_do_flush(); \
				NO_INTR_END_DISCARD(); \
				} \
			}

#define NO_INTR_END_DISCARD()	{ \
			if (intr_init_done) \
				{ \
				intr_count--; \
				RESET_PENDING_ACTIONS(); \
				} \
			}

#define RESET_PENDING_ACTIONS()	{ \
			rotate_requested \
				=flush_requested \
				=terminate_requested \
				=NO; \
			}

/*----------------------------------------------*/

static char *rootpath,*logpath,*pidpath;
static apr_pool_t *pool;
static apr_off_t maxsize=0;
static int intr_count=0;
static BOOL rotate_requested=NO;
static BOOL flush_requested=NO;
static BOOL terminate_requested=NO;
/*@null@*/ static OFILE *logfp=(OFILE *)0;
static apr_fileperms_t logfile_mode;
static BOOL intr_init_done=NO;
static int keep_count;

/*----------------------------------------------*/

static void create_pid_file(void);
static void destroy_pid_file(void);
static void logfile_open(void);
static void logfile_close(void);
static void logfile_do_flush(void);
static void logfile_do_rotate(void);
static void logfile_do_terminate(void);
static void sighup_handler(int signum);
static void sigusr1_handler(int signum);
static void sig_terminate_handler(int signum);
static char *logfile_filename(int num);
static void rotate_file(int level, char *path);

/*----------------------------------------------*/

static void sighup_handler(/*@unused@*/ int signum)
{
static BOOL running=NO;

if (running) return;
running=YES;

rotate_requested=YES;
CHECK_PENDING_ACTIONS();

running=NO;
}

/*----------------------------------------------*/

static void sigusr1_handler(/*@unused@*/ int signum)
{
static BOOL running=NO;

if (running) return;
running=YES;

flush_requested=YES;
CHECK_PENDING_ACTIONS();

running=NO;
}

/*----------------------------------------------*/

static void sig_terminate_handler(/*@unused@*/ int signum)
{
static BOOL running=NO;

if (running) return;
running=YES;

terminate_requested=YES;
CHECK_PENDING_ACTIONS();

running=NO;
}

/*----------------------------------------------*/

static void logfile_do_terminate(void)
{
exit(0);
}

/*----------------------------------------------*/

static void create_pid_file()
{
OFILE *fp;
char buf[32];
size_t len;

NO_INTR_START();

pidpath=allocate(NULL,len=strlen(rootpath)+5);
(void)snprintf(pidpath,len,"%s.pid",rootpath);

fp=file_create(pidpath,(apr_int32_t)PIDFILE_MODE);

(void)snprintf(buf,sizeof(buf),"%lu",(unsigned long)getpid());
file_write_string_nl(fp, buf);

(void)file_close(fp);

NO_INTR_END();
}

/*----------------------------------------------*/

static void destroy_pid_file()
{
(void)file_delete(pidpath,NO);
pidpath=allocate(pidpath,0);
}

/*----------------------------------------------*/

static void logfile_do_flush()
{
if (!logfp) return;

DEBUG1("Flushing logfile (%s)",logfp->path);

logfile_close();
logfile_open();
}
	
/*----------------------------------------------*/

void logfile_init(const char *path,apr_off_t maxsize_arg,apr_fileperms_t mode
	,int keep_count_arg)
{
DEBUG1("Entering logfile_init: path=%s",path);
DEBUG1("Entering logfile_init: maxsize=%lu",(unsigned long)maxsize_arg);
DEBUG1("Entering logfile_init: mode=0x%x",(unsigned int)mode);
DEBUG1("Entering logfile_init: keep=%d",keep_count_arg);

file_init();

logfile_mode=mode;
keep_count=keep_count_arg;
maxsize=maxsize_arg;

(void)apr_pool_create(&pool, NULL);

rootpath=duplicate(path);

logpath=logfile_filename(0);

create_pid_file();

logfile_open();

INTR_INIT();
}

/*----------------------------------------------*/

void logfile_shutdown()
{
INTR_SHUTDOWN();

logfile_close();

destroy_pid_file();

logpath=allocate(logpath,0);
rootpath=allocate(rootpath,0);
}

/*----------------------------------------------*/

static void logfile_open()
{
if (logfp) return;

NO_INTR_START();

logfp=file_open_for_append(logpath,logfile_mode);
C_HANDLER(start,(logfp));

NO_INTR_END();
}

/*----------------------------------------------*/

static void logfile_close()
{
if (!logfp) return;

NO_INTR_START();

C_HANDLER(end,());
logfp=file_close(logfp);

NO_INTR_END();
}

/*----------------------------------------------*/

static char *logfile_filename(int level)
{
char buf[32],*p;
int len;

(void)snprintf(buf,sizeof(buf),"%d",level);

len=strlen(rootpath)+strlen(buf)+2;
if (compress_handler->suffix) len += (strlen(compress_handler->suffix)+1);

p=allocate(NULL,len);

strcpy(p,rootpath);
if (level != 0)
	{
	strcat(p,".");
	strcat(p,buf);
	}
if (compress_handler->suffix != NULL)
	{
	strcat(p,".");
	strcat(p,compress_handler->suffix);
	}

return p;
}

/*----------------------------------------------*/

static void rotate_file(int level, char *path)
{
char *oldpath;

DEBUG2("Rotating logfile %s (level %d)",path,level);

if (file_exists(path))
	{
	if (level==(keep_count-1))
		{
		DEBUG1("Stopping rotation at level %d to respect keep count",level);
		(void)file_delete(path,YES);
		}
	else
		{
		rotate_file(level+1,oldpath=logfile_filename(level+1));
		(void)file_rename(path,oldpath,YES);
		oldpath=allocate(oldpath,0);
		}
	}
}

/*----------------------------------------------*/

static void logfile_do_rotate()
{
if (!logfp) return;

NO_INTR_START();

DEBUG("Starting logfile rotation");

logfile_close();

rotate_file(0,logpath);

logfile_open();

NO_INTR_END();
}

/*----------------------------------------------*/

void logfile_write(const char *buf, apr_size_t size, BOOL can_rotate)
{
apr_size_t csize;
BOOL do_rotate;

NO_INTR_START();

if ((size==0) || (!logfp)) return;

if (can_rotate)
	{
	do_rotate=NO;

	if (maxsize!=0)
		{
		csize=size;
		C_HANDLER(predict_size,(&csize));
		if (((apr_off_t)(logfp->size+csize)) > maxsize) do_rotate=YES;
		}

	/*-- Future rotation delay check will go here --*/

	if (do_rotate) logfile_do_rotate();
	}

C_HANDLER(compress_and_write,(buf,size));

NO_INTR_END();
}

/*----------------------------------------------*/
