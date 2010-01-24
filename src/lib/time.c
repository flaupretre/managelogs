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

#if APR_HAVE_STDIO_H
#include <stdio.h>
#endif

#include <apr_time.h>

/*----------------------------------------------*/

LIB_INTERNAL TIMESTAMP time_now()
{
return (TIMESTAMP)apr_time_sec(apr_time_now());
}

/*----------------------------------------------*/

LIB_INTERNAL TIMESTAMP strval_to_time(const char *val)
{
return (TIMESTAMP)strval_to_ulong(val);
}

/*----------------------------------------------*/
