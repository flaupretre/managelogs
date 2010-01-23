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
#include <apr_getopt.h>

#include "options.h"
#include "managelogs.h"
#include "id.h"
#include "lib/include/util.h"

/*----------------------------------------------*/

#define KILO	1024
#define MEGA	(KILO*KILO)
#define GIGA	(MEGA*KILO)

/*----------------------------------------------*/

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

PRIVATE_POOL

/*----------------------------------------------*/

void usage(int rc)
{
FILE *fd;
char *clist;

fd=((rc>0) ? stderr : stdout);
clist=logmanager_compression_list();

fprintf(fd,"\
managelogs version %s\n\
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
/*-- Get options from command line */

LOGMANAGER_OPTIONS_V1 *get_options(int argc, char **argv)
{
apr_status_t status;
LOGMANAGER_OPTIONS_V1 *op;
int optch;
const char *opt_arg;

op=NEW_STRUCT(LOGMANAGER_OPTIONS_V1);

op->create_mode=LOGFILE_MODE;

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
			op->compress_string=duplicate(opt_arg);
			break;

		case 's':
			op->file_maxsize=convert_size_string(opt_arg);
			break;

		case 'S':
			op->global_maxsize=convert_size_string(opt_arg);
			break;

		case 'k':
			op->keep_count=atoi(opt_arg);
			break;

		case 'V':
			printf(MANAGELOGS_VERSION "\n");
			exit(0);

		case 'm':
			if (sscanf(opt_arg,"%x",&(op->create_mode))!=1)
				{
				usage(-1);
				FATAL_ERROR1("Invalid mode : %s",opt_arg);
				}
			break;

		case 'u':
			change_id(opt_arg);
			break;
		}
	}

op->root_path=duplicate(argv[opt_s->ind]);
if ((!(op->root_path)) || (!(*(op->root_path)))) usage(1);

return op;
}

/*----------------------------------------------*/

apr_off_t convert_size_string(const char *str)
{
char c;
apr_off_t result;

result=(apr_off_t)0;
while ((c=(*(str++)))!='\0')
	{
	if ((c=='k')||(c=='K')) return (result*KILO);
	if ((c=='m')||(c=='M')) return (result*MEGA);
	if ((c=='g')||(c=='G')) return (result*GIGA);
	if ((c<'0')||(c>'9')) return (apr_off_t)0;
	result = (result*10)+(apr_off_t)(c-'0');
	}
return result;
}

/*----------------------------------------------*/
