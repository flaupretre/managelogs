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

#ifndef __LMGR_COMPRESS_H
#define __LMGR_COMPRESS_H

#include <apr.h>

/*----------------------------------------------*/

typedef void (*WRITE_FUNC)(void *mp, const char *buf, apr_off_t size);

typedef struct
	{
	char *suffix;	/* Uncompressed : empty string */
	char *name;
	unsigned long default_ratio;
	void *(*init)(void *zp, const char *level, WRITE_FUNC write_func, void *write_arg);
	void (*destroy)(void *zp);
	void (*start)(void *zp);
	void (*stop)(void *zp);
	void (*compress_and_write)(void *zp, const char *buf
		,apr_off_t size);
	void (*flush)(void *zp);
	} COMPRESS_HANDLER;

/*-------------*/

#define C_HANDLER(_mp,_action) \
	(((_mp)->compress.handler->_action) \
		? (_mp)->compress.handler->_action(_mp)->compress.private : 0)

#define C_HANDLER1(_mp,_action,_arg1) \
	(((_mp)->compress.handler->_action) \
		? (_mp)->compress.handler->_action((_mp)->compress.private,_arg1) : 0)

#define C_HANDLER2(_mp,_action,_arg1,_arg2) \
	(((_mp)->compress.handler->_action) \
		? (_mp)->compress.handler->_action((_mp)->compress.private,_arg1,_arg2) : 0)

#define C_HANDLER3(_mp,_action,_arg1,_arg2,_arg3) \
	(((_mp)->compress.handler->_action) \
		? (_mp)->compress.handler->_action((_mp)->compress.private,_arg1,_arg2,_arg3) : 0)

#define C_VOID_HANDLER(_mp,_action) \
	if ((_mp)->compress.handler->_action) \
		(_mp)->compress.handler->_action((_mp)->compress.private);

#define C_VOID_HANDLER1(_mp,_action,_arg1) \
	if ((_mp)->compress.handler->_action) \
		(_mp)->compress.handler->_action((_mp)->compress.private,_arg1);

#define C_VOID_HANDLER2(_mp,_action,_arg1,_arg2) \
	if ((_mp)->compress.handler->_action) \
		(_mp)->compress.handler->_action((_mp)->compress.private,_arg1,_arg2);

#define C_VOID_HANDLER3(_mp,_action,_arg1,_arg2,_arg3) \
	if ((_mp)->compress.handler->_action) \
		(_mp)->compress.handler->_action((_mp)->compress.private,_arg1,_arg2,_arg3);

/*----------------------------------------------*/

LIB_INTERNAL COMPRESS_HANDLER *compress_handler_from_string(const char *type);
LIB_INTERNAL void compress_and_write(void *zp, const char *buf
	, apr_off_t size, TIMESTAMP t);

/*----------------------------------------------*/
#endif	/* __LMGR_COMPRESS_H */
