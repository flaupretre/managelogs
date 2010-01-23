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

static OFILE *new_ofile(const char *path);
static void destroy_ofile(OFILE *fp);

/*----------------------------------------------*/

static OFILE *new_ofile(const char *path)
{
OFILE *fp;

fp=allocate(NULL,sizeof(OFILE));
fp->path=duplicate(path);
fp->fd=NULL;
fp->size=0;

return fp;
}

/*----------------------------------------------*/

static void destroy_ofile(OFILE *fp)
{
(void)allocate(fp->path,0);
(void)allocate(fp,0);
}

/*----------------------------------------------*/

void file_init()
{
(void)apr_pool_create(&pool, NULL);
}

/*----------------------------------------------*/

BOOL file_exists(const char *path)
{
apr_finfo_t finfo;

return (apr_stat(&finfo,path,0,pool)==APR_SUCCESS);
}

/*----------------------------------------------*/

BOOL file_rename(const char *oldpath,const char *newpath, BOOL fatal)
{
BOOL status;

DEBUG2("Renaming file : %s to %s",oldpath,newpath);

status=(BOOL)(apr_file_rename(oldpath,newpath,pool)==APR_SUCCESS);
if ((!status) && fatal)
	FATAL_ERROR_2("Cannot rename file %s to %s",oldpath,newpath);

return status;
}

/*----------------------------------------------*/

BOOL file_delete(const char *path, BOOL fatal)
{
BOOL status;

DEBUG1("Deleting file : %s",path);

status=(apr_file_remove(path,pool)==APR_SUCCESS);
if ((!status) && fatal)
	FATAL_ERROR_1("Cannot delete file (%s)",path);

return status;
}

/*----------------------------------------------*/

OFILE *file_create(const char *path, apr_int32_t mode)
{
OFILE *fp;

DEBUG2("Creating/Truncating file : %s (mode %x)",path,(unsigned int)mode);

fp=new_ofile(path);
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

DEBUG2("Opening/Appending file : %s (mode %x)",path,mode);

fp=new_ofile(path);
(void)apr_file_open(&(fp->fd),path,APR_WRITE|APR_CREATE|APR_APPEND,mode,pool);
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

fp->size=(apr_size_t)finfo.size;

return fp;
}

/*----------------------------------------------*/

void file_write(OFILE *fp, const char *buf, apr_size_t size)
{
if (size==0) return;

DEBUG2("Writing %d bytes to %s",(int)size,fp->path);

if (apr_file_write_full(fp->fd, buf, size, NULL)!=APR_SUCCESS)
	{
	FATAL_ERROR_1("Cannot write to file (%s)",fp->path);
	}

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

/*@null@*/ OFILE *file_close(OFILE *fp)
{
DEBUG1("Closing file %s",fp->path);

if (fp)
	{
	(void)apr_file_close(fp->fd);
	destroy_ofile(fp);
	}

return (OFILE *)0;
}

/*----------------------------------------------*/
