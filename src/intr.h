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

#ifndef __MGL_INTR_H
#define __MGL_INTR_H

#include <apr.h>

#include "../config.h"

#include <logmanager.h>

/*----------------------------------------------*/

typedef unsigned int ACTION;

#define NO_ACTION			(ACTION)0
#define FLUSH_ACTION		(ACTION)1
#define ROTATE_ACTION		(ACTION)2
#define TERMINATE_ACTION 	(ACTION)3

#define NOINTR_START()	{ intr_count++; }

#define NOINTR_END()	{ intr_count--; }

/*----------------------------------------------*/

extern int intr_count;

/*----------------------------------------------*/

extern void intr_on(void);
extern void intr_off(void);
extern void check_and_run_pending_action(void);
extern void set_pending_action(ACTION action);
extern void signal_init(void);
extern void signal_shutdown(void);
extern void do_action(ACTION action);

/*----------------------------------------------*/
#endif	/* __MGL_INTR_H */
