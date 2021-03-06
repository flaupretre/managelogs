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

#ifndef __LMGR_CHECKSUM_H
#define __LMGR_CHECKSUM_H

/*----------------------------------------------*/

typedef unsigned long CHECKSUM;

/*----------------------------------------------*/

LIB_INTERNAL CHECKSUM update_checksum(CHECKSUM sum, const char *buf, apr_off_t size);

/*----------------------------------------------*/
#endif	/* __LMGR_CHECKSUM_H */
