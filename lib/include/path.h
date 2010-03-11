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

#ifndef __LMGR_PATH_H
#define __LMGR_PATH_H

#include <apr.h>
#include <apr_file_io.h>

/*----------------------------------------------*/

LIB_INTERNAL char *ut_dirname(const char *path);
LIB_INTERNAL const char *ut_basename(const char *path);
LIB_INTERNAL char *mk_abs_path(const char *base_dir, const char *path);
LIB_INTERNAL const char *get_rel_path(const char *base_dir, size_t base_dir_len
	, const char *path);

/*----------------------------------------------*/
#endif	/* __LMGR_PATH_H */
