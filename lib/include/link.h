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

#ifndef __LINK_H
#define __LINK_H

/*----------------------------------------------*/

LIB_INTERNAL char *link_name(LOGMANAGER mp, int num);
LIB_INTERNAL void refresh_active_link(LOGMANAGER mp);
LIB_INTERNAL void clear_logfile_link(LOGMANAGER mp, LOGFILE *lp);
LIB_INTERNAL void create_logfile_link(LOGMANAGER mp, LOGFILE *lp,int num);

/*----------------------------------------------*/
#endif	/* __LINK_H */
