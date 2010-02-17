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

/*----------------------------------------------*/

LIB_INTERNAL char *link_name(LOGMANAGER mp, int num)
{
int len;
char buf[32],*p;

p=allocate(NULL,len=strlen(mp->base_path)+1);
strcpy(p,mp->base_path);

if (num)
	{
	(void)apr_snprintf(buf,sizeof(buf),((num > 999) ? ".B.%d" : ".B.%03d"),num);
	p=allocate(p,len += strlen(buf));
	strcat(p,buf);
	}

if (mp->compress.handler->suffix[0])
	{
	p=allocate(p,len += (strlen(mp->compress.handler->suffix)+1));
	strcat(p,".");
	strcat(p,mp->compress.handler->suffix);
	}

return p;
}

/*----------------------------------------------*/

LIB_INTERNAL void clear_logfile_link(LOGMANAGER mp, LOGFILE *lp)
{
if (lp && lp->link)
	{
	file_delete(lp->link,NO);
	lp->link=allocate(lp->link,0);
	}
}

/*----------------------------------------------*/

LIB_INTERNAL void create_logfile_link(LOGMANAGER mp, LOGFILE *lp,int num)
{
char *lname;

if (!lp) return;

if (((num==0) && (mp->flags & LMGR_ACTIVE_LINK))
	|| ((num!=0) && (mp->flags & LMGR_BACKUP_LINKS)))
	{
	INCR_STAT_COUNT(mp,link);
	lname=link_name(mp,num);
	if (mp->flags & LMGR_HARD_LINKS)
		{
		file_delete(lname,NO);
#if HAVE_LINK
		/* Must use the full path as target */
		(void)link(lp->path,lp->link=lname);
#endif
		}
	else
		{
		file_delete(lname,NO);
#if HAVE_SYMLINK
		(void)symlink(ut_basename(lp->path),lp->link=lname);
#endif
		}
	}
else
	{
	lp->link=NULL;
	}
}

/*----------------------------------------------*/

LIB_INTERNAL void refresh_active_link(LOGMANAGER mp)
{
DEBUG(mp,1,"Refreshing active link");
INCR_STAT_COUNT(mp,refresh_active_link);

clear_logfile_link(mp,mp->active.file);
create_logfile_link(mp,mp->active.file,0);
}

/*------------------------------------------------------------------------*/
