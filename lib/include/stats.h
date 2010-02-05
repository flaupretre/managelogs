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

#ifndef __STATS_H
#define __STATS_H

/*----------------------------------------------*/

#define STAT_COUNT_ITEM(_item)	mp->stats._item ## _count

#define INCR_STAT_COUNT(_item)	{ STAT_COUNT_ITEM(_item)++; }

/*----------------------------------------------*/
#endif	/* __STATS_H */


