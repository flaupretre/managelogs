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

#ifndef __CONFIG_H
#define __CONFIG_H

#include <sys/types.h>
#include <sys/stat.h>

#include <apr.h>

#if APR_HAVE_UNISTD_H
#include <unistd.h>
#endif

#if APR_HAVE_STDLIB_H
#include <stdlib.h>
#endif

#if APR_HAVE_STRING_H
#include <string.h>
#endif

#if APR_HAVE_STRINGS_H
#include <strings.h>
#endif

/*----------------------------------------------*/

#define MANAGELOGS_VERSION	"1.0b1"

/*----------------------------------------------*/

#ifndef MAX_PATH
#define MAX_PATH		1024
#endif

#define BUFSIZE 65536

/* Default permissions for files that are created */
/* Warning : APR flags are not the same as Unix perms. APR flags are set on
4 bits instead of 3, so we read the value as an hex value */

#define LOGFILE_MODE	0x644

#define PIDFILE_MODE	0x0644

/*----------------------------------------------*/
#endif	/* __CONFIG_H */
