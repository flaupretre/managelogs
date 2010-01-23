
#include <apr.h>

#if APR_HAVE_STRING_H
#include <string.h>
#endif

#if APR_HAVE_STRINGS_H
#include <strings.h>
#endif

#include <zlib.h>

#include "compress.h"
#include "gz.h"
#include "error.h"

/*----------------------------------------------*/

#define BUFSIZE 65536

#define DEFAULT_COMPRESS_RATIO	10

#define RESET_OUTPUT_BUFFER()	{ \
								zs.next_out=(Bytef *)compbuf; \
								zs.avail_out=BUFSIZE; \
								}

#define WRITE_OUTPUT_BUFFER()	{ \
								if (zs.avail_out != BUFSIZE) \
									logfile_write_bin_raw(compbuf \
										,BUFSIZE-zs.avail_out); \
								}

/*----------------------------------------------*/

static z_stream zs;
static char compbuf[BUFSIZE];
static uLong compress_ratio=DEFAULT_COMPRESS_RATIO;

/*----------------------------------------------*/

static void gz_compute_paths(char *logpath, char *oldpath);
static void gz_start(int level);
static void gz_end(void);
static void gz_predict_compressed_size(apr_size_t *size);
static void gz_compress_and_write(char *buf, apr_size_t size);

/*----------------------------------------------*/

COMPRESS_DEFS gz_compress_defs=
	{
	gz_compute_paths,
	gz_start,
	gz_end,
	gz_predict_compressed_size,
	gz_compress_and_write
	};

/*----------------------------------------------*/

static void gz_compute_paths(char *logpath, char *oldpath)
{
strcat(logpath,".gz");
strcat(oldpath,".gz");
}

/*----------------------------------------------*/

static void gz_start(int level)
{
zs.zalloc=(alloc_func)Z_NULL;
zs.zfree=(free_func)Z_NULL;

if (deflateInit2(&zs,level,Z_DEFLATED,31,8,Z_DEFAULT_STRATEGY)!=Z_OK)
	FATAL_ERROR("gz: Cannot  initialize compression engine");
}

/*----------------------------------------------*/
/* Recompute compress_ratio only if zs.total_in is high enough */

static void gz_end()
{
RESET_OUTPUT_BUFFER();

if (deflate(&zs,Z_FINISH)!=Z_STREAM_END)
	FATAL_ERROR("Cannot flush compressed data\n");

WRITE_OUTPUT_BUFFER();

if ((zs.total_in > 100000) && zs.total_out)
	{
	compress_ratio=zs.total_in/zs.total_out;
	if (compress_ratio==0) compress_ratio=1; /* Should never happen, but... */
	}

deflateEnd(&zs);
}

/*----------------------------------------------*/

static void gz_predict_compressed_size(apr_size_t *sizep)
{
(*sizep) /= compress_ratio;
}

/*----------------------------------------------*/

static void gz_compress_and_write(char *buf, apr_size_t size)
{
zs.next_in=buf;
zs.avail_in=size;
while (zs.avail_in)
	{
	RESET_OUTPUT_BUFFER();
	if (deflate(&zs,Z_NO_FLUSH)!=Z_OK) FATAL_ERROR("Cannot compress data");
	WRITE_OUTPUT_BUFFER();
	}
}

/*----------------------------------------------*/
