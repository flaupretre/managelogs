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

#include <sys/types.h>
#include <sys/stat.h>

#include <apr.h>

#if APR_HAVE_STRING_H
#include <string.h>
#endif

#include <apr_file_io.h>
#include <apr_file_info.h>

#include "file.h"
#include "util.h"

/*----------------------------------------------*/

static apr_pool_t *pool;

/*----------------------------------------------*/

static OFILE *new_ofile(void);
static void destroy_ofile(OFILE *fp);

/*----------------------------------------------*/

static OFILE *new_ofile()
{
OFILE *fp;

fp=allocate(NULL,sizeof(OFILE));
memset(fp,0,sizeof(OFILE));
return fp;
}

/*----------------------------------------------*/

static void destroy_ofile(OFILE *fp)
{
(void)allocate(fp,0);
}

/*----------------------------------------------*/

void file_init()
{
apr_pool_create(&pool, NULL);
}

/*----------------------------------------------*/

int file_exists(const char *path)
{
apr_finfo_t finfo;

return (apr_stat(&finfo,path,0,pool)==APR_SUCCESS);
}

/*----------------------------------------------*/

int file_rename(const char *oldpath,const char *newpath)
{
return (apr_file_rename(oldpath,newpath,pool)==APR_SUCCESS);
}

/*----------------------------------------------*/

int file_delete(const char *path)
{
DEBUG1("Deleting file : %s",path);

return (apr_file_remove(path,pool)==APR_SUCCESS);
}

/*----------------------------------------------*/

OFILE *file_create(const char *path, apr_int32_t mode)
{
OFILE *fp;

DEBUG2("Creating/Truncating file : %s (%x)",path,mode);

fp=new_ofile();
fp->path=duplicate(path);
apr_file_open(&(fp->fd),path,APR_WRITE|APR_CREATE|APR_TRUNCATE,mode,pool);
if (!(fp->fd))
	{
	destroy_ofile(fp);
	FATAL_ERROR_1("Cannot create file (%s)",path);
	}

return fp;
}

/*----------------------------------------------*/

OFILE *file_open_for_append(const char *path, apr_int32_t mode)
{
OFILE *fp;
apr_finfo_t finfo;

DEBUG2("Opening/Appending file : %s (%x)",path,mode);

fp=new_ofile();
fp->path=duplicate(path);
apr_file_open(&(fp->fd),path,APR_WRITE|APR_CREATE|APR_APPEND,mode,pool);
if (!(fp->fd))
	{
	destroy_ofile(fp);
	FATAL_ERROR_1("Cannot open/append file (%s)",path);
	}

if (apr_file_info_get(&finfo,APR_FINFO_SIZE,fp->fd)!=APR_SUCCESS)
	{
	destroy_ofile(fp);
	FATAL_ERROR_1("Cannot get file size (%s)\n",path);
	}

fp->size=finfo.size;

return fp;
}

/*----------------------------------------------*/

void file_write(OFILE *fp, const char *buf, apr_size_t size)
{
apr_size_t nwrite;

if (!size) return;

nwrite=size;
(void)apr_file_write(fp->fd, buf, &nwrite);
if (nwrite!=size) FATAL_ERROR_1("Cannot write to file (%s)",fp->path);

fp->size += size;
}

/*----------------------------------------------*/

void file_write_string(OFILE *fp, const char *buf)
{
file_write(fp,buf,(apr_size_t)strlen(buf));
}

/*----------------------------------------------*/

void file_write_string_nl(OFILE *fp, const char *buf)
{
file_write_string(fp,buf);
file_write_string(fp,"\n");
}

/*----------------------------------------------*/

OFILE *file_close(OFILE *fp)
{
if (fp)
	{
	apr_file_close(fp->fd);
	destroy_ofile(fp);
	}

return (OFILE *)0;
}

/*----------------------------------------------*/
