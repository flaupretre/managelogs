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

#ifndef __INTR_H
#define __INTR_H

#include <apr.h>

#include "util.h"

/*----------------------------------------------*/

typedef unsigned int ACTION;

#define NO_ACTION	(ACTION)0

#define NOINTR_START()	{ intr_count++; }

#define NOINTR_END()	{ intr_count--; }

/*----------------------------------------------*/

extern BOOL intr_count;

/*----------------------------------------------*/

extern void intr_on(void);
extern void intr_off(void);
extern ACTION check_pending_action();
extern void set_pending_action(ACTION action);

/*----------------------------------------------*/
#endif	/* __INTR_H */
