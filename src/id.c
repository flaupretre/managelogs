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

#include <unistd.h>
#include <ctype.h>
#include <stdio.h>
#include <pwd.h>
#include <grp.h>
#include <sys/types.h>

#include <apr.h>

#include "util.h"

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
	FATAL_ERROR1("ID string too long (%s)",string);
strcpy(buf,string);

gid_set=NO;
if ((group=strchr(buf,':'))!=NULL) *(group++)='\0';

if (isdigit(*buf))
	{
	if (sscanf(buf,"%d",&uid)!=1) FATAL_ERROR1("Invalid uid (%s)",buf);
	}
else
	{
	if ((pp=getpwnam(buf))==NULL)
		FATAL_ERROR1("Cannot convert username to uid (%s)",buf);
	uid=pp->pw_uid;
	gid=pp->pw_gid;
	gid_set=YES;
	}

if (group)
	{
	if (isdigit(*group))
		{
		if (sscanf(group,"%d",&gid)!=1)
			FATAL_ERROR1("Invalid gid (%s)",group);
		}
	else
		{
		if ((gp=getgrnam(group))==NULL)
			FATAL_ERROR1("Cannot convert group name to gid (%s)",group);
		gid=gp->gr_gid;
		}
	gid_set=YES;
	}

if (gid_set)
	{
	if (setgid(gid))
		FATAL_ERROR1("Cannot change effective group ID to %d",(char *)gid);
	}

if (setuid(uid)) FATAL_ERROR1("Cannot change effective user ID to %d",(char *)uid);
}

