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

#ifndef __LMGR_WRITE_H
#define __LMGR_WRITE_H

/*----------------------------------------------*/

#define FUTURE_ACTIVE_SIZE(_mp,_add) \
	(ACTIVE_SIZE(_mp) + ((_mp)->eol_buffer.len + _add)/mp->compress.ratio)

/*----------------------------------------------*/

LIB_INTERNAL void write_end(LOGMANAGER *mp);
LIB_INTERNAL void write_level2(LOGMANAGER *mp, const char *buf, apr_off_t size
	,unsigned int flags, TIMESTAMP t);
LIB_INTERNAL void write_level3(void *_mp, const char *buf, apr_off_t size);

/*----------------------------------------------*/
#endif	/* __LMGR_WRITE_H */
