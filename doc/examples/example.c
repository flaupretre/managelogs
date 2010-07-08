/*===========================================================================
*
* This is a very basic example showing how to create a log manager, send it
* some data read from standard input, and then close and destroy it.
*
*============================================================================
*/

#include <stdio.h>
#include "logmanager.h"

/*---------*/

int main()
{
LOGMANAGER *mp;
LOGMANAGER_OPTIONS opts;
char *buf;
int nread;

/* First, initialize the opts struct */

memset(&opts,0,sizeof(opts));
opts.api_version=LOGMANAGER_API_VERSION; /* Mandatory */

/* Example : set the log manager to maintain a pid file a symbolic to the
* active log, limit individual file size to 1 Mb and global size to 10 Mb,
* and write the log files in '/some/dir' with a filename starting with
* 'prefix' */

opts.flags=LMGR_PID_FILE|LMGR_ACTIVE_LINK;
opts.file_maxsize=1024*1024;
opts.global_maxsize=10*opts.file_maxsize;
opts.base_path="/some/dir/prefix";

/* Create and open the log manager */

mp=new_logmanager(&opts);
logmanager_open(mp,NOW);

/* Here, in a real program, we would free the mem allocated for the opts struct */

/* Write loop */

while((nread=read(0,buf,sizeof(buf))) > 0)
	{
	logmanager_write(mp,buf,nread,NOW);
	}

/* The end: Close and destroy the log manager */

logmanager_close(mp);
logmanager_destroy(mp);

return 0;
}
