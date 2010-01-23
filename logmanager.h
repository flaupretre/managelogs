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

#ifndef __LOGMANAGER_H
#define __LOGMANAGER_H

#include <apr.h>

#include "util.h"
#include "time.h"
#include "compress.h"
#include "options.h"

/*----------------------------------------------*/

/* logmanager_write() flags */

#define NO_ROTATE	0x1

/* LOGMANAGER struct flags */


/*----------------------------------------------*/

typedef struct
	{
	char *path;
	TIMESTAMP start;
	TIMESTAMP end;
	} LOGFILE;

typedef struct
	{
	int api_version;
	char *root_path;
	char *pid_path;
	char *status_path;
	unsigned int flags;
	struct
		{
		COMPRESS_HANDLER *handler;
		void *private;
		} compress;
	struct
		{
		OFILE *fp;
		LOGFILE *file;
		} active;
	struct
		{
		LOGFILE **files;
		int nb;
		apr_off_t size;
		} backups;
	apr_pool_t *pool;
	apr_off_t file_maxsize;
	apr_off_t global_maxsize;
	unsigned int keep_count;
	apr_fileperms_t create_mode;
	} LOGMANAGER;

/*----------------------------------------------*/

extern LOGMANAGER *new_logmanager_v1(LOGMANAGER_OPTIONS_V1 *opts,TIMESTAMP t);
extern void logmanager_open(LOGMANAGER *mp,TIMESTAMP t);
extern void logmanager_close(LOGMANAGER *mp);
extern void logmanager_destroy(LOGMANAGER *mp);
extern void logmanager_write(LOGMANAGER *mp, const char *buf, apr_size_t size
	,unsigned int flags, TIMESTAMP t);
extern void logmanager_flush(LOGMANAGER *mp);
extern void logmanager_rotate(LOGMANAGER *mp,TIMESTAMP t);

/*----------------------------------------------*/
#endif	/* __LOGMANAGER_H */
