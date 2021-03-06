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

#include "../config.h"

#include <apr.h>
#include <apr_user.h>
#include <apr_lib.h>
#include <apr_file_io.h>
#include <apr_strings.h>

#if APR_HAVE_UNISTD_H
#include <unistd.h>
#endif

#if APR_HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif

#include "../common/global.h"

#if APR_HAS_USER

/*----------------------------------------------*/

void change_id(const char *string)
{
char buf[64],*group;
apr_uid_t uid;
apr_gid_t gid;
int read_uid,read_gid; /* For better portability */
BOOL gid_set;
DECLARE_TPOOL;

if (strlen(string) >= sizeof(buf))
	FATAL_ERROR1("ID string too long (%s)",string);
(void)apr_cpystrn(buf,string,sizeof(buf));

gid_set=NO;
if ((group=strchr(buf,':'))!=NULL) *(group++)='\0';

if (apr_isdigit(*buf))
	{
	if (sscanf(buf,"%d",&read_uid)!=1) FATAL_ERROR1("Invalid uid (%s)",buf);
	uid=(apr_uid_t)read_uid;
	}
else
	{
	if (apr_uid_get(&uid,&gid,buf,CHECK_TPOOL()) != APR_SUCCESS)
		FATAL_ERROR1("Cannot convert username to uid (%s)",buf);
	gid_set=YES;
	}

if (group)
	{
	if (apr_isdigit(*group))
		{
		if (sscanf(group,"%d",&read_gid)!=1)	FATAL_ERROR1("Invalid gid (%s)",group);
		gid=(apr_gid_t)read_gid;
		}
	else
		{
		if (apr_gid_get(&gid,group,CHECK_TPOOL()) != APR_SUCCESS)
			FATAL_ERROR1("Cannot convert group name to gid (%s)",group);
		}
	gid_set=YES;
	}

if (gid_set)
	{
	if (setgid((gid_t)gid))
		FATAL_ERROR1("Cannot change effective group ID to %lu"
			,(unsigned long)gid);
	}

if (setuid((uid_t)uid)) FATAL_ERROR1("Cannot change effective user ID to %lu"
	,(unsigned long)uid);

FREE_TPOOL();
}

#endif

