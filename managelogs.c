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

#include <apr_signal.h>

#include <logmanager.h>

#include "intr.h"
#include "options.h"
#include "lib/include/util.h"

/*----------------------------------------------*/

#define CHUNK_MAX	65536

/*----------------------------------------------*/

PRIVATE_POOL

LOGMANAGER *mp=(LOGMANAGER *)0;

char *cmd;

/*----------------------------------------------*/

static void shutdown_proc(void);

/*----------------------------------------------*/

static void shutdown_proc()
{
signal_shutdown();

if (mp) logmanager_destroy(mp,NOW);

apr_terminate();
}

/*----------------------------------------------*/

int main (int argc, char * argv[])
{
apr_file_t *f_stdin;
apr_size_t nread,chunk_size;
char buf[BUFSIZE];
apr_status_t status;
LOGMANAGER_OPTIONS_V1 *op;

cmd=argv[0];

apr_app_initialize(&argc, (char const * const **)(&argv), NULL);
intr_on();
(void)atexit(shutdown_proc);

/*-- Get options and arg */

op=get_options(argc,argv);

/* Create and open log manager */

mp=new_logmanager_v1(op,NOW);

logmanager_open(mp,NOW);

signal_init();

/* Open stdin for reading */

if (apr_file_open_stdin(&f_stdin,_POOL) != APR_SUCCESS)
	FATAL_ERROR("Cannot open stdin\n");

/* Adapt read size if limit is small (better precision on rotation) */
/* '10' is my choice, it could be another value */

chunk_size=CHUNK_MAX;
if (op->file_maxsize && (op->file_maxsize/10) < chunk_size)
	chunk_size=(op->file_maxsize/10);

/* Free options struct as we don't need it anymore */

free_options(op);

/* Loop forever */

for (;;)
	{
	nread=chunk_size;
	status=apr_file_read(f_stdin, buf, &nread);
	if (status==APR_EOF) do_action(TERMINATE_ACTION);
	if (status != APR_SUCCESS) exit(3);

	NOINTR_START();
	logmanager_write(mp,buf,nread,0,NOW);
	NOINTR_END();
	CHECK_EXEC_PENDING_ACTION();
	}

/* return prevents compiler warnings */

return 0;
}

/*----------------------------------------------*/
