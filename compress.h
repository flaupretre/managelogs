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
#include "options.h"

/*----------------------------------------------*/

typedef struct
	{
	char *name;
	char *suffix;
	void (*init_v1)(void *mp, LOGMANAGER_OPTIONS_V1 *opts);
	void (*destroy)(void *mp);
	void (*start)(void *mp);
	void (*end)(void *mp);
	apr_size_t (*predict_size)(void *mp, apr_size_t size);
	void (*compress_and_write)(void *mp, const char *buf
		,apr_size_t size);
	void (*flush)(void *mp);
	} COMPRESS_HANDLER;

/*----------------------------------------------*/

extern char *compress_handler_list(void);
extern BOOL init_compress_handler_from_arg(const char *arg);

/*----------------------------------------------*/
#endif	/* __COMPRESS_H */
