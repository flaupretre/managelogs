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

#ifndef __TIME_H
#define __TIME_H

#include <apr.h>

#if APR_HAVE_STDIO_H
#include <stdio.h>
#endif

#include <apr_time.h>

#include "util.h"

/*----------------------------------------------*/

typedef unsigned long TIMESTAMP;

#define NOW	(TIMESTAMP)0

/*----------------------------------------------*/

TIMESTAMP time_now(void);
TIMESTAMP strval_to_time(const char *val);

/*----------------------------------------------*/
#endif	/* __TIME_H */

