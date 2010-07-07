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
#include "apr_errno.h"

/*----------------------------------------------*/

LIB_INTERNAL void _proc_create_error(LOGMANAGER *mp, apr_pool_t *proc
	, apr_status_t err, const char *description)
{
DEBUG2(mp,1,"Cannot run background command. Code=%d, Reason=%s"
	,err,description);
}

/*----------------------------------------------*/
/* Using apr_proc_create() is not perfect but it is the only portable
 * way to run a child process.
 * Problems :
 * 
 * 	- Using a static variable can cause problems in multithreaded environments
 * 	  but I have not found another solution to know which log manager
 * 	  caused the error (using pool is not possible as it is an opaque struct).
 * 	- When the execv() fails, the callback is executed but it cannot write
 * 	  to its debug output anymore. So we also write the error message to
 * 	  stderr. Even _cur_mp->base_path is not accessible anymore !
 */

static LOGMANAGER *_cur_mp;

LIB_INTERNAL void _proc_create_error_callback(apr_pool_t *proc
	, apr_status_t err, const char *description)
{
fprintf(stderr,"Cannot run background command. Code=%d, Reason=%s\n"
	,err,description);

_proc_create_error(_cur_mp,proc,err,description);
}

/*----------------------------------------------*/
/* Run a command in background */
/* path and t may be null on entry */
/* Note: Background commands requires every paths to be absolute */
/* as detaching the child process causes the current directory */
/* to be changed to '/' */

LIB_INTERNAL void run_bg_cmd(LOGMANAGER *mp, const char *cmd
	, const char *file_path, TIMESTAMP t)
{
char buf[32];
DECLARE_TPOOL;
apr_procattr_t *attr;
apr_proc_t proc;
const char *args[2];
apr_status_t status;
char errbuf[1024];

NORMALIZE_TIMESTAMP(t);
if (!cmd) return; /* Security */

DEBUG1(mp,1,"Running background command : %s",cmd);

/* Set environment variables */

(void)apr_env_set("LOGMANAGER_FILE_PATH",(file_path ? file_path : "")
	,CHECK_TPOOL());
(void)apr_env_set("LOGMANAGER_BASE_PATH",mp->base_path,CHECK_TPOOL());
(void)apr_env_set("LOGMANAGER_BASE_DIR",mp->base_dir,CHECK_TPOOL());
(void)apr_env_set("LOGMANAGER_LOG_PATH",mp->log_path,CHECK_TPOOL());
(void)apr_env_set("LOGMANAGER_LOG_DIR",mp->log_dir,CHECK_TPOOL());
(void)apr_env_set("LOGMANAGER_COMPRESSION"
	,mp->compress.handler->suffix,CHECK_TPOOL());
(void)apr_env_set("LOGMANAGER_VERSION",PACKAGE_VERSION,CHECK_TPOOL());

(void)apr_snprintf(buf,sizeof(buf),"%" TIMESTAMP_FMT,t);
(void)apr_env_set("LOGMANAGER_TIME",buf,CHECK_TPOOL());

/* Run background process */

(void)apr_procattr_create(&attr,CHECK_TPOOL());

/* APR_NO_FILE is defined in APR 1.3.0 and higher */

#ifdef APR_NO_FILE
#define LMGR_IO_ATTR APR_NO_FILE
#else
#define LMGR_IO_ATTR 0
#endif

/*(void)apr_procattr_io_set(attr,LMGR_IO_ATTR,LMGR_IO_ATTR,LMGR_IO_ATTR);*/
(void)apr_procattr_cmdtype_set(attr,APR_PROGRAM_ENV);
(void)apr_procattr_detach_set(attr,1);
(void)apr_procattr_error_check_set(attr,1);
(void)apr_procattr_child_errfn_set(attr,_proc_create_error_callback);

args[0]=cmd;
args[1]=NULL;

status=apr_proc_create(&proc,cmd,args,NULL, attr,CHECK_TPOOL());
if (status != APR_SUCCESS)
	{
	_proc_create_error(mp,CHECK_TPOOL(),status
		,apr_strerror(status,errbuf,sizeof(errbuf)));
	}

/* The background process lives its own life. Don't care about it anymore */

FREE_TPOOL();
return;
}

/*----------------------------------------------*/
