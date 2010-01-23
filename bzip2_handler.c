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

#include "compress.h"
#include "bzip2_handler.h"
#include "file.h"
#include "util.h"

/*----------------------------------------------*/

#define BUFSIZE 65536

#define DEFAULT_COMPRESS_RATIO	20

#define RESET_OUTPUT_BUFFER()	{ \
				bzs.next_out=compbuf; \
				bzs.avail_out=BUFSIZE; \
				}

#define WRITE_OUTPUT_BUFFER()	{ \
				if ((bzs.avail_out != BUFSIZE) && (outfp!=NULL)) \
					file_write(outfp,compbuf,BUFSIZE-bzs.avail_out); \
				}

/*----------------------------------------------*/

static bz_stream bzs;
static char compbuf[BUFSIZE];
static unsigned int compress_ratio=DEFAULT_COMPRESS_RATIO;
static int compress_level;
/*@null@*/ static OFILE *outfp=(OFILE *)0;

/*----------------------------------------------*/

static int bzip2_get_comp_level(const char *clevel);
static void bzip2_init(const char *clevel);
static void bzip2_start(OFILE *fp);
static void bzip2_end(void);
static void bzip2_predict_size(apr_size_t *size);
static void bzip2_compress_and_write(const char *buf, apr_size_t size);

/*----------------------------------------------*/

COMPRESS_HANDLER bzip2_handler=
	{
	"bzip2",						/* name */
	"bz2",							/* suffix */
	bzip2_init,						/* init */
	bzip2_start,					/* start */
	bzip2_end,						/* end */
	bzip2_predict_size,				/* predict_size */
	bzip2_compress_and_write		/* compress_and_write */
	};

/*----------------------------------------------*/

static int bzip2_get_comp_level(const char *clevel)
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
			FATAL_ERROR_1("Invalid compression level : %s",clevel);
		return (int)(c-'0');
	}
}

/*----------------------------------------------*/

static void bzip2_init(const char *clevel)
{
compress_level=bzip2_get_comp_level(clevel);
}

/*----------------------------------------------*/

static void bzip2_start(OFILE *fp)
{
outfp=fp;

bzs.bzalloc=NULL;
bzs.bzfree=NULL;

if (BZ2_bzCompressInit(&bzs,compress_level,0,0)!=BZ_OK)
	FATAL_ERROR("bzip2: Cannot  initialize compression engine");
}

/*----------------------------------------------*/
/* TODO: 64-bit portable division */

static void bzip2_end()
{
int status;

while(YES)
	{
	RESET_OUTPUT_BUFFER();
	status=BZ2_bzCompress(&bzs,BZ_FINISH);
	if ((status!=BZ_STREAM_END)&&(status!=BZ_FINISH_OK))
		FATAL_ERROR("Cannot flush compressed data\n");
	WRITE_OUTPUT_BUFFER();
	if (status==BZ_STREAM_END) break;
	}

if ((bzs.total_in_hi32==0) && (bzs.total_out_hi32==0)
	&& (bzs.total_in_lo32 > 100000) && (bzs.total_out_lo32!=0))
	{
	compress_ratio=bzs.total_in_lo32/bzs.total_out_lo32;
	if (compress_ratio==0) compress_ratio=1; /* Should never happen, but... */
	}

(void)BZ2_bzCompressEnd(&bzs);
outfp=NULL;
}

/*----------------------------------------------*/

static void bzip2_predict_size(apr_size_t *sizep)
{
(*sizep) /= compress_ratio;
}

/*----------------------------------------------*/

static void bzip2_compress_and_write(const char *buf, apr_size_t size)
{
bzs.next_in=(char *)buf;
bzs.avail_in=(unsigned int)size;
while (bzs.avail_in != 0)
	{
	RESET_OUTPUT_BUFFER();
	if (BZ2_bzCompress(&bzs,BZ_RUN)!=BZ_RUN_OK)
		FATAL_ERROR("Cannot compress data");
	WRITE_OUTPUT_BUFFER();
	}
}

/*----------------------------------------------*/
#endif /* DISABLE_BZIP2 */
