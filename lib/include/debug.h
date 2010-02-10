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

#ifndef __DEBUG_H
#define __DEBUG_H

/*----------------------------------------------*/

#define DBG_PREFIX "> "

#define DBG_PASS_LEVEL(_mp,_level) (_level <= (_mp)->debug.level)

#define DEBUG(_mp,_level,_fmt)		{\
	if (DBG_PASS_LEVEL(_mp,_level))\
		(void)apr_file_printf(debug_fd(_mp),DBG_PREFIX _fmt "\n");\
	}

#define DEBUG1(_mp,_level,_fmt,_a1)		{\
	if (DBG_PASS_LEVEL(_mp,_level))\
		(void)apr_file_printf(debug_fd(_mp),DBG_PREFIX _fmt "\n",_a1);\
	}

#define DEBUG2(_mp,_level,_fmt,_a1,_a2)		{\
	if (DBG_PASS_LEVEL(_mp,_level))\
		(void)apr_file_printf(debug_fd(_mp),DBG_PREFIX _fmt "\n",_a1,_a2);\
	}

#define DEBUG3(_mp,_level,_fmt,_a1,_a2,_a3)		{\
	if (DBG_PASS_LEVEL(_mp,_level))\
		(void)apr_file_printf(debug_fd(_mp),DBG_PREFIX _fmt "\n",_a1,_a2,_a3);\
	}

/*----------------------------------------------*/

LIB_INTERNAL OFILE *debug_fp(LOGMANAGER mp);
LIB_INTERNAL apr_file_t *debug_fd(LOGMANAGER mp);
LIB_INTERNAL void debug_close(LOGMANAGER mp);

/*----------------------------------------------*/
#endif	/* __DEBUG_H */
