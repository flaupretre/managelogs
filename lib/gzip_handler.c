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

#define GZ_RESET_OUTPUT_BUFFER()	{ \
				zp->zs.next_out=(Bytef *)(zp->compbuf); \
				zp->zs.avail_out=BUFSIZE; \
				}

#define GZ_WRITE_OUTPUT_BUFFER()	{ \
				if (zp->zs.avail_out != BUFSIZE) \
					zp->write_func(zp->write_arg,zp->compbuf \
						,BUFSIZE-zp->zs.avail_out); \
				}

#define GZ_ZP_INIT	GZIP_DATA *zp=(GZIP_DATA *)_zp

/*----------------------------------------------*/

typedef struct
	{
	z_stream zs;
	char compbuf[BUFSIZE];
	int compress_level;
	WRITE_FUNC write_func;
	void *write_arg;
	} GZIP_DATA;

/*----------------------------------------------*/

static int _gzip_get_comp_level(const char *clevel);
static void *gzip_init(void *_zp, const char *level
	, WRITE_FUNC write_func, void *write_arg);
static void gzip_start(void *_zp);
static void gzip_stop(void *_zp);
static void gzip_compress_and_write(void *_zp, const char *buf, apr_off_t size);
static void gzip_flush(void *_zp);

/*----------------------------------------------*/

LIB_INTERNAL COMPRESS_HANDLER gzip_handler=
	{
	"gz",						/* suffix */
	"gz",						/* name */
	10,							/* default_ratio */
	gzip_init,					/* init */
	NULL,						/* destroy */
	gzip_start,					/* start */
	gzip_stop,					/* stop */
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

static void *gzip_init(void *_zp, const char *clevel
	, WRITE_FUNC write_func, void *write_arg)
{
GZ_ZP_INIT;

zp=NEW_STRUCT(GZIP_DATA);

zp->compress_level=_gzip_get_comp_level(clevel);

zp->write_func=write_func;
zp->write_arg=write_arg;

return zp;
}

/*----------------------------------------------*/

static void gzip_start(void *_zp)
{
GZ_ZP_INIT;

zp->zs.zalloc=(alloc_func)Z_NULL;
zp->zs.zfree=(free_func)Z_NULL;

if (deflateInit2(&(zp->zs),zp->compress_level,Z_DEFLATED,31,8
	,Z_DEFAULT_STRATEGY)!=Z_OK)
	FATAL_ERROR("gzip: Cannot  initialize compression engine");
}

/*----------------------------------------------*/

static void gzip_stop(void *_zp)
{
int status;
GZ_ZP_INIT;

while(YES)
	{
	GZ_RESET_OUTPUT_BUFFER();
	status=deflate(&(zp->zs),Z_FINISH);
	if ((status!=Z_STREAM_END)&&(status!=Z_OK))
		FATAL_ERROR("Cannot flush compressed data\n");
	GZ_WRITE_OUTPUT_BUFFER();
	if (status==Z_STREAM_END) break;
	}

(void)deflateEnd(&(zp->zs));
}

/*----------------------------------------------*/

static void gzip_compress_and_write(void *_zp, const char *buf, apr_off_t size)
{
GZ_ZP_INIT;

zp->zs.next_in=(unsigned char *)buf;
zp->zs.avail_in=(uInt)size;

while (zp->zs.avail_in != 0)
	{
	GZ_RESET_OUTPUT_BUFFER();
	if (deflate(&(zp->zs),Z_NO_FLUSH)!=Z_OK)
		FATAL_ERROR("Cannot compress data");
	GZ_WRITE_OUTPUT_BUFFER();
	}
}

/*----------------------------------------------*/

static void gzip_flush(void *_zp)
{
gzip_stop(_zp);
gzip_start(_zp);
}

/*----------------------------------------------*/
#endif /* HAVE_ZLIB */
