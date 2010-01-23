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

#include "compress.h"
#include "util.h"

#include "gzip_handler.h"
#include "bzip2_handler.h"

/*----------------------------------------------*/

static void plain_compress_and_write(const char *buf, apr_size_t size);

/*----------------------------------------------*/

static COMPRESS_HANDLER plain_handler=
	{
	NULL,						/* name */
	NULL,						/* suffix */
	NULL,						/* init */
	NULL,						/* start */
	NULL,						/* end */
	NULL,						/* predict_size */
	plain_compress_and_write	/* compress_and_write */
	};

static COMPRESS_HANDLER *compress_handlers[]={
#ifndef DISABLE_GZIP
	&gzip_handler,
#endif
#ifndef DISABLE_BZIP2
	&bzip2_handler,
#endif
	NULL };

COMPRESS_HANDLER *compress_handler=&plain_handler;	/* Global */

/*----------------------------------------------*/

static void plain_compress_and_write(const char *buf, apr_size_t size)
{
logfile_write_bin_raw(buf,size);
}

/*----------------------------------------------*/

char *compress_handler_list()
{
COMPRESS_HANDLER **chpp;
char *result,*name;
int size;

result=allocate(NULL,1);
result[size=0]='\0';

for (chpp=compress_handlers;*chpp;chpp++)
	{
	name=(*chpp)->name;
	result=allocate(result,size+strlen(name)+2);
	strcat(result," ");
	strcat(result,name);
	}
return result;
}

/*----------------------------------------------*/

int init_compress_handler_from_arg(const char *arg)
{
COMPRESS_HANDLER **chpp;
char buf[16],*level;

if (strlen(arg) >= sizeof(buf)) FATAL_ERROR("compression arg too long");
strcpy(buf,arg);

if ((level=strchr(buf,':'))!=NULL) *(level++)='\0';

for (chpp=compress_handlers;*chpp;chpp++)
	{
	if (!strcmp((*chpp)->name,buf))
		{
		compress_handler=(*chpp);
		C_HANDLER(init,(level)); 
		break;
		}
	}
if (!(*chpp)) return 0;

return 1;
}

/*----------------------------------------------*/
