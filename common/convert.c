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

#include "global.h"
#include "convert.h"

/*----------------------------------------------*/

LIB_INTERNAL unsigned long strval_to_ulong(const char *val)
{
unsigned long l;

if (sscanf(val,"%lu",&l)!=1)
	FATAL_ERROR1("Cannot read ulong numeric value (%s)",val);

return l;
}

/*----------------------------------------------*/

LIB_INTERNAL unsigned long hexval_to_ulong(const char *val)
{
unsigned long l;

if (sscanf(val,"%X",&l)!=1)
	FATAL_ERROR1("Cannot read hexadecimal value (%s)",val);

return l;
}

/*----------------------------------------------*/

LIB_INTERNAL apr_off_t strval_to_apr_off_t(const char *val)
{
apr_off_t l;

if (sscanf(val,"%" APR_OFF_T_FMT,&l)!=1)
	FATAL_ERROR1("Cannot read apr_off_t numeric value (%s)",val);

return l;
}

/*----------------------------------------------*/
