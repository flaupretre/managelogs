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

#include <sys/types.h>
#include <sys/stat.h>

#include <apr.h>

#if APR_HAVE_STRING_H
#include <string.h>
#endif

#include <apr_file_io.h>
#include <apr_file_info.h>

#include "include/file.h"
#include "include/util.h"

/*----------------------------------------------*/

PRIVATE_POOL

/*----------------------------------------------*/

static OFILE *_new_ofile(const char *path);
static void _destroy_ofile(OFILE *fp);
static void clear_umask(void);

/*----------------------------------------------*/

static OFILE *_new_ofile(const char *path)
{
OFILE *fp;

fp=allocate(NULL,sizeof(OFILE));
fp->path=duplicate(path);
fp->fd=NULL;
fp->size=0;
return fp;
}

/*----------------------------------------------*/

static void clear_umask()
{
(void)umask((mode_t)0);
}

/*----------------------------------------------*/

static void _destroy_ofile(OFILE *fp)
{
(void)allocate(fp->path,0);
(void)allocate(fp,0);
}

/*----------------------------------------------*/

BOOL file_exists(const char *path)
{
apr_finfo_t finfo;

return (apr_stat(&finfo,path,0,_POOL)==APR_SUCCESS);
}

/*----------------------------------------------*/

BOOL file_rename(const char *oldpath,const char *newpath, BOOL fatal)
{
BOOL status;

status=(BOOL)(apr_file_rename(oldpath,newpath,_POOL)==APR_SUCCESS);
if ((!status) && fatal)
	FATAL_ERROR2("Cannot rename file %s to %s",oldpath,newpath);

return status;
}

/*----------------------------------------------*/
/* Deleting a non-existent file is NOT an error */

BOOL file_delete(const char *path, BOOL fatal)
{
apr_status_t status;

status=apr_file_remove(path,_POOL);
if (fatal && (!(APR_STATUS_IS_SUCCESS(status) || APR_STATUS_IS_ENOENT(status))))
	FATAL_ERROR1("Cannot delete file (%s)",path);

return status;
}

/*----------------------------------------------*/

OFILE *file_create(const char *path, apr_int32_t mode)
{
OFILE *fp;

fp=_new_ofile(path);
clear_umask();
apr_file_open(&(fp->fd),path,APR_WRITE|APR_CREATE|APR_TRUNCATE,mode,_POOL);
if (!(fp->fd))
	{
	_destroy_ofile(fp);
	FATAL_ERROR1("Cannot create file (%s)",path);
	}

return fp;
}

/*----------------------------------------------*/

apr_size_t file_size(const char *path)
{
apr_finfo_t finfo;

if (apr_stat(&finfo,path,APR_FINFO_SIZE,_POOL)!=APR_SUCCESS)
	FATAL_ERROR1("Cannot get file size (%s)\n",path);

return (apr_size_t)finfo.size;
}

/*----------------------------------------------*/

OFILE *file_open_for_append(const char *path, apr_int32_t mode)
{
OFILE *fp;
apr_finfo_t finfo;

fp=_new_ofile(path);

if (!strcmp(path,"stdout")) apr_file_open_stdout(&(fp->fd),_POOL);
else
	{
	if (!strcmp(path,"stderr")) apr_file_open_stderr(&(fp->fd),_POOL);
	else
		{
		clear_umask();
		(void)apr_file_open(&(fp->fd),path,APR_WRITE|APR_CREATE|APR_APPEND
			,mode,_POOL);

		if (!(fp->fd))
			{
			_destroy_ofile(fp);
			FATAL_ERROR1("Cannot open/append file (%s)",path);
			}

		if (apr_file_info_get(&finfo,APR_FINFO_SIZE,fp->fd)!=APR_SUCCESS)
			{
			_destroy_ofile(fp);
			FATAL_ERROR1("Cannot get file size (%s)\n",path);
			}

		fp->size=(apr_size_t)finfo.size;
		}
	}

return fp;
}

/*----------------------------------------------*/

void file_write(OFILE *fp, const char *buf, apr_size_t size)
{
if (size==0) return;

if (apr_file_write_full(fp->fd, buf, size, NULL)!=APR_SUCCESS)
	{
	FATAL_ERROR1("Cannot write to file (%s)",fp->path);
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

OFILE *file_close(OFILE *fp)
{
if (fp)
	{
	if (strcmp(fp->path,"stdout") && strcmp(fp->path,"stderr"))
		(void)apr_file_close(fp->fd);

	_destroy_ofile(fp);
	}

return (OFILE *)0;
}

/*----------------------------------------------*/

char *file_get_contents(const char *path, apr_off_t *sizep)
{
apr_file_t *fd;
apr_finfo_t finfo;
char *p;
apr_size_t size;

(void)apr_file_open(&fd,path,APR_READ,0,_POOL);
if (!fd) FATAL_ERROR1("Cannot open file for reading (%s)",path);

if (apr_file_info_get(&finfo,APR_FINFO_SIZE,fd)!=APR_SUCCESS)
	FATAL_ERROR1("Cannot get file size (%s)\n",path);

p=allocate(NULL,finfo.size+1);
p[finfo.size]='\0';

if (finfo.size)
	{
	size=(apr_size_t)(finfo.size);
	if (apr_file_read(fd,p,&size)!=APR_SUCCESS)
		FATAL_ERROR1("Cannot get file contents (%s)\n",path);
	}

(void)apr_file_close(fd);

return p;
}

/*----------------------------------------------*/
