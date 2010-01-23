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

#ifndef DISABLE_BZIP2
/*----------------------------------------------*/

#include <apr.h>

#if APR_HAVE_STDIO_H
#include <stdio.h>
#endif

#if APR_HAVE_STRING_H
#include <string.h>
#endif

#if APR_HAVE_STRINGS_H
#include <strings.h>
#endif

#include <bzlib.h>

#include "include/bzip2_handler.h"
#include "include/logmanager.h"
#include "include/file.h"
#include "include/util.h"
#include "include/config.h"

/*----------------------------------------------*/

#define BZ2_DEFAULT_COMPRESS_RATIO	20

#define BZ2_RESET_OUTPUT_BUFFER()	{ \
				zp->zs.next_out=zp->compbuf; \
				zp->zs.avail_out=BUFSIZE; \
				}

#define WRITE_OUTPUT_BUFFER()	{ \
				if (zp->zs.avail_out != BUFSIZE) \
					file_write(mp->active.fp,zp->compbuf \
						,BUFSIZE-zp->zs.avail_out); \
				}

#define BZ2_INIT_POINTERS() \
	LOGMANAGER *mp; \
	BZIP2_DATA *zp; \
	zp=(BZIP2_DATA *)((mp=(LOGMANAGER *)sp)->compress.private); \

/*----------------------------------------------*/

typedef struct
	{
	bz_stream zs;
	char compbuf[BUFSIZE];
	unsigned int compress_ratio;
	int compress_level;
	} BZIP2_DATA;

/*----------------------------------------------*/

static int  _bzip2_get_comp_level(const char *clevel);
static void bzip2_init_v1(void *sp, const char *level);
static void bzip2_destroy(void *sp);
static void bzip2_start(void *sp);
static void bzip2_end(void *sp);
static apr_size_t bzip2_predict_size(void *sp, apr_size_t size);
static void bzip2_compress_and_write(void *sp, const char *buf, apr_size_t size);
static void bzip2_flush(void *sp);

/*----------------------------------------------*/

COMPRESS_HANDLER bzip2_handler=
	{
	"bz2",							/* suffix */
	bzip2_init_v1,					/* init_v1 */
	bzip2_destroy,					/* destroy */
	bzip2_start,						/* start */
	bzip2_end,						/* end */
	bzip2_predict_size,				/* predict_size */
	bzip2_compress_and_write,		/* compress_and_write */
	bzip2_flush						/* flush */
	};

/*----------------------------------------------*/

static int _bzip2_get_comp_level(const char *clevel)
{
char c;

if (!clevel) return 9;

switch (c=(*clevel))
	{
	case 'f': return 1;
	case 'b': return 9;
	case 's': return 2;
	default:
		if ((c<'1')||(c>'9'))
			FATAL_ERROR1("Invalid compression level : %s",clevel);
		return (int)(c-'0');
	}
}

/*----------------------------------------------*/

static void bzip2_init_v1(void *sp, const char *clevel)
{
BZ2_INIT_POINTERS();

zp=NEW_STRUCT(BZIP2_DATA);

zp->compress_ratio=BZ2_DEFAULT_COMPRESS_RATIO;
zp->compress_level=_bzip2_get_comp_level(clevel);

mp->compress.private=zp;
}

/*----------------------------------------------*/

static void bzip2_destroy(void *sp)
{
BZ2_INIT_POINTERS();

mp->compress.private=allocate(mp->compress.private,0);
}

/*----------------------------------------------*/

static void bzip2_start(void *sp)
{
BZ2_INIT_POINTERS();

zp->zs.bzalloc=NULL;
zp->zs.bzfree=NULL;

if (BZ2_bzCompressInit(&(zp->zs),zp->compress_level,0,0)!=BZ_OK)
	FATAL_ERROR("bzip2: Cannot  initialize compression engine");
}

/*----------------------------------------------*/
/* TODO: 64-bit portable division */

static void bzip2_end(void *sp)
{
int status;
BZ2_INIT_POINTERS();

while(YES)
	{
	BZ2_RESET_OUTPUT_BUFFER();
	status=BZ2_bzCompress(&(zp->zs),BZ_FINISH);
	if ((status!=BZ_STREAM_END)&&(status!=BZ_FINISH_OK))
		FATAL_ERROR("Cannot flush compressed data\n");
	WRITE_OUTPUT_BUFFER();
	if (status==BZ_STREAM_END) break;
	}

if ((zp->zs.total_in_hi32==0) && (zp->zs.total_out_hi32==0)
	&& (zp->zs.total_in_lo32 > 100000) && (zp->zs.total_out_lo32!=0))
	{
	zp->compress_ratio=zp->zs.total_in_lo32/zp->zs.total_out_lo32;
	if (zp->compress_ratio==0) zp->compress_ratio=1; /* Should never happen... */
	}

(void)BZ2_bzCompressEnd(&(zp->zs));
}

/*----------------------------------------------*/

static apr_size_t bzip2_predict_size(void *sp, apr_size_t size)
{
BZ2_INIT_POINTERS();

return size/zp->compress_ratio;
}

/*----------------------------------------------*/

static void bzip2_compress_and_write(void *sp, const char *buf, apr_size_t size)
{
BZ2_INIT_POINTERS();

zp->zs.next_in=(char *)buf;
zp->zs.avail_in=(unsigned int)size;

while (zp->zs.avail_in != 0)
	{
	BZ2_RESET_OUTPUT_BUFFER();
	if (BZ2_bzCompress(&(zp->zs),BZ_RUN)!=BZ_RUN_OK)
		FATAL_ERROR("Cannot compress data");
	WRITE_OUTPUT_BUFFER();
	}
}

/*----------------------------------------------*/

static void bzip2_flush(void *sp)
{
bzip2_end(sp);
bzip2_start(sp);
}

/*----------------------------------------------*/
#endif /* DISABLE_BZIP2 */
