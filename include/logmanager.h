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

/* The data exposed here is everything a program needs to use the logmanager
* library */

#ifndef __LOGMANAGER_H
#define __LOGMANAGER_H

#include <apr.h>
#include <apr_file_io.h>

/*----------------------------------------------*/

typedef unsigned long TIMESTAMP;

#define NOW	(TIMESTAMP)0

/*----------------------------------------------*/
/* logmanager_write() flags */

#define CANNOT_ROTATE	0x1
#define CAN_ROTATE	0x1

/*----------------------------------------------*/

typedef struct
	{
	int opaque;	/* Avoids warning on empty struct */
	} LOGMANAGER;

typedef struct
	{
	char *root_path;
	unsigned int flags;
	char *compress_string;
	apr_off_t file_maxsize;
	apr_off_t global_maxsize;
	unsigned int keep_count;
	apr_fileperms_t create_mode;
	} LOGMANAGER_OPTIONS_V1;

/*----------------------------------------------*/
/* Functions */

extern LOGMANAGER *new_logmanager_v1(LOGMANAGER_OPTIONS_V1 *opts,TIMESTAMP t);
extern void logmanager_destroy(LOGMANAGER *mp);
extern void logmanager_open(LOGMANAGER *mp,TIMESTAMP t);
extern void logmanager_close(LOGMANAGER *mp);
extern void logmanager_write(LOGMANAGER *mp, const char *buf, apr_off_t size
	,unsigned int flags, TIMESTAMP t);
extern void logmanager_flush(LOGMANAGER *mp);
extern void logmanager_rotate(LOGMANAGER *mp,TIMESTAMP t);
extern char *logmanager_compression_list(void);
extern char *logmanager_version(void);

/*----------------------------------------------*/
#endif	/* __LOGMANAGER_H */