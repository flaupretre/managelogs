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

#ifndef __LMGR_GLOBAL_H
#define __LMGR_GLOBAL_H

#include <apr.h>
#include <apr_file_io.h>
#include <apr_pools.h>

#if APR_HAVE_UNISTD_H
#include <unistd.h>
#endif

#if APR_HAVE_STDLIB_H
#include <stdlib.h>
#endif

/*----------------------------------------------*/

#ifdef IN_LMGR_LIB
#define LIB_INTERNAL static
#else
#define LIB_INTERNAL extern
#endif

/*-------------*/
/* Set to pure macro expansion because of cast problems on 64-bit */

#define FATAL_ERROR_STEP1	apr_file_t *__fd; \
							DECLARE_POOL(_my_tmp_pool); \
							(void)apr_file_open_stderr(&__fd \
								,CHECK_POOL(_my_tmp_pool)); \
							(void)apr_file_printf(__fd \
								,"*** ERROR: ");


#define FATAL_ERROR_STEP2	(void)apr_file_printf(__fd,"\n"); \
							(void)apr_file_close(__fd); \
							FREE_POOL(_my_tmp_pool); \
							exit(1);


#define FATAL_ERROR(_msg)	{ \
							FATAL_ERROR_STEP1 \
							(void)apr_file_printf(__fd,_msg); \
							FATAL_ERROR_STEP2 \
							}

#define FATAL_ERROR1(_msg,_arg1)	{ \
							FATAL_ERROR_STEP1 \
							(void)apr_file_printf(__fd,_msg,_arg1); \
							FATAL_ERROR_STEP2 \
							}

#define FATAL_ERROR2(_msg,_arg1,_arg2)	{ \
							FATAL_ERROR_STEP1 \
							(void)apr_file_printf(__fd,_msg,_arg1,_arg2); \
							FATAL_ERROR_STEP2 \
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

#define DECLARE_TPOOL	DECLARE_POOL(_tmp_pool)

#define CHECK_TPOOL() CHECK_POOL(_tmp_pool)

#define FREE_TPOOL()	FREE_POOL(_tmp_pool)

/*----------------------------------------------*/

typedef enum { NO, YES } BOOL;

/*----------------------------------------------*/
#endif	/* __LMGR_GLOBAL_H */
