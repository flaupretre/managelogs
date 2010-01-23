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
#include <apr_getopt.h>

#if APR_HAVE_STDLIB_H
#include <stdlib.h>
#endif

#if APR_HAVE_STRING_H
#include <string.h>
#endif

#include "options.h"
#include "managelogs.h"
#include "id.h"
#include "util.h"

/*----------------------------------------------*/

#define KILO	1024
#define MEGA	(KILO*KILO)
#define GIGA	(MEGA*KILO)

/*----------------------------------------------*/

static apr_getopt_t *opt_s;
static apr_getopt_option_t long_options[]=
	{
	{"help",'h',0 },
	{"debug-file",'d',1 },
	{"verbose",'v',0 },
	{"compress",'c',1 },
	{"size",'s',1 },
	{"global-size",'S',1 },
	{"mode",'m',1 },
	{"user",'u',1 },
	{"keep",'k',1 },
	{"version",'V',0 },
	{"link",'l',0 },
	{"backup-links",'L',0 },
	{"hardlink",'H',0 },
	{"ignore-eol",'e',0 },
	{"time",'t',1 },
	{"stats",'I',0 },
	{"",'\0', 0 }
	};

PRIVATE_POOL

/*----------------------------------------------*/

static apr_off_t convert_size_string(const char *str);
static void usage(int rc);

/*----------------------------------------------*/

static void usage(int rc)
{
FILE *fd;
char *clist;

fd=((rc>0) ? stderr : stdout);
clist=logmanager_compression_list();

fprintf(fd,"\
managelogs version %s\n\
\nUsage: %s [options...] <root-path>\n",MANAGELOGS_VERSION,cmd);

fprintf(fd,"\
\n\
Options :\n\
\n\
 -h|--help           Display this message\n\
\n\
 -v|--verbose        Increment debug level\n\
\n\
 -d|--debug <path>   Set path to debug file and increment debug level\n\
                     Can also be 'stdout' or 'stderr'\n\
\n\
 -c|--compress <comp>[:<level>]  Activate compression and appends the\n\
                     corresponding suffix to the log file names.\n\
                       <comp> is one of : %s\n\
                       <level> is one of {123456789bf} (f=fast, b=best)\n\
                       Default level depends on the compression engine\n\
\n\
 -s|--size <size>    Set the maximum size at which rotation occurs\n\
                       <size> is a numeric value optionnally followed\n\
                       by 'K' (Kilo), 'M' (Mega), or 'G' (Giga). <size>\n\
                       can also be set to 'min' (minimum value)\n\
                       Default: no limit\n\
\n\
 -S|--global-size    Set the maximum size log files will take on disk (active\n\
                       log + backups). Arg syntax: see '--size'\n\
\n\
 -m|--mode <mode>    Permissions to set when creating a new log file\n\
                       <mode> is a numeric Unix-style file permission\n\
                       (man chmod(2) for more). Default mode: %x\n\
\n\
 -u|--user <id>      Program runs with this user ID\n\
                       <id> = <uid>[:<gid>]\n\
                       <uid> and <gid> are user/group names or numeric ids\n\
\n\
 -k|--keep <n>       Only keep <n> log files (the current log file + <n-1>\n\
                     backups)\n\
\n\
 -V|--version        Print version and exit\n\
\n\
 -l|--link           Maintain a link from <root-path> to the current log file\n\
                     (See '-H' to choose between hard/symbolic links)\n\
\n\
 -L|--backup-links   Maintain links to the current and backup log files\n\
                     (backup links are named <root-path>.<1,2,...>, most\n\
                     recent first)\n\
\n\
 -H|--hardlink       Create hard links instead of symbolic links\n\
\n\
 -e|--ignore-eol     By default, log files are always rotated on line\n\
                     boundaries. This flag disables this mechanism.\n\
\n\
 -I|--stats          Display internal stats before exiting (for\n\
                     troubleshooting, debugging, and performance tests)\n\
\n\
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
			if (op->debug_file) op->debug_file=allocate(op->debug_file,0);
			op->debug_file=duplicate(opt_arg);
			/* no break here */
		case 'v':
			if (!op->debug_file) op->debug_file=duplicate("stderr");
			op->debug_level++;
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

		case 'H':
			op->flags |= LMGR_HARD_LINKS;
			break;

		case 'l':
			op->flags |= LMGR_ACTIVE_LINK;
			break;

		case 'L':
			op->flags |= (LMGR_ACTIVE_LINK | LMGR_BACKUP_LINKS);
			break;

		case 'e':
			op->flags |= LMGR_IGNORE_EOL;
			break;

		case 't':
			sscanf(opt_arg,"%lX",&timestamp);
			break;

		case 'I':
			stats_toggle=1;
			break;
		}
	}

if ((!(argv[opt_s->ind])) || (argv[opt_s->ind+1])) usage(1);

op->root_path=duplicate(argv[opt_s->ind]);

if (! op->root_path[0]) usage(1);

return op;
}

/*----------------------------------------------*/

static apr_off_t convert_size_string(const char *str)
{
char c;
apr_off_t result;

result=(apr_off_t)0;

if (!strcmp(str,"min")) return (apr_off_t)1; /* Special value: lower limit */

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

LOGMANAGER_OPTIONS_V1 *free_options(LOGMANAGER_OPTIONS_V1 *op)
{

(void)allocate(op->root_path,0);
(void)allocate(op->compress_string,0);

return allocate(op,0);
}

/*----------------------------------------------*/
