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

#include <unistd.h>
#include <ctype.h>
#include <stdio.h>
#include <pwd.h>
#include <grp.h>
#include <sys/types.h>

#include <apr.h>

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

#define KILO	1024
#define MEGA	(KILO*KILO)
#define GIGA	(MEGA*KILO)

/*----------------------------------------------*/

BOOL debug_toggle=NO;

/*----------------------------------------------*/

void fatal_error_2(const char *msg, const char *arg1, const char *arg2)
{
fprintf(stderr,"*** Fatal Error : ");
fprintf(stderr,msg,arg1,arg2);
fprintf(stderr,"\n");
exit(1);
}

/*----------------------------------------------*/

void *allocate(const void *p, size_t size)
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
		}
	}

return p2;
}	

/*----------------------------------------------*/

void *duplicate(const char *string)
{
void *p;
size_t size;

p=allocate(NULL,size=(strlen(string)+1));
memcpy(p,string,size);
return p;
}

/*----------------------------------------------*/

apr_off_t convert_size_string(const char *str)
{
char c;
apr_off_t result;

result=(apr_off_t)0;
while ((c=(*(str++)))!='\0')
	{
	if ((c=='k')||(c=='K')) return (result*KILO);
	if ((c=='m')||(c=='M')) return (result*MEGA);
	if ((c=='g')||(c=='G')) return (result*GIGA);
	if ((c<'0')||(c>'9')) return (apr_off_t)0;
	result = (result*10)+(apr_off_t)(c-'0');
	}
return result;
}

/*----------------------------------------------*/

void set_debug(BOOL toggle)
{
debug_toggle=toggle;
}

/*----------------------------------------------*/

void change_id(const char *string)
{
char buf[64],*group;
uid_t uid;
gid_t gid;
BOOL gid_set;
struct passwd *pp;
struct group *gp;

if (strlen(string) >= sizeof(buf))
	FATAL_ERROR_1("ID string too long (%s)",string);
strcpy(buf,string);

gid_set=NO;
if ((group=strchr(buf,':'))!=NULL) *(group++)='\0';

if (isdigit(*buf))
	{
	if (sscanf(buf,"%d",&uid)!=1) FATAL_ERROR_1("Invalid uid (%s)",buf);
	}
else
	{
	if ((pp=getpwnam(buf))==NULL)
		FATAL_ERROR_1("Cannot convert username to uid (%s)",buf);
	uid=pp->pw_uid;
	gid=pp->pw_gid;
	gid_set=YES;
	}

if (group)
	{
	if (isdigit(*group))
		{
		if (sscanf(group,"%d",&gid)!=1)
			FATAL_ERROR_1("Invalid gid (%s)",group);
		}
	else
		{
		if ((gp=getgrnam(group))==NULL)
			FATAL_ERROR_1("Cannot convert group name to gid (%s)",group);
		gid=gp->gr_gid;
		}
	gid_set=YES;
	}

if (gid_set)
	{
	if (setgid(gid))
		FATAL_ERROR_1("Cannot change effective group ID to %d",(char *)gid);
	}

if (setuid(uid)) FATAL_ERROR_1("Cannot change effective user ID to %d",(char *)uid);
}

/*----------------------------------------------*/
