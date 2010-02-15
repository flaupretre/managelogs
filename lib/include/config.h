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

#ifndef __LMGR_CONFIG_H
#define __LMGR_CONFIG_H

/*----------------------------------------------*/

#ifndef _WIN32
#define SYMLINK_SUPPORT
#endif

#define BUFSIZE 65536

/* Size limits. We refuse sizes lower than these ones.
 Special case: the value '1' means 'lower limit' */

#define GLOBAL_LOWER_LIMIT	(2*1024)

#define FILE_LOWER_LIMIT	(1024)

/* Default permissions */
/* Warning : APR flags are not the same as Unix perms. APR flags are set on
4 bits instead of 3, so we read the value as an hex value */

#define PIDFILE_MODE	0x0644

#define DEBUGFILE_MODE	0x0644

#define STATUSFILE_MODE	0x0644

/*----------------------------------------------*/
#endif	/* __LMGR_CONFIG_H */
