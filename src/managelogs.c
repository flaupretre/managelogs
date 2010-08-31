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

#include "../config.h"

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

#if APR_HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif

/* Don't use APR_HAVE_SYS_STAT_H (not always correctly defined) */

#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif

/*---------*/

#define ALLOCATE

#include <logmanager.h>

#include "../common/global.h"
#include "managelogs.h"
#include "intr.h"
#include "options.h"

#include "../common/alloc.c"
#include "../common/convert.c"

/*----------------------------------------------*/

#define CHUNK_MAX	65536

/*----------------------------------------------*/

static BOOL input_is_fifo=NO;
static apr_file_t *f_input=(apr_file_t *)0;
static char input_buf[CHUNK_MAX];

DECLARE_POOL(main_pool);

/*----------------------------------------------*/

void destroy_log_managers()
{
LOGMANAGER *mp;
int i;

if (mgrs.count)
	{
	for (i=0;i<mgrs.count;i++)
		{
		mp=mgrs.items[i];
		if (stats_toggle) logmanager_display_stats(mp);
		logmanager_close(mp); /* Optional, but cleaner */
		logmanager_destroy(mp);
		}
	FREE_P(mgrs.items);
	mgrs.count=0;
	}
}

/*----------------------------------------------*/

void exit_proc(int status)
{
int i;

signal_shutdown();

destroy_log_managers();

if (f_input) (void)apr_file_close(f_input);
FREE_P(input_path);

FREE_POOL(main_pool);
apr_terminate();

exit(status);
}

/*----------------------------------------------*/

int main (int argc, char * argv[])
{
apr_size_t nread,chunk_size,tmp_size;
apr_status_t status;
LOGMANAGER_OPTIONS **opp;
int op_count,i;
apr_int32_t flags;
apr_finfo_t finfo;

(void)umask((mode_t)0); /* Clear file creation mask */

apr_app_initialize(&argc, (char const * const **)(&argv), NULL);
intr_on();

/*-- Get options */

opp=get_options(argc,argv,&op_count);

/* Create and open the log managers */
/* Adapt read size if limit is small (better precision on rotation) */
/* '10' is an arbitrary choice, it could be another value */
/* Security : Chunk size cannot be lower than 100 bytes */

chunk_size=CHUNK_MAX;
for (i=0;i<op_count;i++)
	{
	mgrs.count++;
	ALLOC_P(mgrs.items,(mgrs.count)*sizeof(LOGMANAGER *));
	
	mgrs.items[i]=new_logmanager(opp[i]);
	logmanager_open(mgrs.items[i],timestamp);
	tmp_size=opp[i]->file_maxsize/10;
	if (tmp_size && (tmp_size < chunk_size)) chunk_size=tmp_size;
	}

free_options(opp,op_count); /* We don't need the options structs anymore */

if (refresh_only) exit_proc(0);

signal_init();	/* Init signal handlers */

/* Main loop */

for (;;)
	{
	if (! f_input)
		{
		/* -- Open input and check for fifo */
		/* Should be done before entering the loop, but we need to do it */
		/* here because APR does not support non-blocking open flag, so, when */
		/* opening a fifo, the open() call blocks until a writer connects. */
		/* Another solution would be to manage input without APR but it would */
		/* be less portable. */

		if (input_path && strcmp(input_path,"stdin"))
			{
			flags=APR_READ;
			if (apr_stat(&finfo,input_path,APR_FINFO_TYPE,CHECK_POOL(main_pool))
				!=APR_SUCCESS)
				FATAL_ERROR1("Cannot get file type (%s)\n",input_path);
			if ((apr_filetype_e)(finfo.filetype) == APR_PIPE)
				{
				flags |= APR_WRITE; /* Workaround to EOF issue on fifos */
				input_is_fifo=YES;
				}
			(void)apr_file_open(&f_input,input_path,flags,0
				,CHECK_POOL(main_pool));
			if (!f_input) FATAL_ERROR1("Cannot open file (%s)",input_path);
			}
		else
			{
			(void)apr_file_open_stdin(&f_input,CHECK_POOL(main_pool));
			if (!f_input) FATAL_ERROR("Cannot open standard input for reading");
			}
		}

	nread=chunk_size;
	status=apr_file_read(f_input, input_buf, &nread);
	if (status==APR_EOF) do_action(TERMINATE_ACTION);
	if (status != APR_SUCCESS) exit_proc(3);

	NOINTR_START();
	for (i=0;i<mgrs.count;i++)
		logmanager_write(mgrs.items[i],input_buf,nread,timestamp);
	NOINTR_END();
	check_and_run_pending_action();
	}

/* return prevents compiler warnings */

return 0;
}

/*----------------------------------------------*/
