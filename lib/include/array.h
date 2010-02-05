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

#ifndef __ARRAY_H
#define __ARRAY_H

/*----------------------------------------------*/

#define ARRAY(_type) struct \
	{ \
	_type **items; \
	unsigned int count; \
	}

/*----------*/

#define ARRAY_COUNT(_a)		(_a).count
#define ARRAY_ITEMS(_a)		(_a).items

#define INDEX_IN_ARRAY(_a,_num)		(_num < ARRAY_COUNT(_a))

#define ARRAY_CLEAR(_a)		ARRAY_SET_COUNT(_a,0)

#define ARRAY_INCR_COUNT(_a)	ARRAY_SET_COUNT(_a,ARRAY_COUNT(_a)+1)

/* Don't have to protect this against _count < 0 as ARRAY_SET_COUNT is */

#define ARRAY_DECR_COUNT(_a)	ARRAY_SET_COUNT(_a,ARRAY_COUNT(_a)-1)

/*----------*/
/* Protect against negative count */

#define ARRAY_SET_COUNT(_a,_count) \
	{ \
	unsigned int _c; \
	_c=_count; \
	if (_c < 0 ) _c=0; \
	ARRAY_ITEMS(_a)=allocate(ARRAY_ITEMS(_a) \
		,_c * sizeof(*(ARRAY_ITEMS(_a)))); \
	ARRAY_COUNT(_a)=_c; \
	}

/*----------*/

#define ARRAY_SHIFT(_a)  \
	{ \
	ARRAY_INCR_COUNT(_a); \
	if (ARRAY_COUNT(_a) > 1) \
		{ \
		int _i; \
		for (_i=ARRAY_COUNT(_a)-1;_i>0;_i--) \
			ARRAY_ITEMS(_a)[_i]=ARRAY_ITEMS(_a)[_i-1]; \
		} \
	ARRAY_CLEAR_ITEM(_a,0); \
	}

/*----------*/

#define ARRAY_PACK(_a) \
	{ \
	int _offset,_i; \
	void **_ipp; \
 \
	if (ARRAY_COUNT(_a)) \
		{ \
		for (_i=0,_offset=0,_ipp=(void **)ARRAY_ITEMS(_a) \
			;_i < ARRAY_COUNT(_a);_i++,_ipp++) \
			{ \
			if (*_ipp) \
				{ \
				if (_offset) _ipp[-_offset]=(*_ipp); \
				} \
			else _offset++; \
			} \
		if (_offset) ARRAY_SET_COUNT(_a,ARRAY_COUNT(_a)-_offset); \
		} \
	}

/*----------*/

#define ARRAY_CLEAR_ITEM(_a,_num) \
	{ \
	if (INDEX_IN_ARRAY(_a,_num)) ARRAY_ITEMS(_a)[_num]=NULL; \
	}

/*----------------------------------------------*/
#endif	/* __ARRAY_H */
