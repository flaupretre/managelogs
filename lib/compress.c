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

#define PLAIN_INIT_POINTERS() LOGMANAGER mp=(LOGMANAGER )sp;

/*----------------------------------------------*/

static void plain_write(void *sp, const char *buf, apr_size_t size);

/*----------------------------------------------*/

static COMPRESS_HANDLER plain_handler=
	{
	"",				/* suffix */
	NULL,			/* init */
	NULL,			/* destroy */
	NULL,			/* start */
	NULL,			/* end */
	NULL,			/* predict_size */
	plain_write		/* compress_and_write */
	};

static COMPRESS_HANDLER *compress_handlers[]={
	&plain_handler,
#ifndef DISABLE_GZIP
	&gzip_handler,
#endif
#ifndef DISABLE_BZIP2
	&bzip2_handler,
#endif
	NULL };

/*----------------------------------------------*/

static void plain_write(void *sp, const char *buf, apr_size_t size)
{
PLAIN_INIT_POINTERS();

file_write(((LOGMANAGER )sp)->active.fp,buf,size,mp->flags & LMGR_FAIL_ENOSPC);
}

/*----------------------------------------------*/

LIB_INTERNAL char *compress_handler_list()
{
COMPRESS_HANDLER **chpp;
char *result,*suffix;
int size;

result=allocate(NULL,1);
result[size=0]='\0';

for (chpp=compress_handlers;*chpp;chpp++)
	{
	if (! *(suffix=(*chpp)->suffix)) continue;	/* Plain handler */
	result=allocate(result,size+strlen(suffix)+2);
	strcat(result," ");
	strcat(result,suffix);
	}
return result;
}

/*----------------------------------------------*/

LIB_INTERNAL void init_compress_handler_from_string(void *sp, char *arg)
{
COMPRESS_HANDLER **chpp;
char buf[LMGR_COMPRESS_STRING_SIZE],*level;
LOGMANAGER mp=(LOGMANAGER )sp;

if ((strlen(arg)+1) >= sizeof(buf)) FATAL_ERROR("compression arg too long");
strcpy(buf,arg);

if ((level=strchr(buf,':'))!=NULL) *(level++)='\0';

for (chpp=compress_handlers;*chpp;chpp++)
	{
	if (!strcmp((*chpp)->suffix,buf))
		{
		mp->compress.handler=(*chpp);
		if ((*chpp)->init_v1) (*chpp)->init_v1(sp,level);
		break;
		}
	}
if (!(*chpp)) FATAL_ERROR1("Invalid compression : %s",buf);
}

/*----------------------------------------------*/
