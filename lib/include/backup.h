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

#ifndef __BACKUP_H
#define __BACKUP_H

/*----------------------------------------------*/

#define BACKUP_COUNT(_mp)	ARRAY_COUNT((_mp)->backup.files)
#define BACKUP_FILES(_mp)	ARRAY_ITEMS((_mp)->backup.files)
#define BACKUP_SIZE(_mp)	(_mp)->backup.size

#define OLDEST_BACKUP_FILE(_mp)	(BACKUP_FILES(_mp)[BACKUP_COUNT(_mp)-1])

/*----------------------------------------------*/

LIB_INTERNAL void purge_backup_files(LOGMANAGER mp,apr_off_t add,TIMESTAMP t);
LIB_INTERNAL void remove_oldest_backup(LOGMANAGER mp);
LIB_INTERNAL void refresh_backup_links(LOGMANAGER mp);

/*----------------------------------------------*/
#endif	/* __BACKUP_H */

