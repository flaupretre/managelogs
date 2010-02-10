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
/* Called when the manager is closed. Even if we don't have an EOL, we
* must write the buffer */

LIB_INTERNAL void write_end(LOGMANAGER mp, TIMESTAMP t)
{
write_level2(mp,mp->eol_buffer.buf,mp->eol_buffer.len,0,t);
mp->eol_buffer.buf=allocate(mp->eol_buffer.buf,mp->eol_buffer.len=0);
}

/*----------------------------------------------*/
/* Buffer output so that the files are cut on line boundaries ('\n' char) */

void logmanager_write(LOGMANAGER mp, const char *buf, apr_off_t size
	,unsigned int flags, TIMESTAMP t)
{
int i;

CHECK_MP(mp);
CHECK_TIME(mp,t);

DEBUG1(mp,2,"Starting logmanager_write (size=%lu)",(unsigned long)size);
INCR_STAT_COUNT(write);

if ((!buf) || (!size)) return;

if (mp->flags & LMGR_IGNORE_EOL)
	{
	write_level2(mp,buf,size,flags,t);
	return;
	}

#define APPEND_TO_EOL_BUF(_buf,_size) { \
	DEBUG1(mp,3,"Appending %lu bytes to eol buffer",(unsigned long)(_size)); \
	mp->eol_buffer.buf=allocate(mp->eol_buffer.buf \
		,mp->eol_buffer.len+_size); \
	memcpy(&(mp->eol_buffer.buf[mp->eol_buffer.len]),_buf,_size); \
	mp->eol_buffer.len += _size; \
	}

#define WRITE_EOL_BUF() { \
	write_level2(mp,mp->eol_buffer.buf,mp->eol_buffer.len,flags,t); \
	FREE_EOL_BUF(); \
	}

#define FREE_EOL_BUF() \
	mp->eol_buffer.buf=allocate(mp->eol_buffer.buf,mp->eol_buffer.len=0);

/* 1. If eol_buffer contains some data from a previous write, search a '\n'
from the beginning. If found, output the buffer and input data up to \n,
truncate data. If not found, append data to the buffer */

if (mp->eol_buffer.buf)
	{
	for (i=0;i<size;i++)
		{
		if (buf[i]=='\n')
			{
			/* Append to eol buf first, so that the whole line is sent to write_level2 in only one call */
			/* So, rotation can occur. If we did it with 2 calls, we should disable rotation for the */
			/* second call, and file size could be exceeded */
			APPEND_TO_EOL_BUF(buf,i+1);
			buf += (i+1);
			size -= (i+1);
			WRITE_EOL_BUF();
			break;
			}
		}

	if (mp->eol_buffer.buf) /* if not found, append to eol_buffer.buf */
		{
		APPEND_TO_EOL_BUF(buf,size);
		return;
		}
	}

/* 2. Search last \n. If found, move trailing data to eol_buffer and truncate.
If not found, put everything in eol_buffer and return.
When entering this section, eol_buffer is always empty. */

if (!size) return;

for (i=size-1;;i--)
	{
	if ((i<0) || (buf[i]=='\n'))
		{
		mp->eol_buffer.len=(size-i-1);
		if (mp->eol_buffer.len)
			{
			DEBUG1(mp,3,"Storing %lu bytes in eol buffer",(unsigned long)(mp->eol_buffer.len));
			size=i+1;
			mp->eol_buffer.buf=duplicate_mem(&(buf[size]),mp->eol_buffer.len);
			}
		break;
		}
	}

/* 2.If something remains, write it */

if (size) write_level2(mp,buf,size,flags,t);
}

/*----------------------------------------------*/

LIB_INTERNAL void write_level2(LOGMANAGER mp, const char *buf, apr_off_t size
	,unsigned int flags, TIMESTAMP t)
{
apr_off_t csize;

DEBUG1(mp,2,"Starting write_level2 (size=%lu)",(unsigned long)size);
INCR_STAT_COUNT(write2);

if ((!buf) || (size==0) || (!IS_OPEN(mp))) return;

csize=C_HANDLER1(mp,predict_size,size);

/*-- rotate/purge ? (before writing) */

if ((!(flags & LMGRW_CANNOT_ROTATE)) && should_rotate(mp,csize,t))
	{
	logmanager_rotate(mp,t); /* includes a purge */
	}
else purge_backup_files(mp,csize,t);

/*-- Write data */

C_VOID_HANDLER2(mp,compress_and_write,buf,size);

/*-- Update end timestamp */

mp->active.file->end=t;
}

/*------------------------------------------------------------------------*/
