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

#if APR_HAVE_STDIO_H
#include <stdio.h>
#endif

/*----------------------------------------------*/

static COMPRESS_HANDLER *compress_handlers[]={
	&plain_handler,
#ifdef HAVE_ZLIB
	&gzip_handler,
#endif
#ifdef HAVE_BZ2
	&bzip2_handler,
#endif
	NULL };

/*----------------------------------------------*/

char *logmanager_compression_list()
{
COMPRESS_HANDLER **chpp;
char *result,*name;
int size;

result=allocate(NULL,1);
result[size=0]='\0';

for (chpp=compress_handlers;*chpp;chpp++)
	{
	name=(*chpp)->name;
	if (! name[0]) continue;	/* Plain handler */
	ALLOC_P(result,size+strlen(name)+2);
	strcat(result," ");
	strcat(result,name);
	}
return result;
}

/*----------------------------------------------*/

LIB_INTERNAL void init_compress_handler_from_string(void *sp, const char *arg)
{
COMPRESS_HANDLER **chpp;
char *buf,*level,*name;
LOGMANAGER mp=(LOGMANAGER )sp;
CHECK_MP(mp);

mp->compress.handler = &plain_handler;	/* Default */

if (arg)
	{
	buf=duplicate(arg);

	if ((level=strchr(buf,':'))!=NULL) *(level++)='\0';

	for (chpp=compress_handlers;*chpp;chpp++)
		{
		name=(*chpp)->name;
		if (!strcmp(name,buf))
			{
			mp->compress.handler=(*chpp);
			if (mp->compress.handler->init)
				mp->compress.handler->init(sp,level);
			break;
			}
		}
	if (!(*chpp)) FATAL_ERROR1("Invalid compression : %s",buf);
	FREE_P(buf);
	}
}

/*----------------------------------------------*/

LIB_INTERNAL void compress_and_write(void *sp, const char *buf
	, apr_off_t size, TIMESTAMP t)
{
LOGMANAGER mp=(LOGMANAGER )sp;
CHECK_MP(mp);

C_VOID_HANDLER2(mp,compress_and_write,buf,size);

/* Update sizes */

mp->active.file->osize += size;
mp->active.file->size=mp->active.fp->size;

mp->active.file->end=t;	/*-- Update end timestamp */

}

/*----------------------------------------------*/
