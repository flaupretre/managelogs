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

#ifndef __COMPRESS_H
#define __COMPRESS_H

#include <apr.h>

#include "file.h"

/*----------------------------------------------*/

#define C_HANDLER(_action,_args)	{ \
								if (compress_handler->_action) \
									compress_handler->_action _args; \
								}

/*----------------------------------------------*/

typedef struct
	{
	char *name;
	/*@null@*/ char *suffix;
	/*@null@*/ void (*init)(/*@null@*/ const char *clevel);
	/*@null@*/ void (*start)(OFILE *fp);
	/*@null@*/ void (*end)();
	/*@null@*/ void (*predict_size)(apr_size_t *size);
	/*@null@*/ void (*compress_and_write)(const char *buf, apr_size_t size);
	} COMPRESS_HANDLER;

/*----------------------------------------------*/

extern COMPRESS_HANDLER *compress_handler;

/*----------------------------------------------*/

extern char *compress_handler_list(void);
extern BOOL init_compress_handler_from_arg(const char *arg);

/*----------------------------------------------*/
#endif	/* __COMPRESS_H */
