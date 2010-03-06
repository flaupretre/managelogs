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

#ifndef __LMGR_ALLOC_H
#define __LMGR_ALLOC_H

#include <apr.h>

#if APR_HAVE_STDLIB_H
#include <stdlib.h>
#endif

/*-------------*/

#define NEW_STRUCT(_type)	(_type *)allocate(NULL,sizeof(_type))

#define ALLOC_P(_p,_sz)	(_p)=allocate(_p,_sz)

#define FREE_P(_p)	ALLOC_P(_p,0)

#define SET_P(_p,_sp)	{ \
	FREE_P(_p); \
	(_p)=(_sp); \
	}

#define DUP_P(_p,_sp)	SET_P(_p,duplicate(_sp))

/*----------------------------------------------*/

LIB_INTERNAL void *allocate(/*@null@*/ const void *p, apr_size_t size);
LIB_INTERNAL void *duplicate(const char *string);
LIB_INTERNAL void *duplicate_mem(const void *,apr_size_t size);
LIB_INTERNAL void *combine_strings(const char *str1,const char *str2);

/*----------------------------------------------*/
#endif	/* __LMGR_ALLOC_H */
