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

for (result=NULL,size=0,chpp=compress_handlers;*chpp;chpp++)
	{
	name=(*chpp)->name;
	if (! (*chpp)->suffix[0]) continue;	/* Plain handler */
	size += strlen(name)+1;
	ALLOC_P(result,size+1);
	strcat(result," ");
	strcat(result,name);
	}
return result;
}

/*----------------------------------------------*/

LIB_INTERNAL COMPRESS_HANDLER *compress_handler_from_string(const char *type)
{
COMPRESS_HANDLER **chpp,*hp;

hp = &plain_handler;	/* Default */

if (type && type[0])
	{
	for (chpp=compress_handlers;*chpp;chpp++)
		{
		if (!strcmp((*chpp)->name,type))
			{
			hp=(*chpp);
			break;
			}
		}
	if (!hp) FATAL_ERROR1("Invalid compression : %s",type);
	}

return hp;
}

/*----------------------------------------------*/

LIB_INTERNAL void compress_and_write(void *sp, const char *buf
	, apr_off_t size, TIMESTAMP t)
{
LOGMANAGER *mp=(LOGMANAGER *)sp;
CHECK_MP(mp);

C_VOID_HANDLER2(mp,compress_and_write,buf,size);

/* Update original size (compressed is updated by write_level3) */

mp->active.file->osize += size;

mp->active.file->end=t;	/*-- Update end timestamp */

}

/*----------------------------------------------*/
