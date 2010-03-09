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

#include "../config.h"

#include <apr.h>
#include <apr_getopt.h>

#if APR_HAVE_STDLIB_H
#include <stdlib.h>
#endif

#if APR_HAVE_STRING_H
#include <string.h>
#endif

#include "../common/global.h"
#include "../common/alloc.h"
#include "../common/time.h"
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
	{"ignore-enospc",'x',0 },
	{"input",'i',1 },
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

fprintf(fd,MANAGELOGS_BANNER "\n\
Usage: managelogs [options...] <base-path1> [[options...] <base-path2> ...] \n");

fprintf(fd,"\n\
*---- Global options (these options apply to the whole process) :\n\
\n\
 -h|--help                 Display this message\n\
 -u|--user <id>            Program runs with this user ID\n\
 -V|--version              Print version banner and exit\n\
 -I|--stats                Display internal stats before exiting \n\
 -R|--refresh-only         Just refresh/purge files, then exit\n\
 -i|--input <path>         Read from <file> (Default : stdin)\n\
\n\
*---- per-manager options (these options apply to the next <base-path> only) :\n\
\n\
 -v|--verbose              Increment debug level\n\
 -d|--debug <path>         Write debug messages to <path>\n");

if (clist[0]) /* If at least one compression scheme is supported */
	{
	fprintf(fd,"\
 -c|--compress <cp>[:lvl]  Activate compression, <cp> is one of : %s\n",clist);
	}

fprintf(fd,"\
 -s|--size <size>          Set the file size limit\n\
 -r|--rotate-delay <delay> Set the rotation delay\n\
 -p|--purge-delay <delay>  Set the purge delay\n\
 -S|--global-size <size>   Set the global size limit\n\
 -m|--mode <mode>          Permissions to set when creating a log file\n\
 -k|--keep <n>             Only keep <n> log files\n\
 -l|--link                 Maintain a link to the active log file\n\
 -L|--backup-links         Also maintain links to backup log files\n\
 -H|--hardlink             Create hard links instead of symbolic links\n\
 -e|--ignore-eol           Disables the eol buffering mechanism\n\
 -C|--rotate-cmd <cmd>     Execute <cmd> on every rotation\n\
 -x|--ignore-enospc        Ignore 'no more space' write errors\n\
\n");

FREE_P(clist);

if (rc >= 0) exit_proc(rc);
}

/*----------------------------------------------*/
/*-- Get options from command line */

LOGMANAGER_OPTIONS **get_options(int argc, char **argv, int *countp)
{
apr_status_t status;
LOGMANAGER_OPTIONS **opp, *op;
int optch;
unsigned int mode;
const char *opt_arg;
char *clevel;
DECLARE_TPOOL;

opp=(LOGMANAGER_OPTIONS **)0;
(*countp)=0;

while (1)
	{
	if (argc < 2) break;

	op=NEW_STRUCT(LOGMANAGER_OPTIONS);
	op->create_mode=LOGFILE_MODE;
	op->flags=LMGR_PID_FILE;	/* maintain a pid file */
	ALLOC_P(opp,(++(*countp))*sizeof(*opp));
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
				FREE_TPOOL();
				usage(0);
				break;

			case 'd':
				DUP_P(op->debug_file,opt_arg);
				break;

			case 'v':
				op->debug_level++;
				break;

			case 'c':
				DUP_P(op->compress.type,opt_arg);
				clevel=strchr(op->compress.type,':');
				if (clevel)
					{
					*(clevel++)='\0';
					DUP_P(op->compress.level,clevel);
					}
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
				printf(MANAGELOGS_BANNER);
				exit_proc(0);

			case 'm':
				if (sscanf(opt_arg,"%x",&mode)!=1)
					{
					usage(-1);
					FATAL_ERROR1("Invalid mode : %s",opt_arg);
					}
				op->create_mode=(apr_fileperms_t)mode;
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
				op->flags |= LMGR_IGNORE_ENOSPC;
				break;

			case 't':
				sscanf(opt_arg,"%" TIMESTAMP_FMT,&timestamp);
				break;

			case 'I':
				stats_toggle=YES;
				break;

			case 'R':
				refresh_only=YES;
				break;

			case 'C':
				DUP_P(op->rotate_cmd,opt_arg);
				break;

			case 'r':
				op->rotate_delay=convert_delay(opt_arg);
				break;

			case 'p':
				op->purge_delay=convert_delay(opt_arg);
				break;

			case 'i':
				DUP_P(input_path,opt_arg);
				break;
			}
		}

	if (!(argv[opt_s->ind])) usage(1);
	DUP_P(op->base_path,argv[opt_s->ind]);
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
char c,*input_str;
apr_off_t result;

input_str=str;
result=(apr_off_t)0;

if (!strcmp(str,"min")) return (apr_off_t)1; /* Special value: lower limit */

while ((c=(*(str++)))!='\0')
	{
	if ((c=='k')||(c=='K')) return (result*KILO);
	if ((c=='m')||(c=='M')) return (result*MEGA);
	if ((c=='g')||(c=='G')) return (result*GIGA);
	if ((c<'0')||(c>'9')) FATAL_ERROR1("Invalid size string: %s",input_str);
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
	FREE_P(opp[i]->base_path);
	FREE_P(opp[i]->debug_file);
	FREE_P(opp[i]->rotate_cmd);
	FREE_P(opp[i]->compress.type);
	FREE_P(opp[i]->compress.level);
	FREE_P(opp[i]);
	}

FREE_P(opp);
}

/*----------------------------------------------*/
