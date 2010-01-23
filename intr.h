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

#include <logmanager.h>

/*----------------------------------------------*/

typedef unsigned int ACTION;

#define NO_ACTION			(ACTION)0
#define FLUSH_ACTION		(ACTION)1
#define ROTATE_ACTION		(ACTION)2
#define TERMINATE_ACTION 	(ACTION)3

#define CHECK_EXEC_PENDING_ACTION()	{ \
	ACTION action; \
	if ((action=check_pending_action())!=NO_ACTION) do_action(action); \
	}

#define NOINTR_START()	{ intr_count++; }

#define NOINTR_END()	{ intr_count--; }

/*----------------------------------------------*/

extern int intr_count;
extern LOGMANAGER *mp;

/*----------------------------------------------*/

extern void intr_on(void);
extern void intr_off(void);
extern ACTION check_pending_action();
extern void set_pending_action(ACTION action);
extern void signal_init();
extern void signal_shutdown();
extern void do_action(ACTION action);

/*----------------------------------------------*/
#endif	/* __INTR_H */
