/*=============================================================================

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

#ifndef DISABLE_GZIP
/*----------------------------------------------*/

#include <apr.h>

#if APR_HAVE_STRING_H
#include <string.h>
#endif

#if APR_HAVE_STRINGS_H
#include <strings.h>
#endif

#include <zlib.h>

#include "compress.h"
#include "gzip_handler.h"
#include "file.h"
#include "util.h"

/*----------------------------------------------*/

#define BUFSIZE 65536

#define DEFAULT_COMPRESS_RATIO	10

#define RESET_OUTPUT_BUFFER()	{ \
				zs.next_out=(Bytef *)compbuf; \
				zs.avail_out=BUFSIZE; \
				}

#define WRITE_OUTPUT_BUFFER()	{ \
				if (zs.avail_out != BUFSIZE) \
					file_write(outfp,compbuf,BUFSIZE-zs.avail_out); \
				}

/*----------------------------------------------*/

static z_stream zs;
static char compbuf[BUFSIZE];
static uLong compress_ratio=DEFAULT_COMPRESS_RATIO;
static int compress_level;
static OFILE *outfp=(OFILE *)0;

/*----------------------------------------------*/

static int gzip_get_comp_level(const char *clevel);
static void gzip_init(/*@null@*/ const char *clevel);
static void gzip_start(OFILE *fp);
static void gzip_end(void);
static void gzip_predict_size(apr_size_t *size);
static void gzip_compress_and_write(const char *buf, apr_size_t size);

/*----------------------------------------------*/

COMPRESS_HANDLER gzip_handler=
	{
	"gzip",							/* name */
	"gz",							/* suffix */
	gzip_init,						/* init */
	gzip_start,						/* start */
	gzip_end,						/* end */
	gzip_predict_size,				/* predict_size */
	gzip_compress_and_write			/* compress_and_write */
	};

/*----------------------------------------------*/

static int gzip_get_comp_level(const char *clevel)
{
char c;

if (!clevel) return Z_DEFAULT_COMPRESSION;

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

static void gzip_init(const char *clevel)
{
compress_level=gzip_get_comp_level(clevel);
}

/*----------------------------------------------*/

static void gzip_start(OFILE *fp)
{
outfp=fp;

zs.zalloc=(alloc_func)Z_NULL;
zs.zfree=(free_func)Z_NULL;

if (deflateInit2(&zs,compress_level,Z_DEFLATED,31,8,Z_DEFAULT_STRATEGY)!=Z_OK)
	FATAL_ERROR("gzip: Cannot  initialize compression engine");
}

/*----------------------------------------------*/
/* Recompute compress_ratio only if zs.total_in is high enough */

static void gzip_end()
{
int status;

while(YES)
	{
	RESET_OUTPUT_BUFFER();
	status=deflate(&zs,Z_FINISH);
	if ((status!=Z_STREAM_END)&&(status!=Z_OK))
		FATAL_ERROR("Cannot flush compressed data\n");
	WRITE_OUTPUT_BUFFER();
	if (status==Z_STREAM_END) break;
	}

if ((zs.total_in > 100000) && (zs.total_out!=0))
	{
	compress_ratio=zs.total_in/zs.total_out;
	if (compress_ratio==0) compress_ratio=1; /* Should never happen, but... */
	}

(void)deflateEnd(&zs);
outfp=NULL;
}

/*----------------------------------------------*/

static void gzip_predict_size(apr_size_t *sizep)
{
(*sizep) /= compress_ratio;
}

/*----------------------------------------------*/

static void gzip_compress_and_write(const char *buf, apr_size_t size)
{
zs.next_in=(unsigned char *)buf;
zs.avail_in=(uInt)size;
while (zs.avail_in != 0)
	{
	RESET_OUTPUT_BUFFER();
	if (deflate(&zs,Z_NO_FLUSH)!=Z_OK) FATAL_ERROR("Cannot compress data");
	WRITE_OUTPUT_BUFFER();
	}
}

/*----------------------------------------------*/
#endif /* DISABLE_GZIP */
