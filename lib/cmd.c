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

/*----------------------------------------------*/
/* Run a command in background */

LIB_INTERNAL void run_bg_cmd(LOGMANAGER mp,char *cmd, LOGFILE *file,TIMESTAMP t)
{
char buf[32];
DECLARE_TPOOL;

if (!cmd) return; /* Should not happen, but... */

DEBUG1(mp,1,"Running rotate command : %s",cmd);

if (fork())		/* Parent returns */
	{
	FREE_TPOOL();
	return;
	}

/* Set environment variables */

(void)apr_env_set("LOGMANAGER_FILE_PATH",file->path,CHECK_TPOOL());
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
