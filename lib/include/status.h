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

#ifndef __LMGR_STATUS_H
#define __LMGR_STATUS_H

/*----------------------------------------------*/

LIB_INTERNAL char *status_path(LOGMANAGER mp);
LIB_INTERNAL void get_status_from_file(LOGMANAGER mp);
LIB_INTERNAL void dump_status_to_file(LOGMANAGER mp);

/*----------------------------------------------*/
#endif	/* __LMGR_STATUS_H */

