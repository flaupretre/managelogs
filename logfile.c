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

#include <apr_file_io.h>
#include <apr_file_info.h>

#include "config.h"
#include "compress.h"
#include "logfile.h"
#include "error.h"
#include "util.h"

/*----------------------------------------------*/

#define NO_INTR_START()			{ \
								intr_flag++; \
								}

#define NO_INTR_END()			{ \
								if (!(--intr_flag)) \
									{ \
									if (rotate_requested) logfile_do_rotate(); \
									else \
										{ \
										if (flush_requested) logfile_do_flush(); \
										} \
									} \
								}

#define NO_INTR_END_DISCARD()	{ \
								intr_flag--; \
								rotate_requested=flush_requested=0; \
								}

/*----------------------------------------------*/

static apr_file_t *logfd = NULL;
static char rootpath[MAX_PATH],logpath[MAX_PATH],oldpath[MAX_PATH]
	,pidpath[MAX_PATH];
static apr_off_t logsize;
static apr_pool_t *pool;
static int logfile_is_open=0;
static apr_off_t maxsize,limit;
static int intr_flag=0;
static int rotate_requested=0;
static int flush_requested=0;

/*----------------------------------------------*/

static void create_pid_file(void);
static void destroy_pid_file(void);
static void logfile_open(void);
static void logfile_close(void);
static void logfile_do_flush(void);
static void logfile_do_flush(void);
static void logfile_do_rotate(void);
static int limit_exceeded(apr_size_t size);

/*----------------------------------------------*/

static void create_pid_file()
{
static apr_file_t *fd;
char buf[32];
apr_size_t nb;

sprintf(pidpath,"%s.pid",rootpath);
fd=NULL;
apr_file_open(&fd,pidpath,APR_WRITE|APR_CREATE|APR_TRUNCATE,APR_OS_DEFAULT,pool);
if (!fd) FATAL_ERROR_1("Cannot open pid file (%s)",pidpath);

sprintf(buf,"%lu\n",(unsigned long)getpid());
nb=(apr_size_t)strlen(buf);
apr_file_write(fd, buf, &nb);
apr_file_close(fd);
}

/*----------------------------------------------*/

static void destroy_pid_file()
{
(void)apr_file_remove(pidpath,pool);
}

/*----------------------------------------------*/

void logfile_flush()
{
if (intr_flag)
	{
	flush_requested=1;
	return;
	}
else logfile_do_flush();
}

/*----------------------------------------------*/

static void logfile_do_flush()
{
logfile_close();
logfile_open();

flush_requested=0;
}
	
/*----------------------------------------------*/

void logfile_init(const char *path,apr_off_t maxsize_arg)
{
NO_INTR_START();

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

NO_INTR_END();
}

/*----------------------------------------------*/

void logfile_shutdown()
{
NO_INTR_START();

logfile_close();

destroy_pid_file();

NO_INTR_END_DISCARD();
}

/*----------------------------------------------*/

static void logfile_open()
{
apr_finfo_t finfo;

if (logfile_is_open) return;

NO_INTR_START();

C_HANDLER(start,());

apr_file_open(&logfd,logpath,APR_WRITE|APR_CREATE|APR_APPEND
	,APR_OS_DEFAULT,pool);
if (!logfd) FATAL_ERROR_1("Cannot open log file (%s)\n",logpath);

if (apr_file_info_get(&finfo,APR_FINFO_SIZE,logfd)!=APR_SUCCESS)
	FATAL_ERROR_1("Cannot get logfile size (%s)\n",logpath);

logsize=finfo.size;
logfile_is_open=1;

NO_INTR_END();
}

/*----------------------------------------------*/

static void logfile_close()
{
if (!logfile_is_open) return;

NO_INTR_START();

C_HANDLER(end,());

apr_file_close(logfd);

logfd=NULL;
logfile_is_open=0;

NO_INTR_END();
}

/*----------------------------------------------*/

void logfile_rotate()
{
if (intr_flag)
	{
	rotate_requested=1;
	return;
	}
else logfile_do_rotate();
}


/*----------------------------------------------*/

static void logfile_do_rotate()
{
apr_finfo_t finfo;

if (!logfile_is_open) return;

logfile_close();

/* Remove old log file if it exists */

if ((apr_stat(&finfo,oldpath,0,pool)==APR_SUCCESS)
	&& (apr_file_remove(oldpath,pool)!=APR_SUCCESS))
		FATAL_ERROR_1("Cannot rotate %s (cannot remove old logfile)"
			,logpath);

/* then, rename log file to old */

if (apr_file_rename(logpath,oldpath,pool)!=APR_SUCCESS)
	FATAL_ERROR_1("Cannot rotate %s (cannot rename logfile)",logpath);

/* Create new empty log file and reset cursize */

logfile_open();

rotate_requested=0;
}

/*----------------------------------------------*/

apr_off_t logfile_size()
{
return logsize;
}

/*----------------------------------------------*/

void logfile_write_bin_raw(const char *buf, apr_size_t size)
{
apr_size_t nwrite;

if (!size) return;

NO_INTR_START();

nwrite=size;
apr_file_write(logfd, buf, &nwrite);
if (nwrite!=size) FATAL_ERROR_1("Cannot write to log file (%s)\n", logpath);

logsize += size;

NO_INTR_END();
}

/*----------------------------------------------*/

static int limit_exceeded(apr_size_t size)
{
if (!limit) return 0;

C_HANDLER(predict_size,(&size));

return ((logsize+size) > limit);
}

/*----------------------------------------------*/

void logfile_write_bin(const char *buf, apr_size_t size, rotate_flag can_rotate)
{
if (!size) return;

if (can_rotate && limit_exceeded(size)) logfile_do_rotate();

C_HANDLER(compress_and_write,(buf,size));
}

/*----------------------------------------------*/

void logfile_write(const char *str)
{
apr_size_t len;

len=(apr_size_t)strlen(str);

NO_INTR_START();

logfile_write_bin(str,len,CAN_ROTATE);
logfile_write_bin("\n",1,CANNOT_ROTATE);

NO_INTR_END();
}

/*----------------------------------------------*/
