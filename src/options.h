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

#ifndef __MGL_OPTIONS_H
#define __MGL_OPTIONS_H

#include <apr.h>

#include "../config.h"

#include <logmanager.h>

/*----------------------------------------------*/

extern LOGMANAGER_OPTIONS **get_options(int argc, char **argv, int *countp);
extern void free_options(LOGMANAGER_OPTIONS **opp , int count);

/*----------------------------------------------*/
#endif	/* __MGL_OPTIONS_H */
