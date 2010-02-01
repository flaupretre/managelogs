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
#include <apr_file_io.h>

#if APR_HAVE_UNISTD_H
#include <unistd.h>
#endif

#if APR_HAVE_CTYPE_H
#include <ctype.h>
#endif

#if APR_HAVE_SYS_TYPE_H
#include <sys/types.h>
#endif

#if APR_HAVE_STRING_H
#include <string.h>
#endif

#if APR_HAVE_STDLIB_H
#include <stdlib.h>
#endif

#if APR_HAVE_STDIO_H
#include <stdio.h>
#endif

#include "util.h"

/*----------------------------------------------*/

LIB_INTERNAL void fatal_error_2(const char *msg, const char *arg1, const char *arg2)
{
apr_file_t *fd;
DECLARE_TPOOL

(void)apr_file_open_stderr(&fd,CHECK_TPOOL());

(void)apr_file_printf(fd,"*** Fatal Error : ");
(void)apr_file_printf(fd,msg,arg1,arg2);
(void)apr_file_printf(fd,"\n");

(void)apr_file_close(fd);

FREE_TPOOL();
exit(1);
}

/*----------------------------------------------*/

LIB_INTERNAL void *allocate(const void *p, size_t size)
{
void *p2;

p2=(void *)p; /* Avoids compile warnings for const/non-const declarations */

if (p2)
	{
	if (size)
		{
		p2=realloc(p2,size);
		if (!p2) FATAL_ERROR("realloc error");
		}
	else
		{
		free(p2);
		p2=NULL;
		}
	}
else
	{
	if (size)
		{
		p2=malloc(size);
		if (!p2) FATAL_ERROR("malloc error");
		memset(p2,0,size);
		}
	}

return p2;
}	

/*----------------------------------------------*/

LIB_INTERNAL void *duplicate(const char *string)
{
if (!string) return NULL; /* Test before calling strlen() */

return duplicate_mem(string,(apr_size_t)(strlen(string)+1));
}

/*----------------------------------------------*/

LIB_INTERNAL void *duplicate_mem(const void *source,apr_size_t size)
{
void *p;

if ((!source)||(!size)) return NULL;

p=allocate(NULL,size);
memcpy(p,source,size);
return p;
}

/*----------------------------------------------*/

LIB_INTERNAL unsigned long strval_to_ulong(const char *val)
{
unsigned long l;

if (sscanf(val,"%lu",&l)!=1)
	FATAL_ERROR1("Cannot read numeric value (%s)",val);

return l;
}

/*----------------------------------------------*/
