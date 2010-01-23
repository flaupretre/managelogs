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

#ifndef __LOGFILE_H
#define __LOGFILE_H

#include <apr.h>

/*----------------------------------------------*/

typedef enum { CANNOT_ROTATE, CAN_ROTATE } rotate_flag;

/*----------------------------------------------*/

extern void logfile_init(const char *path,apr_off_t maxsize_arg,apr_fileperms_t mode);
extern void logfile_shutdown(void);
extern void logfile_write(const char *buf, apr_size_t size
	,rotate_flag can_rotate);

/*----------------------------------------------*/

#endif
