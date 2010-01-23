/*
===============================================================================

Gestion des logs Apache.

Auteur : F. Laupretre (OCT-2008)

Ce programme derive de rotatelogs.

Il gere la rotation des logs et la purge des anciens fichiers sur la base
d'une taille maximale.

Usage :	s'utilise dans une directive de log de type CustomLog, ErrorLog, etc
sous la forme :
	"|/<path absolu>/manage_logs <path log> <size>"
ou <path log> est le path absolu du fichier log a creer
et <size> est la taille maxi, eventuellement suffixee par 'm' ou 'k'.

Le fichier <path log> grossira donc jusqu'a (<size>/2), puis sera renomme
en <path log>.old, l'ancien <path log>.old sera supprime, et un nouveau
<path log> sera cree. Donc, on est sur que la taille totale des fichiers
<path log> et <path log>.old ne depasse jamais <size>.

===============================================================================
*/

#include "global.h"
#include "managelogs.h"
#include "logfile.h"

#include <apr_file_io.h>
#include <apr_file_info.h>
#include <apr_signal.h>
#include <apr_getopt.h>

/*----------------------------------------------*/

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
	{"compress",'c',0 },
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

void fatal_error_1(char *msg, char *arg)
{
fprintf(stderr,"*** Fatal Error : ");
fprintf(stderr,msg,arg);
fprintf(stderr,"\n");
exit(1);
}

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
	case 'd': return 6;
	case 'f': return 1;
	case 'b': return 9;
	default:
		if ((c<'1')||(c>'9')) usage(1);
		return (int)(c-'0');
	}
}

/*----------------------------------------------*/

static void usage(int rc)
{
fprintf(stderr,"\nUsage: %s [-h] [-c] [-l <level>] [-s <size>]\n\
\n\
Options :\n\
\n\
 -h : display this message\n\
 -c : Activate compression\n\
 -l : set compression level\n\
       Argument : one of {0123456789bdf} (d='default', f='fast', b='best')\n\
       Default = 6\n\
 -s : Set max size.\n\
       Argument : number optionnally suffixed with one of {kmg}\n\
       Default: 0 => no size limit\n\n",cmd);

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
compress_level=6;

(void)apr_getopt_init(&opt_s,pool,argc,(char const * const *)argv);
while (1)
	{
	status=apr_getopt_long(opt_s,long_options,&optch,&optarg);
	if (status==APR_EOF) break;
	if (status != APR_SUCCESS) exit(1);
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
if ((!path)||(!(*path))) usage(1);

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
	{
	fprintf(stderr, "Unable to open stdin\n");
	exit(1);
	}

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
