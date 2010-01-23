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

#include <apr.h>
#include <apr_signal.h>

/*----------------------------------------------*/
/* Interrupt system - Delays signals until they can be handled, so that
  everything remains consistent */

#define NOINTR_END_DISCARD()	{ \
			intr_count--; \
			RESET_PENDING_ACTION(); \
			}

#define RESET_PENDING_ACTION()	{ _pending_action=0; }

/*----------------------------------------------*/

BOOL intr_count=0;

/*----------------------------------------------*/

static BOOL _intr_is_active=NO;
static ACTION _pending_action=NO_ACTION;

/*----------------------------------------------*/

void intr_on()
{
intr_count=0;
RESET_PENDING_ACTION();
_intr_is_active=YES;
}

/*----------------------------------------------*/

void intr_off()
{
_intr_is_active=NO;
}

/*----------------------------------------------*/

ACTION check_pending_action()
{
ACTION action;

action=0;

if (_intr_is_active && (intr_count==0))
	{
	NO_INTR_START();
	action=pending_action;
	NO_INTR_END_DISCARD();
	}

return action;
}

/*----------------------------------------------*/

void set_pending_action(ACTION action)
{
if (_intr_is_active && (action > _pending_action)) _pending_action=action;
}

/*----------------------------------------------*/
