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

#ifndef __MANAGELOGS_H
#define __MANAGELOGS_H

/*----------------------------------------------*/

#define MANAGELOGS_BANNER	"\
managelogs V %s\n\
(C) 2008-2010 - Francois Laupretre <francois@tekwire.net>\n\
For more info, please consult http://managelogs.tekwire.net\n"

#define LOGFILE_MODE	0x644

/*----------------------------------------------*/

extern TIMESTAMP timestamp;
extern int stats_toggle;
extern int refresh_only;
extern LOGMANAGER **mpp;
extern int mgr_count;

/*----------------------------------------------*/
#endif	/* __MANAGELOGS_H */
