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

#ifndef __UTIL_H
#define __UTIL_H

#include <apr.h>
#include <apr_file_io.h>


/*----------------------------------------------*/

#define FATAL_ERROR(_msg)		{ \
								fatal_error_2(_msg,NULL,NULL); \
								}

#define FATAL_ERROR1(_msg,_arg) { \
								fatal_error_2(_msg,(const char *)(_arg),NULL); \
								}

#define FATAL_ERROR2(_msg,_arg1,_arg2) { \
								fatal_error_2(_msg,(const char *)(_arg1) \
									,(const char *)(_arg2)); \
								}

/*-------------*/

#define DBG_PASS_LEVEL(_mp,_level) \
	((_mp)->debug.fp && (_level <= (_mp)->debug.level))

#define DEBUG(_mp,_level,_fmt)		{\
	if (DBG_PASS_LEVEL(_mp,_level))\
		(void)apr_file_printf((_mp)->debug.fp->fd,"> " _fmt "\n");\
	}

#define DEBUG1(_mp,_level,_fmt,_a1)		{\
	if (DBG_PASS_LEVEL(_mp,_level))\
		(void)apr_file_printf((_mp)->debug.fp->fd,"> " _fmt "\n",_a1);\
	}

#define DEBUG2(_mp,_level,_fmt,_a1,_a2)		{\
	if (DBG_PASS_LEVEL(_mp,_level))\
		(void)apr_file_printf((_mp)->debug.fp->fd,"> " _fmt "\n",_a1,_a2);\
	}

/*-------------*/

#ifdef _POOL
#undef _POOL
#endif

#define PRIVATE_POOL	static apr_pool_t *_private_pool=(apr_pool_t *)0;

#define _POOL (_private_pool ? _private_pool \
	: ((void)apr_pool_create(&_private_pool, NULL), _private_pool))

/*-------------*/

#define NEW_STRUCT(_type)	(_type *)allocate(NULL,sizeof(_type))

/*----------------------------------------------*/

typedef enum { NO, YES } BOOL;

/*----------------------------------------------*/

extern void *allocate(/*@null@*/ const void *p, size_t size);
extern void *duplicate(const char *string);
extern void fatal_error_2(const char *msg,/*@null@*/ const char *arg1,/*@null@*/ const char *arg2);
extern unsigned long strval_to_ulong(const char *val);
extern void *duplicate_mem(const void *,apr_size_t size);

/*----------------------------------------------*/
#endif	/* __UTIL_H */
