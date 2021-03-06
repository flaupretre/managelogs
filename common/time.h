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

#ifndef __LMGR_TIME_H
#define __LMGR_TIME_H

/* Don't conflict with declaration in logmanager.h */

#ifndef NOW
typedef apr_time_t TIMESTAMP;

#define NOW	(TIMESTAMP)0
#endif

#define TIMESTAMP_FMT APR_TIME_T_FMT

#define NORMALIZE_TIMESTAMP(_t)	{ if ((_t)==NOW) (_t)=time_now(); }

#define SET_LAST_TIMESTAMP(_mp,_t)	{ (_mp)->last_timestamp=(_t); }

#define CHECK_TIMESTAMP(_mp,_t)	{ \
	NORMALIZE_TIMESTAMP(_t); \
	if ((_t) < (_mp)->last_timestamp) (_t)=(_mp)->last_timestamp; \
	SET_LAST_TIMESTAMP((_mp),(_t)); \
	}

/*----------------------------------------------*/

LIB_INTERNAL TIMESTAMP time_now(void);
LIB_INTERNAL TIMESTAMP strval_to_time(const char *val);

/*----------------------------------------------*/
#endif	/* __LMGR_TIME_H */

