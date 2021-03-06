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
#include <apr_strings.h>

#if APR_HAVE_STRING_H
#include <string.h>
#endif

#if APR_HAVE_STDLIB_H
#include <stdlib.h>
#endif

#if WITH_DMALLOC
#include <dmalloc.h>
#endif

#include "global.h"
#include "alloc.h"

/*----------------------------------------------*/

LIB_INTERNAL void *allocate(const void *p, apr_size_t size)
{
void *p2;

p2=(void *)p; /* Avoids compile warnings for const/non-const declarations */

if (p2)
	{
	if (size)
		{
		p2=realloc(p2,(size_t)size);
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
		p2=malloc((size_t)size);
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

LIB_INTERNAL void *combine_strings(const char *str1,const char *str2)
{
size_t size;
char *p;

if (!str1) str1="";
if (!str2) str2="";
size=strlen(str1) + strlen(str2) +1;

p=allocate(NULL,size);
apr_snprintf(p,size,"%s%s",str1,str2);

return p;
}

/*----------------------------------------------*/
