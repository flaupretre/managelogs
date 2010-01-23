
#include "logfile.h"

#include <apr.h>
#include <apr_file_io.h>
#include <apr_file_info.h>

#include <zlib.h>

/*----------------------------------------------*/

#define RESET_OUTPUT_BUFFER()	{ \
								zs.next_out=compbuf; \
								zs.avail_out=BUFSIZE; \
								}

#define WRITE_OUTPUT_BUFFER(_can_rotate)	{ \
								if (zs.avail_out != BUFSIZE) \
									logfile_write_bin_raw(compbuf \
										,BUFSIZE-zs.avail_out,_can_rotate); \
								}

#define FATAL_ERROR(_msg)		{ \
								fatal_error_1(_msg,NULL); \
								}

#define FATAL_ERROR_1(_msg,_arg) { \
								fatal_error_1(_msg,_arg); \
								}

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

#define BUFSIZE 65536

/*----------------------------------------------*/

static apr_file_t *logfd = NULL;
static char logpath[MAX_PATH],oldpath[MAX_PATH],pidpath[MAX_PATH];
static apr_off_t logsize;
static apr_pool_t *pool;
static int compress_toggle,compress_level;
static z_stream zs;
static int logfile_is_open=0;
static char compbuf[BUFSIZE];
static apr_off_t maxsize,limit;
static int intr_flag=0;
static int rotate_requested=0;
static int flush_requested=0;

/*----------------------------------------------*/

extern void fatal_error_1(char *msg, char *arg);

/*----------------------------------------------*/

static void create_pid_file(void);
static void destroy_pid_file(void);
static void logfile_open(void);
static void logfile_close(void);
static void logfile_write_bin_raw(char *buf, apr_size_t size
	,rotate_flag can_rotate);
static void logfile_do_flush(void);
static void logfile_do_flush(void);
static void logfile_do_rotate(void);

/*----------------------------------------------*/

static void create_pid_file()
{
static apr_file_t *fd;
char buf[32];
int nb;

sprintf(pidpath,"%s.pid",logpath);
fd=NULL;
apr_file_open(&fd,pidpath,APR_WRITE|APR_CREATE|APR_TRUNCATE,APR_OS_DEFAULT,pool);
if (!fd) FATAL_ERROR_1("Cannot open pid file (%s)",pidpath);

sprintf(buf,"%lu\n",(unsigned long)getpid());
nb=strlen(buf);
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

void logfile_init(char *path,int compress_toggle_value,int compress_level_value
	,apr_off_t maxsize_value)
{
NO_INTR_START();

apr_pool_create(&pool, NULL);

compress_toggle=compress_toggle_value;
compress_level=compress_level_value;
maxsize=maxsize_value;
limit=maxsize/2;

strcpy(logpath,path);
sprintf(oldpath,"%s.old",path);
if (compress_toggle)
	{
	strcat(logpath,".gz");
	strcat(oldpath,".gz");
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

if (compress_toggle)
	{
	zs.zalloc=(alloc_func)Z_NULL;
	zs.zfree=(free_func)Z_NULL;
	if (deflateInit2(&zs,compress_level,Z_DEFLATED,31,8
		,Z_DEFAULT_STRATEGY)!=Z_OK)
			FATAL_ERROR_1("Cannot open log file (%s)",logpath);
	}

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

if (compress_toggle)
	{
	RESET_OUTPUT_BUFFER();
	if (deflate(&zs,Z_FINISH)!=Z_STREAM_END)
		FATAL_ERROR("Cannot flush compressed data\n");
	WRITE_OUTPUT_BUFFER(CANNOT_ROTATE);
	deflateEnd(&zs);
	}

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

static void logfile_write_bin_raw(char *buf, apr_size_t size
	, rotate_flag can_rotate)
{
apr_size_t nwrite;

if (!size) return;

NO_INTR_START();

if (can_rotate && limit && ((logsize+size) > limit)) logfile_do_rotate();

nwrite=size;
apr_file_write(logfd, buf, &nwrite);
if (nwrite!=size) FATAL_ERROR_1("Cannot write to log file (%s)\n", logpath);

logsize += size;

NO_INTR_END();
}

/*----------------------------------------------*/

void logfile_write_bin(char *buf, apr_size_t size, rotate_flag can_rotate)
{
if (!size) return;

if (compress_toggle)
	{
	zs.next_in=buf;
	zs.avail_in=size;
	while (zs.avail_in)
		{
		RESET_OUTPUT_BUFFER();
		if (deflate(&zs,Z_NO_FLUSH)!=Z_OK) FATAL_ERROR("Cannot compress data");
		WRITE_OUTPUT_BUFFER(can_rotate);
		}
	}
else logfile_write_bin_raw(buf,size,can_rotate);
}

/*----------------------------------------------*/

void logfile_write(char *str)
{
apr_size_t len;

NO_INTR_START();

len=(apr_size_t)strlen(str);

logfile_write_bin(str,len,CAN_ROTATE);
logfile_write_bin("\n",1,CANNOT_ROTATE);

NO_INTR_END();
}

/*----------------------------------------------*/
