/*=============================================================================

Copyright F. Laupretre (francois@tekwire.net)

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

#ifndef __UTIL_H
#define __UTIL_H

#include <apr.h>

#include "config.h"

/*----------------------------------------------*/

#define FATAL_ERROR(_msg)		{ \
								fatal_error_2(_msg,NULL,NULL); \
								}

#define FATAL_ERROR_1(_msg,_arg) { \
								fatal_error_2(_msg,(const char *)_arg,NULL); \
								}

#define FATAL_ERROR_2(_msg,_arg1,_arg2) { \
								fatal_error_2(_msg,(const char *)_arg1 \
									,(const char *)_arg2); \
								}

#define DEBUG(_fmt)		{\
						if (debug_toggle) printf("> " _fmt "\n"); \
						}

#define DEBUG1(_fmt,_a1)		{\
						if (debug_toggle) printf("> " _fmt "\n",_a1); \
						}

#define DEBUG2(_fmt,_a1,_a2)		{\
						if (debug_toggle) printf("> " _fmt "\n",_a1,_a2); \
						}

typedef enum { NO, YES } BOOL;

/*----------------------------------------------*/

extern BOOL debug_toggle;

/*----------------------------------------------*/

extern void *allocate(/*@null@*/ const void *p, size_t size);
extern void *duplicate(const char *string);
extern void fatal_error_2(const char *msg,/*@null@*/ const char *arg1,/*@null@*/ const char *arg2);
extern apr_off_t convert_size_string(const char *str);
extern void set_debug(BOOL toggle);
extern void change_id(const char *string);

/*----------------------------------------------*/
#endif	/* __UTIL_H */
