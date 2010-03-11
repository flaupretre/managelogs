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
#include <apr_file_info.h>
#include <apr_strings.h>

#if APR_HAVE_STRING_H
#include <string.h>
#endif

/*----------------------------------------------*/

#define _IS_SEPARATOR(_c) ((_c=='/')||(_c=='\\'))

/*----------------------------------------------*/
/* Return a pointer to the char after the last separator. If a separator
 * is not found, return a pointer to the full string.
 * Warning : the output is not duplicated : it is a pointer inside the input
 */

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
	if (_IS_SEPARATOR(c)) return (p+1);
	}
return path;
}

/*----------------------------------------------*/
/* Return a duplicate of the dirname of a path (with the trailing separator).
 * An empty string and a string without separator return "./"
 */

LIB_INTERNAL char *ut_dirname(const char *path)
{
char *p2,c;
int i;

for (i=strlen(path)-1;;i--)
	{
	if (i < 0) return duplicate("./");
	c=path[i];
	if (_IS_SEPARATOR(c))
		{
		p2=duplicate_mem(path,i+2);
		p2[i+1]='\0';
		return p2;
		}
	}
}

/*----------------------------------------------*/
/*
 * If path is an absolute path, return a copy of path
 * If path is relative return the corresponding absolute path, computed
 * from base_dir or from the current dir if base_dir is null.
 * On entry :
 *   - base_dir is an absolute path or null
 *   - path is a relative or absolute path (or null)
 * Return : a newly-allocated string containing an absolute path
 * The return string is newly-allocated, even if <path> is absolute.
 * If <path> is null, return null.
 */

LIB_INTERNAL char *mk_abs_path(const char *base_dir, const char *path)
{
char *ap,*ap2;
const char *p,*p2;
apr_status_t status;
char errbuf[1024];
DECLARE_TPOOL;

if (!path) return NULL;

p=NULL;
p2=path;
status=apr_filepath_root(&p,&p2,0,CHECK_TPOOL());
if (status==APR_SUCCESS) return duplicate(path); /* Absolute path */

status=apr_filepath_merge(&ap,base_dir,path,APR_FILEPATH_NOTRELATIVE
	,CHECK_TPOOL());
if (status != APR_SUCCESS)
	FATAL_ERROR3("Cannot compute absolute path - %s (base=%s, path=%s)"
		,apr_strerror(status,errbuf,sizeof(errbuf))
		,(base_dir ? base_dir : "<NULL>")
		,path);

ap2=duplicate(ap); /* Duplicate mem before freeing it */
FREE_TPOOL();
return ap2;
}

/*----------------------------------------------*/
/* Returns a relative path computed from the base_dir if possible.
 * If computing a relative path is not possible, return the
 * received path.
 * The returned string is NOT allocated. It is a pointer into the
 * input string.
 */

LIB_INTERNAL const char *get_rel_path(const char *base_dir, size_t base_dir_len, const char *path)
{
size_t plen;
const char *p;
char c;

plen=strlen(path);

if ((!base_dir_len) || (plen <= base_dir_len)) return path;	/* Too short */
if (strncmp(base_dir,path,base_dir_len)) return path; /* Prefix does not match */

for (p=path+base_dir_len;;p++)
	{
	c=(*p);
	if (! _IS_SEPARATOR(c)) return p;
	if (!c) break;
	}

return path;
}

/*----------------------------------------------*/
