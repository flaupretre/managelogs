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

#define PLAIN_INIT_POINTERS() LOGMANAGER mp=(LOGMANAGER )sp;

/*----------------------------------------------*/

static apr_size_t plain_predict_size(void *sp, apr_size_t size);
static void plain_compress_and_write(void *sp, const char *buf, apr_size_t size);

/*----------------------------------------------*/

LIB_INTERNAL COMPRESS_HANDLER plain_handler=
	{
	"",							/* suffix */
	"none",						/* name */
	NULL,						/* init */
	NULL,						/* destroy */
	NULL,						/* start */
	NULL,						/* end */
	plain_predict_size,			/* predict_size */
	plain_compress_and_write,	/* compress_and_write */
	NULL						/* flush */
	};

/*----------------------------------------------*/

static apr_size_t plain_predict_size(void *sp, apr_size_t size)
{
return size;
}

/*----------------------------------------------*/

static void plain_compress_and_write(void *sp, const char *buf, apr_size_t size)
{
PLAIN_INIT_POINTERS();

file_write(mp->active.fp,buf,size,mp->flags & LMGR_FAIL_ENOSPC);
}

/*----------------------------------------------*/
