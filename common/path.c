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

#include "global.h"
#include "path.h"
#include "alloc.h"

/*----------------------------------------------*/
/* Return a pointer to the char after the last separator. If a separator
is not found, return a pointer to the full string.
Warning : the output is not duplicated : it is a pointer inside the input */

LIB_INTERNAL const char *ut_basename(const char *path)
{
const char *p;
char c;
int i;

for (i=strlen(path);;i--)
	{
	if (!i) break;
	c=(*(p=path+i));
	if (!c) continue; /* First char of non-empty string */
	if ((c=='/')||(c=='\\')) return (p+1);
	}
return path;
}

/*----------------------------------------------*/
/* Return a duplicate of the dirname of a path (with the trailing separator) */
/* An empty string and a string without separator return a null pointer */

LIB_INTERNAL char *ut_dirname(const char *path)
{
const char *p;
char *p2,c;
int i;

for (i=strlen(path)-1;;i--)
	{
	if (i < 0) return NULL;
	c=(*(p=&(path[i])));
	if ((c=='/')||(c=='\\'))
		{
		p2=duplicate_mem(path,i+2);
		p2[i+1]='\0';
		return p2;
		}
	}
}

/*----------------------------------------------*/

LIB_INTERNAL char *ut_absolute_path(const char *root_dir, const char *str)
{
char *p;
int len;

if (!root_dir) return duplicate(str);

p=allocate(NULL,len=strlen(root_dir)+strlen(str)+2);
(void)apr_snprintf(p,len,"%s%s",root_dir,str);
return p;
}

/*----------------------------------------------*/
