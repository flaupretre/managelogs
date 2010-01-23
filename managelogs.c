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
#include <apr_getopt.h>
#include <apr_signal.h>

#include "logfile.h"
#include "compress.h"
#include "util.h"
#include "intr.h"
#include "config.h"

/*----------------------------------------------*/

#define FLUSH_ACTION		1
#define ROTATE_ACTION		2
#define TERMINATE_ACTION 	3

#define CHECK_EXEC_PENDING_ACTION()	{ \
	ACTION action; \
	if ((action=check_pending_action())!=NO_ACTION) do_action(action); \
	}

/*----------------------------------------------*/

static char *cmd;
PRIVATE_POOL

static apr_getopt_t *opt_s;
static apr_getopt_option_t long_options[]=
	{
	{"help",'h',0 },
	{"debug",'d',0 },
	{"compress",'c',1 },
	{"size",'s',1 },
	{"mode",'m',1 },
	{"user",'u',1 },
	{"keep",'k',1 },
	{"version",'V',0 },
	{"",'\0', 0 }
	};

/*----------------------------------------------*/

static void usage(int rc);
static void shutdown_proc(void);
static void do_action(unsigned int action);

/*----------------------------------------------*/

static void do_action(unsigned int action)
{
switch(action)
	{
	case FLUSH_ACTION:
		logmanager_flush(mp);
		break;

	case ROTATE_ACTION:
		logmanager_rotate(mp);
		break;

	case TERMINATE_ACTION:
		exit(0);
		break;
	}
}

/*----------------------------------------------*/

static void usage(int rc)
{
FILE *fd;
char *clist;

fd=((rc>0) ? stderr : stdout);
clist=compress_handler_list();

fprintf(fd,"\
managelogs %s\n\
\nUsage: %s [options...] <path>\n",MANAGELOGS_VERSION,cmd);

fprintf(fd,"\
\n\
Options :\n\
\n\
 -h|--help           Display this message\n\
\n\
 -d|--debug          Display debug messages to stdout\n\
 \n\
 -c|--compress <comp>[:<level>]  Activate compression and appends the\n\
                     corresponding suffix to the log file names.\n\
                        <comp> is one of : %s\n\
                        <level> is one of {0123456789bf} (f=fast, b=best)\n\
                        Default level depends on the compression engine\n\
\n\
 -s|--size <size>    Set the maximum size at which rotation occurs\n\
                        <size> is a numeric value optionnally followed\n\
                        by 'K' (Kilo), 'M' (Mega), or 'G' (Giga)\n\
                        Default: no limit\n\
\n\
 -k|--keep <n>       Keep only <n> log files (the current log file and <n-1>\n\
                     backups)\n\
\n\
 -m|--mode <mode>    File mode to use for newly-created log files\n\
                        <mode> is a numeric Unix-style file	permission\n\
                        (see chmod(2)). Default mode: %x\n\
\n\
 -u|--user <id>      Program runs with this user ID\n\
                        <id> = <uid>[:<gid>]\n\
                        <uid> and <gid> are user/group names or numeric ids\n\
\n\
 -V|--version        Print version and exit\n\
\n",clist,LOGFILE_MODE);

(void)allocate(clist,0);

if (rc >= 0) exit(rc);
}

/*----------------------------------------------*/

static void __signal_handler(int sig)
{
unsigned int action;

switch(sig)
	{
	case SIGUSR1:
		set_pending_action(FLUSH_ACTION);
		CHECK_EXEC_PENDING_ACTION();
		break;

	case SIGHUP:
		set_pending_action(ROTATE_ACTION);
		CHECK_EXEC_PENDING_ACTION();
		break;

#ifdef SIGHUP
		case SIGHUP:
#endif
#ifdef SIGUSR1
		case SIGUSR1:
#endif
#ifdef SIGTERM
		case SIGTERM:
#endif
#ifdef SIGINT
		case SIGINT:
#endif
#ifdef SIGQUIT
		case SIGQUIT:
#endif
#ifdef SIGTRAP
		case SIGTRAP:
#endif
#ifdef SIGABRT
		case SIGABRT:
#endif
#ifdef SIGURG
		case SIGURG:
#endif
		set_pending_action(TERMINATE_ACTION);
		CHECK_EXEC_PENDING_ACTION();
		break;
}

/*----------------------------------------------*/

static void _signal_init()
{
#ifdef SIGHUP
(void)apr_signal(SIGHUP,_signal_handler);
#endif
#ifdef SIGUSR1
(void)apr_signal(SIGUSR1,_signal_handler);
#endif
#ifdef SIGTERM
(void)apr_signal(SIGTERM,_signal_handler);
#endif
#ifdef SIGINT
(void)apr_signal(SIGINT,_signal_handler);
#endif
#ifdef SIGQUIT
(void)apr_signal(SIGQUIT,_signal_handler);
#endif
#ifdef SIGTRAP
(void)apr_signal(SIGTRAP,_signal_handler);
#endif
#ifdef SIGABRT
(void)apr_signal(SIGABRT,_signal_handler);
#endif
#ifdef SIGURG
(void)apr_signal(SIGURG,_signal_handler);
#endif
}

/*----------------------------------------------*/

void _signal_shutdown()
{
#ifdef SIGHUP
(void)apr_signal(SIGHUP,SIG_IGN);
#endif
#ifdef SIGUSR1
(void)apr_signal(SIGUSR1,SIG_IGN);
#endif
#ifdef SIGTERM
(void)apr_signal(SIGTERM,SIG_IGN);
#endif
#ifdef SIGINT
(void)apr_signal(SIGINT,SIG_IGN);
#endif
#ifdef SIGQUIT
(void)apr_signal(SIGQUIT,SIG_IGN);
#endif
#ifdef SIGTRAP
(void)apr_signal(SIGTRAP,SIG_IGN);
#endif
#ifdef SIGABRT
(void)apr_signal(SIGABRT,SIG_IGN);
#endif
#ifdef SIGURG
(void)apr_signal(SIGURG,SIG_IGN);
#endif
}

/*----------------------------------------------*/

static void shutdown_proc()
{
_signal_shutdown();

logfile_shutdown();

apr_terminate();
}

/*----------------------------------------------*/

int main (int argc, char * argv[])
{
apr_off_t maxsize;
apr_file_t *f_stdin;
apr_size_t ntoread,nread;
char buf[BUFSIZE],*path;
apr_status_t status;
int optch;
const char *opt_arg;
apr_fileperms_t mode;
int keep_count;

cmd=argv[0];

apr_app_initialize(&argc, (char const * const **)(&argv), NULL);
intr_on();
(void)atexit(shutdown_proc);

/*-- Get options and arg */

maxsize=0;
mode=LOGFILE_MODE;
keep_count=0;

(void)apr_getopt_init(&opt_s,_POOL,argc,(char const * const *)argv);
while (YES)
	{
	status=apr_getopt_long(opt_s,long_options,&optch,&opt_arg);
	if (status==APR_EOF) break;
	if (status != APR_SUCCESS) usage(1);
	switch ((char)optch)
		{
		case 'h':
			usage(0);
			break;

		case 'd':
			set_debug(YES);
			break;

		case 'c':
			if (!init_compress_handler_from_arg(opt_arg))
				{
				usage(-1);
				FATAL_ERROR_1("Invalid compression spec : %s",opt_arg);
				}
			break;

		case 's':
			maxsize=convert_size_string(opt_arg);
			if (maxsize <= 0)
				{
				usage(-1);
				FATAL_ERROR_1("Invalid size : %s",opt_arg);
				}
			break;

		case 'k':
			keep_count=-1;
			keep_count=atoi(opt_arg);
			if (keep_count <= 0)
				{
				usage(-1);
				FATAL_ERROR_1("Invalid keep value : %s",opt_arg);
				}
			break;

		case 'V':
			printf(MANAGELOGS_VERSION "\n");
			exit(0);

		case 'm':
			if (sscanf(opt_arg,"%x",&mode)!=1)
				{
				usage(-1);
				FATAL_ERROR_1("Invalid mode : %s",opt_arg);
				}
			break;

		case 'u':
			change_id(opt_arg);
			break;
		}
	}

path=argv[opt_s->ind];
if ((!path)||(!(*path))||(argv[opt_s->ind+1])) usage(1);

/* Init logfile */

logfile_init(path,maxsize,mode,keep_count);

/* Read by blocks of at most <maxsize> bytes */

ntoread=sizeof(buf);
if (maxsize && (maxsize < ntoread)) ntoread=maxsize;
DEBUG1("Reading by chunks of %d bytes",ntoread);

/* Open stdin for reading */

if (apr_file_open_stdin(&f_stdin,_POOL) != APR_SUCCESS)
	FATAL_ERROR("Cannot open stdin\n");

/* Loop forever */

for (;;)
	{
	nread=ntoread;
	status=apr_file_read(f_stdin, buf, &nread);
	if (status==APR_EOF) do_action(TERMINATE_ACTION);
	if ((status != APR_SUCCESS) exit(3);

	NOINTR_START();
	logfile_write(buf,nread,CAN_ROTATE,time_now());
	NOINTR_END();
	CHECK_EXEC_PENDING_ACTION();
	}

/* return prevents compiler warnings */

return 0;
}

/*----------------------------------------------*/
