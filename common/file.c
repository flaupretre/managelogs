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

#include <apr.h>
#include <apr_errno.h>
#include <apr_file_io.h>
#include <apr_file_info.h>

#if APR_HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif

/* Don't use APR_HAVE_SYS_STAT_H (not always correctly defined) */

#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif

#if APR_HAVE_STRING_H
#include <string.h>
#endif

/*----------------------------------------------*/

static OFILE *_new_ofile(const char *path);
static void _destroy_ofile(OFILE *fp);

/*----------------------------------------------*/
/* As blocks allocated in a pool cannot be freed independantly, each open */
/* file has its own pool */

static OFILE *_new_ofile(const char *path)
{
OFILE *fp;

fp=NEW_STRUCT(OFILE);
fp->pool=NULL_POOL;
fp->path=duplicate(path);
fp->fd=NULL;
fp->size=0;
return fp;
}

/*----------------------------------------------*/

static void _destroy_ofile(OFILE *fp)
{
FREE_POOL(fp->pool);
FREE_P(fp->path);
FREE_P(fp);
}

/*----------------------------------------------*/

LIB_INTERNAL BOOL file_exists(const char *path)
{
apr_finfo_t finfo;
apr_status_t status;
DECLARE_TPOOL;

status=apr_stat(&finfo,path,0,CHECK_TPOOL());

FREE_TPOOL();
return (status==APR_SUCCESS);
}

/*----------------------------------------------*/

LIB_INTERNAL BOOL file_rename(const char *oldpath,const char *newpath, BOOL fatal)
{
BOOL status;
DECLARE_TPOOL;

status=(BOOL)(apr_file_rename(oldpath,newpath,CHECK_TPOOL())==APR_SUCCESS);
if ((!status) && fatal)
	FATAL_ERROR2("Cannot rename file %s to %s",oldpath,newpath);

FREE_TPOOL();
return status;
}

/*----------------------------------------------*/
/* Deleting a non-existent file is NOT an error */

LIB_INTERNAL BOOL file_delete(const char *path, BOOL fatal)
{
apr_status_t status;
DECLARE_TPOOL;

status=apr_file_remove(path,CHECK_TPOOL());
if (fatal && (status!=APR_SUCCESS) && (status!=APR_ENOENT))
	FATAL_ERROR1("Cannot delete file (%s)",path);

FREE_TPOOL();
return (status==APR_SUCCESS);
}

/*----------------------------------------------*/

LIB_INTERNAL OFILE *file_create(const char *path, apr_int32_t mode)
{
OFILE *fp;

fp=_new_ofile(path);
apr_file_open(&(fp->fd),path,APR_WRITE|APR_CREATE|APR_TRUNCATE,mode
	,CHECK_POOL(fp->pool));
if (!(fp->fd))
	{
	_destroy_ofile(fp);
	FATAL_ERROR1("Cannot create file (%s)",path);
	}

return fp;
}

/*----------------------------------------------*/

LIB_INTERNAL apr_off_t file_size(const char *path)
{
apr_finfo_t finfo;
apr_off_t size;
DECLARE_TPOOL;

if (apr_stat(&finfo,path,APR_FINFO_SIZE,CHECK_TPOOL())!=APR_SUCCESS)
	FATAL_ERROR1("Cannot get file size (%s)\n",path);
size=(apr_off_t)(finfo.size);

FREE_TPOOL();
return size;
}

/*----------------------------------------------*/

LIB_INTERNAL apr_filetype_e file_type(const char *path)
{
apr_finfo_t finfo;
apr_filetype_e type;
DECLARE_TPOOL;

if (apr_stat(&finfo,path,APR_FINFO_TYPE,CHECK_TPOOL())!=APR_SUCCESS)
	FATAL_ERROR1("Cannot get file type (%s)\n",path);
type=(apr_filetype_e)(finfo.filetype);

FREE_TPOOL();
return type;
}

/*----------------------------------------------*/

LIB_INTERNAL OFILE *file_open_for_append(const char *path, apr_int32_t mode)
{
OFILE *fp;
apr_finfo_t finfo;

fp=_new_ofile(path);

if (!strcmp(path,"stdout"))
	{
	apr_file_open_stdout(&(fp->fd),CHECK_POOL(fp->pool));
	}
else
	{
	if (!strcmp(path,"stderr")) apr_file_open_stderr(&(fp->fd)
		,CHECK_POOL(fp->pool));
	else
		{
		(void)apr_file_open(&(fp->fd),path,APR_WRITE|APR_CREATE|APR_APPEND
			,mode,CHECK_POOL(fp->pool));

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

		fp->size=(apr_off_t)finfo.size;
		}
	}

return fp;
}

/*----------------------------------------------*/

LIB_INTERNAL void file_write(OFILE *fp, const char *buf, apr_off_t size
	,BOOL no_space_fatal)
{
apr_status_t status;

if (size==0) return;

if ((status=apr_file_write_full(fp->fd, buf, size, NULL))!=APR_SUCCESS)
	{
	size=0;	/* Written size */
	if (no_space_fatal || (! APR_STATUS_IS_ENOSPC(status)))
		{
		FATAL_ERROR2("Cannot write to file %s (errno=%d)",fp->path
			,apr_get_os_error());
		}
	}

fp->size += size;
}

/*----------------------------------------------*/

LIB_INTERNAL void file_write_string(OFILE *fp, const char *buf
	,BOOL no_space_fatal)
{
file_write(fp,buf,(apr_off_t)strlen(buf),no_space_fatal);
}

/*----------------------------------------------*/

LIB_INTERNAL void file_write_string_nl(OFILE *fp, const char *buf
	,BOOL no_space_fatal)
{
file_write_string(fp,buf,no_space_fatal);
file_write_string(fp,"\n",no_space_fatal);
}

/*----------------------------------------------*/

LIB_INTERNAL OFILE *file_close(OFILE *fp)
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
/* Returns a null-terminated buffer
if sizep is not null, return the data size (without trailing null) */

LIB_INTERNAL char *file_get_contents(const char *path, apr_off_t *sizep)
{
apr_file_t *fd;
apr_finfo_t finfo;
char *p;
apr_size_t size;
DECLARE_TPOOL;

(void)apr_file_open(&fd,path,APR_READ,0,CHECK_TPOOL());
if (!fd) FATAL_ERROR1("Cannot open file for reading (%s)",path);

if (apr_file_info_get(&finfo,APR_FINFO_SIZE,fd)!=APR_SUCCESS)
	FATAL_ERROR1("Cannot get file size (%s)\n",path);

p=allocate(NULL,finfo.size+1);
p[finfo.size]='\0';
if (sizep) (*sizep)=(apr_off_t)(finfo.size);

if (finfo.size)
	{
	size=(apr_size_t)(finfo.size);
	if (apr_file_read(fd,p,&size)!=APR_SUCCESS)
		FATAL_ERROR1("Cannot get file contents (%s)\n",path);
	}

(void)apr_file_close(fd);

FREE_TPOOL();
return p;
}

/*----------------------------------------------*/
