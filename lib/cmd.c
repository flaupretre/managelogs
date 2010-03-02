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

#include "apr_thread_proc.h"

/*----------------------------------------------*/
/* Run a command in background */
/* path and t may be null on entry */

LIB_INTERNAL void run_bg_cmd(LOGMANAGER *mp, const char *cmd
	, const char *path, TIMESTAMP t)
{
char buf[32];
DECLARE_TPOOL;
NORMALIZE_TIMESTAMP(t);

if (!cmd) return; /* Should not happen, but... */

DEBUG1(mp,1,"Running background command : %s",cmd);

if (fork())		/* Parent returns */
	{
	FREE_TPOOL();
	return;
	}

/* Detach process */
/* Don't use apr_proc_detach() as it chdir() to /, which is incompatible */
/* with the relative paths we can transmit to the child process */
/* Code extracted from apr_proc_detach() */

#ifdef HAVE_SETSID
if (setsid() == -1) exit(1);
#elif defined(NEXT) || defined(NEWSOS)
if (setpgrp(0, getpid()) == -1) exit(1);
#elif defined(OS2) || defined(TPF) || defined(MPE)
    /* do nothing */
#else
if (setpgid(0, 0) == -1) exit(1);
#endif

/* Set environment variables */

(void)apr_env_set("LOGMANAGER_FILE_PATH",(path ? path : ""),CHECK_TPOOL());
(void)apr_env_set("LOGMANAGER_BASE_PATH",mp->base_path,CHECK_TPOOL());
(void)apr_env_set("LOGMANAGER_ROOT_DIR",mp->root_dir,CHECK_TPOOL());
(void)apr_env_set("LOGMANAGER_COMPRESSION"
	,mp->compress.handler->suffix,CHECK_TPOOL());
(void)apr_env_set("LOGMANAGER_VERSION",PACKAGE_VERSION,CHECK_TPOOL());

(void)apr_snprintf(buf,sizeof(buf),"%lu",t);
(void)apr_env_set("LOGMANAGER_TIME",buf,CHECK_TPOOL());

/* Run command through shell */

(void)system(cmd);

exit(0);
}

/*----------------------------------------------*/
