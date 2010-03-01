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

#include <apr.h>

#if APR_HAVE_STRING_H
#include <string.h>
#endif

#if APR_HAVE_STRINGS_H
#include <strings.h>
#endif

/*----------------------------------------------*/

#define PLAIN_ZP_INIT	PLAIN_DATA *zp; zp=(PLAIN_DATA *)_zp

/*----------------------------------------------*/

typedef struct
	{
	WRITE_FUNC write_func;
	void *write_arg;
	} PLAIN_DATA;

/*----------------------------------------------*/

static void *plain_init(void *_zp, const char *level
	, WRITE_FUNC write_func, void *write_arg);
static void plain_compress_and_write(void *_zp, const char *buf
	, apr_off_t size);

/*----------------------------------------------*/

LIB_INTERNAL COMPRESS_HANDLER plain_handler=
	{
	"",							/* suffix */
	"none",						/* name */
	1,							/* default_ratio */
	plain_init,					/* init */
	NULL,						/* destroy */
	NULL,						/* start */
	NULL,						/* stop */
	plain_compress_and_write,	/* compress_and_write */
	NULL						/* flush */
	};

/*----------------------------------------------*/

static void *plain_init(void *_zp, const char *clevel
	, WRITE_FUNC write_func, void *write_arg)
{
PLAIN_ZP_INIT;

zp=NEW_STRUCT(PLAIN_DATA);

zp->write_func=write_func;
zp->write_arg=write_arg;

return zp;
}

/*----------------------------------------------*/

static void plain_compress_and_write(void *_zp, const char *buf, apr_off_t size)
{
PLAIN_ZP_INIT;

zp->write_func(zp->write_arg,buf,size);
}

/*----------------------------------------------*/
