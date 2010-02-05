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

#ifndef __COMPRESS_H
#define __COMPRESS_H

/*----------------------------------------------*/

typedef struct
	{
	char suffix[LMGR_COMPRESS_SIZE+1];	/* Uncompressed : empty string */
	void (*init_v1)(void *sp, const char *level);
	void (*destroy)(void *sp);
	void (*start)(void *sp);
	void (*end)(void *sp);
	apr_size_t (*predict_size)(void *sp, apr_size_t size);
	void (*compress_and_write)(void *sp, const char *buf
		,apr_size_t size);
	void (*flush)(void *sp);
	} COMPRESS_HANDLER;

/*-------------*/

#define C_HANDLER(_mp,_action) \
	(((_mp)->compress.handler->_action) \
		? (_mp)->compress.handler->_action(_mp) : 0)

#define C_HANDLER1(_mp,_action,_arg1) \
	(((_mp)->compress.handler->_action) \
		? (_mp)->compress.handler->_action((_mp),_arg1) : 0)

#define C_HANDLER2(_mp,_action,_arg1,_arg2) \
	(((_mp)->compress.handler->_action) \
		? (_mp)->compress.handler->_action((_mp),_arg1,_arg2) : 0)

/*----------------------------------------------*/

LIB_INTERNAL void init_compress_handler_from_string(void *sp, char *arg);

/*----------------------------------------------*/
#endif	/* __COMPRESS_H */
