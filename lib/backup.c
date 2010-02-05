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
/* We must proceed in 2 steps : first clear all, then (re)create all.
* If we don't do that, clear can destroy new links
*/

LIB_INTERNAL void refresh_backup_links(LOGMANAGER mp)
{
int i;

DEBUG(mp,1,"Refreshing backup links");
INCR_STAT_COUNT(refresh_backup_links);

if (BACKUP_COUNT(mp))
	{
	for (i=0;i<BACKUP_COUNT(mp);i++)
		{
		clear_logfile_link(mp,BACKUP_FILES(mp)[i]);
		}

	for (i=0;i<BACKUP_COUNT(mp);i++)
		{
		create_logfile_link(mp,BACKUP_FILES(mp)[i],i+1);
		}
	}
}

/*----------------------------------------------*/
/* Before removing backup files, we check the existing backup files on disk
to confirm actual sizes. If a backup file has been deleted by an external
action, maybe we don't actually exceed the limits */

LIB_INTERNAL void purge_backup_files(LOGMANAGER mp,apr_off_t add,TIMESTAMP t)
{
if (GLOBAL_CONDITIONS_EXCEEDED(mp,add,t))
	{
	_sync_logfiles_from_disk(mp);	/* Confirm that limits are REALLY exceeded */

	while (GLOBAL_CONDITIONS_EXCEEDED(mp,add,t)) remove_oldest_backup(mp);
	}
}

/*----------------------------------------------*/

LIB_INTERNAL void remove_oldest_backup(LOGMANAGER mp)
{
if (! BACKUP_COUNT(mp)) return; /* Should never happen */

DEBUG1(mp,1,"Removing oldest backup file (%s)",OLDEST_BACKUP_FILE(mp)->path);
INCR_STAT_COUNT(remove_oldest);

mp->backup.size -= OLDEST_BACKUP_FILE(mp)->size;
DELETE_LOGFILE(OLDEST_BACKUP_FILE(mp));

ARRAY_DECR_COUNT(mp->backup.files);
}

/*------------------------------------------------------------------------*/
