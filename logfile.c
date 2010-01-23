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
			RESET_PENDING_ACTIONS(); \
			(void)apr_signal(SIGHUP,sighup_handler); \
			(void)apr_signal(SIGUSR1,sigusr1_handler); \
			(void)apr_signal(SIGTERM,sig_terminate_handler); \
			intr_init_done=1; \
			}

#define INTR_SHUTDOWN()	{ \
			intr_init_done=0; \
			(void)apr_signal(SIGHUP,SIG_IGN); \
			(void)apr_signal(SIGUSR1,SIG_IGN); \
			(void)apr_signal(SIGTERM,SIG_IGN); \
			RESET_PENDING_ACTIONS(); \
			}

#define NO_INTR_START()	{ \
			if (intr_init_done) intr_flag++; \
			}

#define NO_INTR_END()	{ \
			if (intr_init_done) \
				{ \
				intr_flag--; \
				CHECK_PENDING_ACTIONS(); \
				} \
			}

#define CHECK_PENDING_ACTIONS()	{ \
			if (intr_init_done && (!intr_flag)) \
				{ \
				if (terminate_requested) \
					logfile_do_terminate(); \
				else if (rotate_requested) \
					logfile_do_rotate(); \
				else if (flush_requested) \
					logfile_do_flush(); \
				RESET_PENDING_ACTIONS(); \
				} \
			}

#define NO_INTR_END_DISCARD()	{ \
			if (intr_init_done) \
				{ \
				intr_flag--; \
				RESET_PENDING_ACTIONS(); \
				} \
			}

#define RESET_PENDING_ACTIONS()	{ \
			rotate_requested \
				=flush_requested \
				=terminate_requested \
				=0; \
			}

/*----------------------------------------------*/

static char rootpath[MAX_PATH],logpath[MAX_PATH],oldpath[MAX_PATH]
	,pidpath[MAX_PATH];
static apr_pool_t *pool;
static apr_off_t maxsize,limit;
static int intr_flag=0;
static int rotate_requested;
static int flush_requested;
static int terminate_requested;
static OFILE *logfp=(OFILE *)0;
static apr_fileperms_t logfile_mode;
static int intr_init_done=0;

/*----------------------------------------------*/

static void create_pid_file(void);
static void destroy_pid_file(void);
static void logfile_open(void);
static void logfile_close(void);
static void logfile_do_flush(void);
static void logfile_do_rotate(void);
static void logfile_do_terminate(void);
static int limit_exceeded(apr_size_t size);
static void sighup_handler(int signum);
static void sigusr1_handler(int signum);
static void sig_terminate_handler(int signum);

/*----------------------------------------------*/

static void sighup_handler(int signum)
{
static int running=0;

if (running) return;
running=1;

rotate_requested=1;
CHECK_PENDING_ACTIONS();

running=0;
}

/*----------------------------------------------*/

static void sigusr1_handler(int signum)
{
static int running=0;

if (running) return;
running=1;

flush_requested=1;
CHECK_PENDING_ACTIONS();

running=0;
}

/*----------------------------------------------*/

static void sig_terminate_handler(int signum)
{
static int running=0;

if (running) return;
running=1;

terminate_requested=1;
CHECK_PENDING_ACTIONS();

running=0;
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

sprintf(pidpath,"%s.pid",rootpath);

NO_INTR_START();

fp=file_create(pidpath,(apr_int32_t)0x0644);

sprintf(buf,"%lu",(unsigned long)getpid());
file_write_string_nl(fp, buf);

file_close(fp);

NO_INTR_END();
}

/*----------------------------------------------*/

static void destroy_pid_file()
{
(void)file_delete(pidpath);
}

/*----------------------------------------------*/

static void logfile_do_flush()
{
logfile_close();
logfile_open();
}
	
/*----------------------------------------------*/

void logfile_init(const char *path,apr_off_t maxsize_arg,apr_fileperms_t mode)
{
DEBUG1("Entering logfile_init: path=%s",path);
DEBUG1("Entering logfile_init: maxsize=%lu",maxsize_arg);
DEBUG1("Entering logfile_init: mode=0x%x",mode);

file_init();

logfile_mode=mode;

apr_pool_create(&pool, NULL);

maxsize=maxsize_arg;
limit=maxsize/2;

if (strlen(path) >= BUFSIZE-18) FATAL_ERROR("Path too long");
strcpy(rootpath,path);

strcpy(logpath,rootpath);
sprintf(oldpath,"%s.old",rootpath);
if (compress_handler->suffix)
	{
	strcat(logpath,".");
	strcat(logpath,compress_handler->suffix);
	strcat(oldpath,".");
	strcat(oldpath,compress_handler->suffix);
	}

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

static void logfile_do_rotate()
{
if (!logfp) return;

NO_INTR_START();

logfile_close();

/* Remove old log file if it exists */

if (file_exists(oldpath) && (!file_delete(oldpath)))
	FATAL_ERROR_1("Cannot rotate %s (cannot remove old logfile)",logfp->path);

/* then, rename log file to old */

if (!file_rename(logpath,oldpath))
	FATAL_ERROR_1("Cannot rotate %s (cannot rename logfile)",logfp->path);

/* Create new empty log file and reset cursize */

logfile_open();

NO_INTR_END();
}

/*----------------------------------------------*/

static int limit_exceeded(apr_size_t size)
{
if (!limit) return 0;

C_HANDLER(predict_size,(&size));

return ((logfp->size+size) > limit);
}

/*----------------------------------------------*/

void logfile_write(const char *buf, apr_size_t size, rotate_flag can_rotate)
{
if (!size) return;

NO_INTR_START();

if (can_rotate && limit_exceeded(size)) logfile_do_rotate();

C_HANDLER(compress_and_write,(buf,size));

NO_INTR_END();
}

/*----------------------------------------------*/
