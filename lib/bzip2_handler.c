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

#ifdef HAVE_BZ2
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

/*----------------------------------------------*/

#define BZ2_RESET_OUTPUT_BUFFER()	{ \
				zp->zs.next_out=zp->compbuf; \
				zp->zs.avail_out=BUFSIZE; \
				}

#define BZ2_WRITE_OUTPUT_BUFFER()	{ \
				if (zp->zs.avail_out != BUFSIZE) \
					zp->write_func(zp->write_arg,zp->compbuf \
						,BUFSIZE-zp->zs.avail_out); \
				}

#define BZ2_ZP_INIT	BZIP2_DATA *zp=(BZIP2_DATA *)_zp

/*----------------------------------------------*/

typedef struct
	{
	bz_stream zs;
	char compbuf[BUFSIZE];
	int compress_level;
	WRITE_FUNC write_func;
	void *write_arg;
	} BZIP2_DATA;

/*----------------------------------------------*/

static int _bzip2_get_comp_level(const char *clevel);
static void *bzip2_init(void *zp, const char *level
	, WRITE_FUNC write_func, void *write_arg);
static void bzip2_start(void *zp);
static void bzip2_stop(void *zp);
static void bzip2_compress_and_write(void *zp, const char *buf, apr_off_t size);
static void bzip2_flush(void *zp);

/*----------------------------------------------*/

LIB_INTERNAL COMPRESS_HANDLER bzip2_handler=
	{
	"bz2",							/* suffix */
	"bz2",							/* name */
	20,								/* default_ratio */
	bzip2_init,						/* init */
	NULL,							/* destroy */
	bzip2_start,					/* start */
	bzip2_stop,						/* stop */
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

static void *bzip2_init(void *_zp, const char *clevel
	, WRITE_FUNC write_func, void *write_arg)
{
BZ2_ZP_INIT;

zp=NEW_STRUCT(BZIP2_DATA);

zp->compress_level=_bzip2_get_comp_level(clevel);

zp->write_func=write_func;
zp->write_arg=write_arg;

return zp;
}

/*----------------------------------------------*/

static void bzip2_start(void *_zp)
{
BZ2_ZP_INIT;

zp->zs.bzalloc=NULL;
zp->zs.bzfree=NULL;

if (BZ2_bzCompressInit(&(zp->zs),zp->compress_level,0,0)!=BZ_OK)
	FATAL_ERROR("bzip2: Cannot  initialize compression engine");
}

/*----------------------------------------------*/

static void bzip2_stop(void *_zp)
{
int status;
BZ2_ZP_INIT;

while(YES)
	{
	BZ2_RESET_OUTPUT_BUFFER();
	status=BZ2_bzCompress(&(zp->zs),BZ_FINISH);
	if ((status!=BZ_STREAM_END)&&(status!=BZ_FINISH_OK))
		FATAL_ERROR("Cannot flush compressed data\n");
	BZ2_WRITE_OUTPUT_BUFFER();
	if (status==BZ_STREAM_END) break;
	}

(void)BZ2_bzCompressEnd(&(zp->zs));
}

/*----------------------------------------------*/

static void bzip2_compress_and_write(void *_zp, const char *buf, apr_off_t size)
{
BZ2_ZP_INIT;

zp->zs.next_in=(char *)buf;
zp->zs.avail_in=(unsigned int)size;

while (zp->zs.avail_in != 0)
	{
	BZ2_RESET_OUTPUT_BUFFER();
	if (BZ2_bzCompress(&(zp->zs),BZ_RUN)!=BZ_RUN_OK)
		FATAL_ERROR("Cannot compress data");
	BZ2_WRITE_OUTPUT_BUFFER();
	}
}

/*----------------------------------------------*/

static void bzip2_flush(void *_zp)
{
bzip2_stop(_zp);
bzip2_start(_zp);
}

/*----------------------------------------------*/
#endif /* HAVE_BZ2 */
