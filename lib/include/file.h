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

#ifndef __FILE_H
#define __FILE_H

#include <apr.h>

/*----------------------------------------------*/

typedef struct
	{
	apr_pool_t *pool;
	apr_file_t *fd;
	const char *path;
	apr_size_t size;
	} OFILE;

/*----------------------------------------------*/

LIB_INTERNAL BOOL file_exists(const char *path);
LIB_INTERNAL BOOL file_rename(const char *oldpath,const char *newpath, BOOL fatal);
LIB_INTERNAL BOOL file_delete(const char *path, BOOL fatal);
LIB_INTERNAL OFILE *file_create(const char *path, apr_int32_t mode);
LIB_INTERNAL apr_size_t file_size(const char *path);
LIB_INTERNAL OFILE *file_open_for_append(const char *path, apr_int32_t mode);
LIB_INTERNAL void file_write(OFILE *fp, const char *buf, apr_size_t size, BOOL no_space_fatal);
LIB_INTERNAL void file_write_string(OFILE *fp, const char *buf, BOOL no_space_fatal);
LIB_INTERNAL void file_write_string_nl(OFILE *fp, const char *buf, BOOL no_space_fatal);
LIB_INTERNAL OFILE *file_close(OFILE *fp);
LIB_INTERNAL char *file_get_contents(const char *path, apr_off_t *sizep);

/*----------------------------------------------*/
#endif	/* __FILE_H */

