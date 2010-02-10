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

#ifdef HAVE_ZLIB
/*----------------------------------------------*/

#include <apr.h>

#if APR_HAVE_STRING_H
#include <string.h>
#endif

#if APR_HAVE_STRINGS_H
#include <strings.h>
#endif

#include <zlib.h>

/*----------------------------------------------*/

#define GZ_DEFAULT_COMPRESS_RATIO	10

#define GZ_RESET_OUTPUT_BUFFER()	{ \
				zp->zs.next_out=(Bytef *)(zp->compbuf); \
				zp->zs.avail_out=BUFSIZE; \
				}

#define WRITE_OUTPUT_BUFFER()	{ \
				if (zp->zs.avail_out != BUFSIZE) \
					file_write(mp->active.fp,zp->compbuf \
					  ,BUFSIZE-zp->zs.avail_out,mp->flags & LMGR_FAIL_ENOSPC); \
				}

#define GZ_INIT_POINTERS() \
	LOGMANAGER mp; \
	GZIP_DATA *zp; \
	zp=(GZIP_DATA *)((mp=(LOGMANAGER )sp)->compress.private); \

/*----------------------------------------------*/

typedef struct
	{
	z_stream zs;
	char compbuf[BUFSIZE];
	uLong compress_ratio;
	int compress_level;
	} GZIP_DATA;

/*----------------------------------------------*/

static int  _gzip_get_comp_level(const char *clevel);
static void gzip_init(void *sp, const char *level);
static void gzip_destroy(void *sp);
static void gzip_start(void *sp);
static void gzip_end(void *sp);
static apr_size_t gzip_predict_size(void *sp, apr_size_t size);
static void gzip_compress_and_write(void *sp, const char *buf, apr_size_t size);
static void gzip_flush(void *sp);

/*----------------------------------------------*/

LIB_INTERNAL COMPRESS_HANDLER gzip_handler=
	{
	"gz",						/* suffix */
	gzip_init,					/* init */
	gzip_destroy,				/* destroy */
	gzip_start,					/* start */
	gzip_end,					/* end */
	gzip_predict_size,			/* predict_size */
	gzip_compress_and_write,	/* compress_and_write */
	gzip_flush					/* flush */
	};

/*----------------------------------------------*/

static int _gzip_get_comp_level(const char *clevel)
{
char c;

if (!clevel) return Z_BEST_COMPRESSION;

switch (c=(*clevel))
	{
	case 'f': return Z_BEST_SPEED;
	case 'b': return Z_BEST_COMPRESSION;
	default:
		if ((c<'1')||(c>'9'))
			FATAL_ERROR1("Invalid compression level : %s",clevel);
		return (int)(c-'0');
	}
}

/*----------------------------------------------*/

static void gzip_init(void *sp, const char *clevel)
{
GZ_INIT_POINTERS();

zp=NEW_STRUCT(GZIP_DATA);

zp->compress_ratio=GZ_DEFAULT_COMPRESS_RATIO;
zp->compress_level=_gzip_get_comp_level(clevel);

mp->compress.private=zp;
}

/*----------------------------------------------*/

static void gzip_destroy(void *sp)
{
GZ_INIT_POINTERS();

mp->compress.private=allocate(mp->compress.private,0);
}

/*----------------------------------------------*/

static void gzip_start(void *sp)
{
GZ_INIT_POINTERS();

zp->zs.zalloc=(alloc_func)Z_NULL;
zp->zs.zfree=(free_func)Z_NULL;

if (deflateInit2(&(zp->zs),zp->compress_level,Z_DEFLATED,31,8
	,Z_DEFAULT_STRATEGY)!=Z_OK)
	FATAL_ERROR("gzip: Cannot  initialize compression engine");
}

/*----------------------------------------------*/
/* Recompute compress_ratio only if zs.total_in is high enough */

static void gzip_end(void *sp)
{
int status;
GZ_INIT_POINTERS();

while(YES)
	{
	GZ_RESET_OUTPUT_BUFFER();
	status=deflate(&(zp->zs),Z_FINISH);
	if ((status!=Z_STREAM_END)&&(status!=Z_OK))
		FATAL_ERROR("Cannot flush compressed data\n");
	WRITE_OUTPUT_BUFFER();
	if (status==Z_STREAM_END) break;
	}

if ((zp->zs.total_in > 100000) && (zp->zs.total_out!=0))
	{
	zp->compress_ratio=zp->zs.total_in/zp->zs.total_out;
	if (zp->compress_ratio==0) zp->compress_ratio=1; /* Should never happen...*/
	}

(void)deflateEnd(&(zp->zs));
}

/*----------------------------------------------*/

static apr_size_t gzip_predict_size(void *sp, apr_size_t size)
{
GZ_INIT_POINTERS();

return size/zp->compress_ratio;
}

/*----------------------------------------------*/

static void gzip_compress_and_write(void *sp, const char *buf, apr_size_t size)
{
GZ_INIT_POINTERS();

zp->zs.next_in=(unsigned char *)buf;
zp->zs.avail_in=(uInt)size;

while (zp->zs.avail_in != 0)
	{
	GZ_RESET_OUTPUT_BUFFER();
	if (deflate(&(zp->zs),Z_NO_FLUSH)!=Z_OK)
		FATAL_ERROR("Cannot compress data");
	WRITE_OUTPUT_BUFFER();
	}
}

/*----------------------------------------------*/

static void gzip_flush(void *sp)
{
gzip_end(sp);
gzip_start(sp);
}

/*----------------------------------------------*/
#endif /* HAVE_ZLIB */
