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

#ifndef __FILE_H
#define __FILE_H

#include <apr.h>
#include <apr_file_io.h>

#include "util.h"

/*----------------------------------------------*/

typedef struct
	{
	apr_file_t *fd;
	const char *path;
	apr_size_t size;
	} OFILE;

/*----------------------------------------------*/

extern BOOL file_exists(const char *path);
extern BOOL file_rename(const char *oldpath,const char *newpath, BOOL fatal);
extern BOOL file_delete(const char *path, BOOL fatal);
extern OFILE *file_create(const char *path, apr_int32_t mode);
extern apr_size_t file_size(const char *path);
extern OFILE *file_open_for_append(const char *path, apr_int32_t mode);
extern void file_write(OFILE *fp, const char *buf, apr_size_t size);
extern void file_write_string(OFILE *fp, const char *buf);
extern void file_write_string_nl(OFILE *fp, const char *buf);
extern OFILE *file_close(OFILE *fp);

/*----------------------------------------------*/
#endif	/* __FILE_H */

