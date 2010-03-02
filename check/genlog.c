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
#include <apr_time.h>

#if APR_HAVE_STDIO_H
#include <stdio.h>
#endif

#include "../common/global.h"
#include "../common/convert.h"

#include "../common/convert.c"

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

char *path_strings[]=
		{
		"root",
		"wiki",
		"run",
		"software",
		"web",
		"pilot",
		"wtf",
		"demo",
		"product",
		"analyze",
		"static",
		"dynamic",
		"php",
		"anything",
		"noideaanymore",
		"reallyboring"
		};

STRING_TAB paths={
	16,
	path_strings
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

char *ip_address()
{
static char buf[16];

sprintf(buf,"%d.%d.%d.%d",rnum(1,253),rnum(1,253),rnum(1,253),rnum(1,253));
return buf;
}

/*---------*/

char *agent_string()
{
return rstring_tab(&agents);
}

/*---------*/

char *path()
{
static char buf[256];

sprintf(buf,"/%s/%s/%s/%s_%c.htm"
	,rstring_tab(&paths)
	,rstring_tab(&paths)
	,rstring_tab(&paths)
	,rstring_tab(&paths)
	,rnum('a','z')
	);

return buf;
}

/*---------*/
/* Adapted from apache / mod_log_config.c */

char *date_string()
{
static apr_time_t request_time=0;
static char buf[32];
apr_time_exp_t xt;
char sign;
int timz;

request_time=(request_time ? (request_time+APR_USEC_PER_SEC) : apr_time_now());
apr_time_exp_lt(&xt,request_time);
timz = xt.tm_gmtoff;
if (timz < 0)
	{
	timz = -timz;
	sign = '-';
	}
else sign = '+';

apr_snprintf(buf, sizeof(buf),
			 "[%02d/%s/%d:%02d:%02d:%02d %c%.2d%.2d]",
			 xt.tm_mday, apr_month_snames[xt.tm_mon],
			 xt.tm_year+1900, xt.tm_hour, xt.tm_min, xt.tm_sec,
			 sign, timz / (60*60), (timz % (60*60)) / 60);

return buf;
}

/*---------*/

int main (int argc, char * argv[])
{
int i;
unsigned long size_limit,size;
char buf[1024],*p;

sscanf(argv[1],"%lu",&size_limit);

apr_app_initialize(&argc, (char const * const **)(&argv), NULL);

size=0;
while(1)
	{
	p=path();
	sprintf(buf,"%s - - [%s] \"GET  %s HTTP/1.1\" 200 %d \"http://www.fake.com%s\" \"%s\"\n"
		,ip_address()
		,date_string()
		,p
		,rnum(40,60000)
		,p
		,agent_string());
	printf("%s",buf);
	size += strlen(buf);
	if (size >= size_limit) break;
	}

apr_terminate();

return 0;
}

/*---------*/


















