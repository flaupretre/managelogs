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

#include "util/util.h"
#include "options.h"
#include "managelogs.h"
#include "id.h"

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
	{"refresh-only",'R',0 },
	{"rotate-cmd",'C',1 },
	{"rotate-delay",'r',1 },
	{"purge-delay",'p',1 },
	{"enospc-abort",'x',0 },
	{"",'\0', 0 }
	};

/*----------------------------------------------*/

static apr_off_t convert_size_string(const char *str);
static TIMESTAMP convert_delay(const char *str);
static void usage(int rc);

/*----------------------------------------------*/

static void usage(int rc)
{
FILE *fd;
char *clist;

fd=((rc>0) ? stderr : stdout);
clist=logmanager_compression_list();

fprintf(fd,MANAGELOGS_BANNER,MANAGELOGS_VERSION);
fprintf(fd,"\n\
Usage: managelogs [options...] <base-path1> [[options...] <base-path2> ...] \n");

fprintf(fd,"\
\n\
*---- Global options (these options apply to the whole process) :\n\
\n\
 -h|--help           Display this message\n\
\n\
 -u|--user <id>      Program runs with this user ID\n\
                       <id> = <uid>[:<gid>]\n\
                       <uid> and <gid> are user/group names or numeric ids\n\
\n\
 -V|--version        Print version banner and exit\n\
\n\
 -I|--stats          Display internal stats before exiting (for\n\
                     troubleshooting, debugging, and performance tests)\n\
\n\
 -R|--refresh-only   Just refresh/purge files, then exit\n\
\n\
*---- per-manager options (these options apply to the next <base-path> only) :\n\
\n\
 -v|--verbose        Increment debug level\n\
\n\
 -d|--debug <path>   Write debug messages to <path>\n\
                       <path> can be a file path, 'stdout', or 'stderr'.\n\
					   Default: stderr\n\
\n\
 -c|--compress <comp>[:<level>]  Activate compression and appends the\n\
                     corresponding suffix to the log file names.\n\
                       <comp> must be one of : %s\n\
                       <level> is one of [123456789bf] (f=fast, b=best)\n\
                       Default level: best\n\
\n\
 -s|--size <size>    Set the maximum file size at which rotation occurs\n\
                       <size> is a numeric value optionnally followed\n\
                       by 'K' (Kilo), 'M' (Mega), or 'G' (Giga).\n\
                       Default: no limit\n\
\n\
 -r|--rotate-delay <delay>  Set the maximum delay between the creation of a new\n\
                       log file and the next rotation.\n\
                       <delay> is a suite of patterns in the form '[0-9]+[dhm]'\n\
                       (d=days, h=hours, m=minutes)\n\
                       Example : 1d12h = 1 day and 12 hours (same as '36h')\n\
 \n\
 -p|--purge-delay <delay>  Remove backup log files older than <delay>\n\
                         Argument: same format as for '--rotate-delay'\n\
\n\
 -S|--global-size <size>  Set the maximum size the whole set of log files can\n\
                     take on disk (active log + backups).\n\
                       Argument: same as for '--size'\n\
\n\
 -m|--mode <mode>    Permissions to set when creating a new log file\n\
                       <mode> is an octal Unix-style file permission\n\
                       (see man chmod(2) for more). Default mode: %x\n\
\n\
 -k|--keep <n>       Only keep <n> log files (the active one + <n-1>\n\
                     backups)\n\
\n\
 -l|--link           Maintain a link to the active log file\n\
\n\
 -L|--backup-links   Maintain links to the active and backup log files\n\
\n\
 -H|--hardlink       Create hard links instead of symbolic links\n\
\n\
 -e|--ignore-eol     Disables the internal mechanism ensuring that log files\n\
                     are rotated on line boundaries\n\
\n\
 -C|--rotate-cmd <cmd>    Execute <cmd> on every rotation\n\
\n\
 -x|--enospc-abort   Abort on 'no more space' write error (default: ignore)\n\
\n",clist,LOGFILE_MODE);

(void)allocate(clist,0);

if (rc >= 0) exit(rc);
}

/*----------------------------------------------*/
/*-- Get options from command line */

LOGMANAGER_OPTIONS **get_options(int argc, char **argv, int *countp)
{
apr_status_t status;
LOGMANAGER_OPTIONS **opp, *op;
int optch;
const char *opt_arg;
DECLARE_TPOOL

opp=(LOGMANAGER_OPTIONS **)0;
(*countp)=0;

while (1)
	{
	if (argc < 2) break;

	op=NEW_STRUCT(LOGMANAGER_OPTIONS);
	op->create_mode=LOGFILE_MODE;
	opp=allocate(opp,(++(*countp))*sizeof(*opp));
	opp[(*countp)-1]=op;

	(void)apr_getopt_init(&opt_s,CHECK_TPOOL(),argc,(char const * const *)argv);
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
				break;

			case 'v':
				if (!op->debug_file) op->debug_file=duplicate("stderr");
				op->debug_level++;
				break;

			case 'c':
				if (strlen(opt_arg) >= LMGR_COMPRESS_STRING_SIZE)
					FATAL_ERROR1("Invalid compression parameter : %s",opt_arg);
				strcpy(op->compress_string,opt_arg);
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
				printf(MANAGELOGS_BANNER,MANAGELOGS_VERSION);
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

			case 'x':
				op->flags |= LMGR_FAIL_ENOSPC;
				break;

			case 't':
				sscanf(opt_arg,"%lX",&timestamp);
				break;

			case 'I':
				stats_toggle=1;
				break;

			case 'R':
				refresh_only=1;
				break;

			case 'C':
				op->rotate_cmd=duplicate(opt_arg);
				break;

			case 'r':
				op->rotate_delay=convert_delay(opt_arg);
				break;

			case 'p':
				op->purge_delay=convert_delay(opt_arg);
				break;
			}
		}

	if (!(argv[opt_s->ind])) usage(1);
	op->base_path=duplicate(argv[opt_s->ind]);
	if (! op->base_path[0]) usage(1);

	argc -= opt_s->ind;
	argv += opt_s->ind;
	}

if (!opp) usage(1);
FREE_TPOOL();
return opp;
}

/*----------------------------------------------*/

static TIMESTAMP convert_delay(const char *str)
{
char c;
TIMESTAMP delay,tdelay;

delay=tdelay=0;

while ((c=(*(str++)))!='\0')
	{
	switch(c)
		{
		case 'd':
		case 'D':
			tdelay *= 24; /* No break here */
		case 'h':
		case 'H':
			tdelay *= 60; /* No break here */
		case 'm':
		case 'M':
			delay += (tdelay * 60);
			tdelay=0;
			break;
		default:
			tdelay = (tdelay*10)+(TIMESTAMP)(c-'0');
		}
	}
			
return delay;
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

void free_options(LOGMANAGER_OPTIONS **opp, int count)
{
int i;

for (i=0;i<count;i++)
	{
	(void)allocate(opp[i]->base_path,0);
	(void)allocate(opp[i]->debug_file,0);
	(void)allocate(opp[i]->rotate_cmd,0);
	(void)allocate(opp[i],0);
	}

(void)allocate(opp,0);
}

/*----------------------------------------------*/
