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

/*=============================================================================

Gestion des logs Apache.

Ce programme est une alternative a rotatelogs.

Il gere la rotation des logs et la purge des anciens fichiers sur la base
d'une taille maximale occupee sur le disque.

Usage :	s'utilise dans une directive de log de type CustomLog, ErrorLog, etc.

Le fichier <path_log> grossira jusqu'a (<maxsize>/2), puis sera renomme
en <path_log>.old, l'ancien <path_log>.old sera supprime, et un nouveau
fichier <path_log> vide sera cree.
On est donc sur que la taille totale des fichiers <path_log> et <path_log>.old
ne depasse jamais <maxsize>.

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

#include <zlib.h>

#include "logfile.h"
#include "error.h"

/*----------------------------------------------*/

#define MANAGELOGS_VERSION	"1.0b1"

#define BUFSIZE		 65536

#define KILO	1024
#define MEGA	(KILO*KILO)
#define GIGA	(MEGA*KILO)

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
	{"",'\0', 0 }
	};

/*----------------------------------------------*/

static apr_off_t get_size_arg(const char *str);
static int get_compress_level(char c);
static void usage(int rc);
static void shutdown_proc(void);
static void sighup_handler(int signum);
static void sigusr1_handler(int signum);

/*----------------------------------------------*/

static apr_off_t get_size_arg(const char *str)
{
char c;
apr_off_t result;

result=(apr_off_t)0;
while ((c=(*(str++)))!='\0')
	{
	if ((c=='k')||(c=='K')) return (result*KILO);
	if ((c=='m')||(c=='M')) return (result*MEGA);
	if ((c=='g')||(c=='G')) return (result*GIGA);
	if ((c<'0')||(c>'9')) usage(1);
	result = (result*10)+(apr_off_t)(c-'0');
	}
return result;
}

/*----------------------------------------------*/

static int get_compress_level(char c)
{
switch (c)
	{
	case 'd': return Z_DEFAULT_COMPRESSION;
	case 'f': return Z_BEST_SPEED;
	case 'b': return Z_BEST_COMPRESSION;
	default:
		if ((c<'1')||(c>'9')) usage(1);
		return (int)(c-'0');
	}
}

/*----------------------------------------------*/

static void usage(int rc)
{
fprintf(rc ? stderr : stdout,"\
managelogs %s\n\
\nUsage: %s [options...] <path>\n\
\n\
Options :\n\
\n\
 -h|--help           Display this message\n\
 -c|--compress <compression> Activate compression\n\
 -l|--level <level>  Set compression level\n\
                        <level> is one of {0123456789bdf}\n\
                        (d=default, f=fast, b=best)\n\
 -s|--size <size>    Set the maximal size log files can take on disk\n\
                        <size> is a numeric value optionnally followed\n\
                        by one of 'k' (kilo), 'm' (mega), or 'g' (giga)\n\
                        Default: 0 => no limit\n\
\n",MANAGELOGS_VERSION,cmd);

exit(rc);
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
int optch,compress,compress_level;
const char *optarg;

cmd=argv[0];

apr_app_initialize(&argc, (char const * const **)(&argv), NULL);
atexit(shutdown_proc);
apr_pool_create(&pool, NULL);

/*-- Get options and arg */

maxsize=limit=0;
compress=0;
compress_level=Z_DEFAULT_COMPRESSION;

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

		case 'c':
			compress=1;
			break;

		case 'l':
			compress_level=get_compress_level(*optarg);
			break;

		case 's':
			maxsize=get_size_arg(optarg);
			limit=maxsize/2;
			break;
		}
	}

path=argv[opt_s->ind];
if ((!path)||(!(*path))||(argv[opt_s->ind+1]))usage(1);

/* Init logfile */

logfile_init(path,compress,compress_level,maxsize);

/* Register signal handlers */

(void)apr_signal(SIGHUP,sighup_handler);
(void)apr_signal(SIGUSR1,sigusr1_handler);

/* Read by blocks of at most <maxsize> bytes */

ntoread=sizeof(buf);
if (limit && (limit < ntoread)) ntoread=limit;

/* Open stdin for reading */

if (apr_file_open_stdin(&f_stdin,pool) != APR_SUCCESS)
	FATAL_ERROR("Cannot open stdin\n");

/* Loop forever */

for (;;)
	{
	nread=ntoread;
	if (apr_file_read(f_stdin, buf, &nread) != APR_SUCCESS) exit(3);

	logfile_write_bin(buf,nread,CAN_ROTATE);
	}

/* return prevents compiler warnings */

return 0;
}

/*----------------------------------------------*/
