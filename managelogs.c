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
#include <apr_signal.h>
#include <apr_getopt.h>

#include "logfile.h"
#include "compress.h"
#include "util.h"

/*----------------------------------------------*/

#define MANAGELOGS_VERSION	"1.0b1"

/*----------------------------------------------*/

static char *cmd;
static apr_pool_t *pool;

static apr_getopt_t *opt_s;
static apr_getopt_option_t long_options[]=
	{
	{"help",'h',0 },
	{"compress",'c',1 },
	{"level",'l',1 },
	{"size",'s',1 },
	{"debug",'d',0 },
	{"",'\0', 0 }
	};

/*----------------------------------------------*/

static void usage(int rc);
static void shutdown_proc(void);
static void sighup_handler(int signum);
static void sigusr1_handler(int signum);

/*----------------------------------------------*/

static void usage(int rc)
{
FILE *fd;
char *clist;

fd=(rc ? stderr : stdout);
clist=compress_handler_list();

fprintf(fd,"\
managelogs %s\n\
\nUsage: %s [options...] <path>\n",MANAGELOGS_VERSION,cmd);

fprintf(fd,"\
\n\
Options :\n\
\n\
 -h|--help            Display this message\n\
\n\
 -d|--debug           Display debug messages to stdout\n\
 \n\
 -c|--compress <comp[:level]>  Activate compression\n\
                        <comp> is one of : %s\n\
						<level> is one of {0123456789bf} (f=fast, b=best)\n\
                        Default level depends on compression engine\n\
\n\
 -s|--size <size>    Set the maximal size log files can take on disk\n\
                        <size> is a numeric value optionnally followed\n\
                        by one of 'k' (kilo), 'm' (mega), or 'g' (giga)\n\
                        Default: 0 => no limit\n\n",clist);

allocate(clist,0);

if (rc >= 0) exit(rc);
}

/*----------------------------------------------*/

static void shutdown_proc()
{
logfile_shutdown();

apr_terminate();
}

/*----------------------------------------------*/

static void sighup_handler(int signum)
{
static int running=0;

if (running) return;

running=1;
logfile_rotate();
running=0;
}

/*----------------------------------------------*/

static void sigusr1_handler(int signum)
{
static int running=0;

if (running) return;

running=1;
logfile_flush();
running=0;
}

/*----------------------------------------------*/

int main (int argc, char * argv[])
{
apr_off_t maxsize,limit;
apr_file_t *f_stdin;
apr_size_t ntoread,nread;
char buf[BUFSIZE],*path;
apr_status_t status;
int optch;
const char *optarg;

cmd=argv[0];

apr_app_initialize(&argc, (char const * const **)(&argv), NULL);
atexit(shutdown_proc);
apr_pool_create(&pool, NULL);

/*-- Get options and arg */

maxsize=limit=0;

(void)apr_getopt_init(&opt_s,pool,argc,(char const * const *)argv);
while (1)
	{
	status=apr_getopt_long(opt_s,long_options,&optch,&optarg);
	if (status==APR_EOF) break;
	if (status != APR_SUCCESS) usage(1);
	switch ((char)optch)
		{
		case 'h':
			usage(0);

		case 'd':
			debug_on();
			break;

		case 'c':
			if (!init_compress_handler_from_arg(optarg))
				{
				usage(1);
				FATAL_ERROR_1("Invalid compression arg : %s",optarg);
				}
			break;

		case 's':
			maxsize=convert_size_string(optarg);
			if (maxsize == 0)
				{
				usage(-1);
				FATAL_ERROR_1("Invalid size : %s",optarg);
				}
			limit=maxsize/2;
			break;
		}
	}

path=argv[opt_s->ind];
if ((!path)||(!(*path))||(argv[opt_s->ind+1])) usage(1);

/* Init logfile */

logfile_init(path,maxsize);

/* Register signal handlers */

(void)apr_signal(SIGHUP,sighup_handler);
(void)apr_signal(SIGUSR1,sigusr1_handler);

/* Read by blocks of at most <limit> bytes */

ntoread=sizeof(buf);
if (limit && (limit < ntoread)) ntoread=limit;
DEBUG1("Reading by chunks of %d bytes",ntoread);

/* Open stdin for reading */

if (apr_file_open_stdin(&f_stdin,pool) != APR_SUCCESS)
	FATAL_ERROR("Cannot open stdin\n");

/* Loop forever */

for (;;)
	{
	nread=ntoread;
	if ((status=apr_file_read(f_stdin, buf, &nread)) != APR_SUCCESS)
		exit((status==APR_EOF) ? 0 :3);

	logfile_write_bin(buf,nread,CAN_ROTATE);
	}

/* return prevents compiler warnings */

return 0;
}

/*----------------------------------------------*/
