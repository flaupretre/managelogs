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
/* Return absolute path of PID file */

LIB_INTERNAL char *pid_path(LOGMANAGER mp)
{
char *p;
int len;

p=allocate(NULL,len=(strlen(mp->base_path)+5));
snprintf(p,len,"%s.pid",mp->base_path);

return p;
}

/*----------------------------------------------*/

LIB_INTERNAL void create_pid_file(LOGMANAGER mp)
{
OFILE *fp;
char buf[32];
unsigned long pid;

if (! mp->pid_path) return;

pid=(unsigned long)getpid();
DEBUG2(mp,1,"Creating PID file (%s, pid=%lu)",mp->pid_path,pid);
DEBUG1(mp,2,"PPID=%lu",(unsigned long)getppid());

fp=file_create(mp->pid_path,(apr_int32_t)PIDFILE_MODE);

(void)snprintf(buf,sizeof(buf),"%lu",pid);
file_write_string_nl(fp,buf,YES);

(void)file_close(fp);
}

/*----------------------------------------------*/
/* Note: Don't remove the pid file if it has been overwritten by another 
log manager (happens with error_log when apache starts) */

LIB_INTERNAL void remove_pid_file(LOGMANAGER mp)
{
char *buf;
unsigned long pid;

if (! mp->pid_path) return;

DEBUG1(mp,1,"Removing PID file(%s)",mp->pid_path);

if (!file_exists(mp->pid_path)) return; /* Should not happen but, in case... */

buf=file_get_contents(mp->pid_path,NULL);
pid=0;
(void)sscanf(buf,"%lu",&pid);
(void)allocate(buf,0);

if (pid == (unsigned long)getpid()) (void)file_delete(mp->pid_path,NO);
}

/*------------------------------------------------------------------------*/
