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

#include <apr.h>
#include <apr_signal.h>

#if APR_HAVE_STDLIB_H
#include <stdlib.h>
#endif

#include "util/util.h"
#include "intr.h"
#include "managelogs.h"

/*----------------------------------------------*/
/* Interrupt system - Delays signals until they can be handled, so that
  everything remains consistent */

#define NOINTR_END_DISCARD()	{ \
			intr_count--; \
			RESET_PENDING_ACTION(); \
			}

#define RESET_PENDING_ACTION()	{ _pending_action=NO_ACTION; }

#define ENABLE_INTR()	{ _intr_is_active=YES; }

#define DISABLE_INTR()	{ _intr_is_active=NO; }

/*----------------------------------------------*/

int intr_count=0;

/*----------------------------------------------*/

static BOOL _intr_is_active=NO;
static ACTION _pending_action=NO_ACTION;

/*----------------------------------------------*/

static void _signal_handler(int sig);


/*----------------------------------------------*/

void intr_on()
{
intr_count=0;
RESET_PENDING_ACTION();
ENABLE_INTR();
}

/*----------------------------------------------*/

void intr_off()
{
DISABLE_INTR();
}

/*----------------------------------------------*/

void check_and_run_pending_action()
{
if (_intr_is_active && (intr_count==0))
	{
	DISABLE_INTR();
	if (_pending_action!=NO_ACTION)
		{
		do_action(_pending_action);
		}
	RESET_PENDING_ACTION();
	ENABLE_INTR();
	}
}

/*----------------------------------------------*/

void set_pending_action(ACTION action)
{
if (_intr_is_active && (action > _pending_action)) _pending_action=action;
}

/*----------------------------------------------*/

static void _signal_handler(int sig)
{
switch(sig)
	{
#ifdef SIGUSR1
	case SIGUSR1:
		set_pending_action(ROTATE_ACTION);
		check_and_run_pending_action();
		break;
#endif

#ifdef SIGUSR2
	case SIGUSR2:
		set_pending_action(FLUSH_ACTION);
		check_and_run_pending_action();
		break;
#endif

#ifdef SIGTERM
		case SIGTERM:
#endif
#ifdef SIGINT
		case SIGINT:
#endif
#ifdef SIGQUIT
		case SIGQUIT:
#endif
#ifdef SIGTRAP
		case SIGTRAP:
#endif
#ifdef SIGABRT
		case SIGABRT:
#endif
#ifdef SIGURG
		case SIGURG:
#endif
		set_pending_action(TERMINATE_ACTION);
		check_and_run_pending_action();
		break;
	}
}

/*----------------------------------------------*/

void signal_init()
{
#ifdef SIGCHLD
(void)apr_signal(SIGCHLD,SIG_IGN);
#endif

#ifdef SIGUSR1
(void)apr_signal(SIGUSR1,_signal_handler);
#endif
#ifdef SIGUSR2
(void)apr_signal(SIGUSR2,_signal_handler);
#endif
#ifdef SIGTERM
(void)apr_signal(SIGTERM,_signal_handler);
#endif
#ifdef SIGINT
(void)apr_signal(SIGINT,_signal_handler);
#endif
#ifdef SIGQUIT
(void)apr_signal(SIGQUIT,_signal_handler);
#endif
#ifdef SIGTRAP
(void)apr_signal(SIGTRAP,_signal_handler);
#endif
#ifdef SIGABRT
(void)apr_signal(SIGABRT,_signal_handler);
#endif
#ifdef SIGURG
(void)apr_signal(SIGURG,_signal_handler);
#endif
}

/*----------------------------------------------*/

void signal_shutdown()
{
#ifdef SIGUSR1
(void)apr_signal(SIGUSR1,SIG_IGN);
#endif
#ifdef SIGUSR2
(void)apr_signal(SIGUSR2,SIG_IGN);
#endif
#ifdef SIGTERM
(void)apr_signal(SIGTERM,SIG_IGN);
#endif
#ifdef SIGINT
(void)apr_signal(SIGINT,SIG_IGN);
#endif
#ifdef SIGQUIT
(void)apr_signal(SIGQUIT,SIG_IGN);
#endif
#ifdef SIGTRAP
(void)apr_signal(SIGTRAP,SIG_IGN);
#endif
#ifdef SIGABRT
(void)apr_signal(SIGABRT,SIG_IGN);
#endif
#ifdef SIGURG
(void)apr_signal(SIGURG,SIG_IGN);
#endif
}

/*----------------------------------------------*/

void do_action(unsigned int action)
{
int i;

switch(action)
	{
	case FLUSH_ACTION:
		for (i=0;i<mgr_count;i++) logmanager_flush(mpp[i],timestamp);
		break;

	case ROTATE_ACTION:
		for (i=0;i<mgr_count;i++) logmanager_rotate(mpp[i],timestamp);
		break;

	case TERMINATE_ACTION:
		exit(0);
		break;
	}
}

/*----------------------------------------------*/
