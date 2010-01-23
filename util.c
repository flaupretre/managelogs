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

#if APR_HAVE_STDLIB_H
#include <stdlib.h>
#endif

#if APR_HAVE_STDIO_H
#include <stdio.h>
#endif

#include "util.h"

/*----------------------------------------------*/

#define KILO	1024
#define MEGA	(KILO*KILO)
#define GIGA	(MEGA*KILO)

/*----------------------------------------------*/

int debug_toggle=0;

/*----------------------------------------------*/

void fatal_error_1(const char *msg, const char *arg)
{
fprintf(stderr,"*** Fatal Error : ");
fprintf(stderr,msg,arg);
fprintf(stderr,"\n");
exit(1);
}

/*----------------------------------------------*/

void *allocate(void *p, size_t size)
{
if (p)
	{
	if (size)
		{
		p=realloc(p,size);
		if (!p) FATAL_ERROR("realloc error");
		}
	else
		{
		free(p);
		p=NULL;
		}
	}
else
	{
	if (size)
		{
		p=malloc(size);
		if (!p) FATAL_ERROR("malloc error");
		}
	}
return p;
}	

/*----------------------------------------------*/

apr_off_t convert_size_string(const char *str)
{
char c;
apr_off_t result;

result=(apr_off_t)0;
while ((c=(*(str++)))!='\0')
	{
	if ((c=='k')||(c=='K')) return (result*KILO);
	if ((c=='m')||(c=='M')) return (result*MEGA);
	if ((c=='g')||(c=='G')) return (result*GIGA);
	if ((c<'0')||(c>'9')) return (apr_off_t)0;
	result = (result*10)+(apr_off_t)(c-'0');
	}
return result;
}

/*----------------------------------------------*/

extern void debug_on(void)
{
debug_toggle=1;
}

/*----------------------------------------------*/
