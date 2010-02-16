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
char *result,*suffix;
int size;

result=allocate(NULL,1);
result[size=0]='\0';

for (chpp=compress_handlers;*chpp;chpp++)
	{
	suffix=(*chpp)->suffix;
	if (! suffix[0]) continue;	/* Plain handler */
	result=allocate(result,size+strlen(suffix)+2);
	strcat(result," ");
	strcat(result,suffix);
	}
return result;
}

/*----------------------------------------------*/

LIB_INTERNAL void init_compress_handler_from_string(void *sp, const char *arg)
{
COMPRESS_HANDLER **chpp;
char *buf,*level,*suffix;
LOGMANAGER mp=(LOGMANAGER )sp;

mp->compress.handler = &plain_handler;	/* Default */

if (arg)
	{
	buf=duplicate(arg);

	if ((level=strchr(buf,':'))!=NULL) *(level++)='\0';

	for (chpp=compress_handlers;*chpp;chpp++)
		{
		suffix=(*chpp)->suffix;
		if (!suffix[0]) continue; /* Ignore plain handler */
		if (!strcmp(suffix,buf))
			{
			mp->compress.handler=(*chpp);
			if (mp->compress.handler->init)
				mp->compress.handler->init(sp,level);
			break;
			}
		}
	(void)allocate(buf,0);
	if (!(*chpp)) FATAL_ERROR1("Invalid compression : %s",buf);
	}
}

/*----------------------------------------------*/

