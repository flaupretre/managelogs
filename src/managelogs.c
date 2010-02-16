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

#if APR_HAVE_UNISTD_H
#include <unistd.h>
#endif

#if APR_HAVE_STDLIB_H
#include <stdlib.h>
#endif

#if APR_HAVE_STRING_H
#include <string.h>
#endif

#if APR_HAVE_STRINGS_H
#include <strings.h>
#endif

/*---------*/

#include <logmanager.h>

#include "../util/util.h"
#include "intr.h"
#include "options.h"

#include "../util/util.c"

/*----------------------------------------------*/

#define CHUNK_MAX	65536

/*----------------------------------------------*/

LOGMANAGER *mpp=(LOGMANAGER *)0;
int mgr_count;

TIMESTAMP timestamp=NOW;
int stats_toggle=0;
int refresh_only=0;

DECLARE_POOL(main_pool);

/*----------------------------------------------*/

void exit_proc(int status)
{
int i;

signal_shutdown();

if (mpp)
	{
	for (i=0;i<mgr_count;i++)
		{
		if (stats_toggle) logmanager_display_stats(mpp[i]);
		logmanager_destroy(mpp[i]);
		}
	}

mpp=allocate(mpp,0);

apr_terminate();	/* Includes main_pool free */

exit(status);
}

/*----------------------------------------------*/

int main (int argc, char * argv[])
{
apr_file_t *f_stdin;
apr_size_t nread,chunk_size,tmp_size;
char buf[CHUNK_MAX];
apr_status_t status;
LOGMANAGER_OPTIONS **opp;
int i;

apr_app_initialize(&argc, (char const * const **)(&argv), NULL);
intr_on();

/*-- Get options */

opp=get_options(argc,argv,&mgr_count);

/* Create and open the log managers */
/* Adapt read size if limit is small (better precision on rotation) */
/* '10' is an arbitrary choice, it could be another value */
/* Security : Chunk size cannot be lower than 100 bytes */

mpp=allocate(NULL,mgr_count*sizeof(*mpp));
chunk_size=CHUNK_MAX;
for (i=0;i<mgr_count;i++)
	{
	mpp[i]=new_logmanager(opp[i]);
	logmanager_open(mpp[i],timestamp);
	tmp_size=opp[i]->file_maxsize/10;
	if (tmp_size && (tmp_size < chunk_size)) chunk_size=tmp_size;
	}

free_options(opp,mgr_count); /* We don't need the options structs anymore */

if (refresh_only) exit_proc(0);

/* Open stdin for reading */

if (apr_file_open_stdin(&f_stdin,CHECK_POOL(main_pool)) != APR_SUCCESS)
	FATAL_ERROR("Cannot open stdin\n");

signal_init();

/* Main loop */

for (;;)
	{
	nread=chunk_size;
	status=apr_file_read(f_stdin, buf, &nread);
	if (status==APR_EOF) do_action(TERMINATE_ACTION);
	if (status != APR_SUCCESS) exit_proc(3);

	NOINTR_START();
	for (i=0;i<mgr_count;i++) logmanager_write(mpp[i],buf,nread,0,timestamp);
	NOINTR_END();
	check_and_run_pending_action();
	}

/* return prevents compiler warnings */

return 0;
}

/*----------------------------------------------*/
