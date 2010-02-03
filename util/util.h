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

#ifdef IN_LMGR_LIB
#define LIB_INTERNAL static
#else
#define LIB_INTERNAL extern
#endif

/*-------------*/

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

#define POOL	apr_pool_t *

#define NULL_POOL	(POOL)0

#define DECLARE_POOL(_p)	POOL _p=NULL_POOL

#define NEW_POOL(_p)	((void)apr_pool_create(&_p, NULL), _p)

#define CHECK_POOL(_p) (_p ? _p : NEW_POOL(_p))

#define FREE_POOL(_p)	{ \
	if (_p) \
		{ \
		(void)apr_pool_destroy(_p); \
		_p=NULL_POOL; \
		} \
	}

#define DECLARE_TPOOL	DECLARE_POOL(_tmp_pool);

#define CHECK_TPOOL() CHECK_POOL(_tmp_pool)

#define FREE_TPOOL()	FREE_POOL(_tmp_pool)

/*-------------*/

#define NEW_STRUCT(_type)	(_type *)allocate(NULL,sizeof(_type))

/*----------------------------------------------*/

typedef enum { NO, YES } BOOL;

/*----------------------------------------------*/

LIB_INTERNAL void *allocate(/*@null@*/ const void *p, apr_size_t size);
LIB_INTERNAL void *duplicate(const char *string);
LIB_INTERNAL void fatal_error_2(const char *msg,/*@null@*/ const char *arg1,/*@null@*/ const char *arg2);
LIB_INTERNAL unsigned long strval_to_ulong(const char *val);
LIB_INTERNAL void *duplicate_mem(const void *,apr_size_t size);

/*----------------------------------------------*/
#endif	/* __UTIL_H */
