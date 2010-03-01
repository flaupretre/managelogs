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

/* This program generates pseudo-random CLF log lines for managelogs tests */

#include "../config.h"

#include <apr.h>
#include <apr_general.h>

/*---------*/

typedef struct
	{
	unsigned int count;
	char **strings;
	} STRING_TAB;

/*---------*/

char *rstring_tab(STRING_TAB *tp);
unsigned long rnum(unsigned long low,unsigned long high);

/*---------*/

char *agent_strings[]=
		{
		"Mozilla/5.0 (Windows; U; Windows NT 5.1; fr; rv:1.9.0.1) Gecko/2008070208 Firefox/3.0.1",
		"Opera/9.80 (Windows NT 5.1; U; fr) Presto/2.2.15 Version/10.01",
		"Mozilla/4.0 (compatible; MSIE 6.0; Windows NT 5.1; .NET CLR 1.1.4322)",
		"Mozilla/5.0 (Windows; U; Windows NT 5.1; fr; rv:1.9.1.5) Gecko/20091102 Firefox/3.5.5"
		};

STRING_TAB agents={
	4,
	agent_strings
	};

/*---------*/

char *rstring_tab(STRING_TAB *tp)
{
return tp->strings[rnum(0,tp->count -1)];
}

/*---------*/

unsigned long rnum(unsigned long low,unsigned long high)
{
unsigned char c;
unsigned long l,i,n,delta;

for (i=0,l=0;i<3;i++)
	{
	apr_generate_random_bytes(&c,1);
	l=((l << 8) | c);
	}

/*printf("%06X\n",l);*/

delta=high+1-low;
n=(l / (0x1000000 / delta)) + low;
if (n < low) n=low;
if (n > high) n=high;

return n;
}

/*---------*/

int main (int argc, char * argv[])
{
int i;

apr_app_initialize(&argc, (char const * const **)(&argv), NULL);

for (i=0;i<20;i++)
	{
	printf("%s\n",rstring_tab(&agents));
	}

apr_terminate();

return 0;
}

/*---------*/


















