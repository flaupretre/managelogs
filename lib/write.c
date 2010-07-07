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

#define APPEND_TO_EOL_BUF(_buf,_size) { \
	DEBUG1(mp,3,"Appending %" APR_OFF_T_FMT " bytes to eol buffer",_size); \
	ALLOC_P(mp->eol_buffer.buf,mp->eol_buffer.len+_size); \
	memcpy(&(mp->eol_buffer.buf[mp->eol_buffer.len]),_buf,_size); \
	mp->eol_buffer.len += _size; \
	}

#define WRITE_EOL_BUF(_flags,_t) { \
	write_level2(mp,mp->eol_buffer.buf,mp->eol_buffer.len,_flags,_t); \
	FREE_EOL_BUF(); \
	}

#define FREE_EOL_BUF() { \
	mp->eol_buffer.len=0; \
	FREE_P(mp->eol_buffer.buf); \
	}

#define EOL_BUF_IS_EMPTY()	(mp->eol_buffer.buf == NULL)

/*-- logmanager_write() flags */

/** write_level2() flag - Disable rotation for this write operation
*
* When this flag is set, the write operation must not trigger a rotation,
* even if the rotate constraints (size, time, etc) are exceeded.
*
* Use with care as it causes the logmanager to ignore the rotation
* constraints set at creation time.
*/

#define LMGRW_CANNOT_ROTATE	0x1

/*----------------------------------------------*/
/* Called when the manager is closed. Even if we don't have an EOL, we
* must write the buffer */

LIB_INTERNAL void write_end(LOGMANAGER *mp)
{
WRITE_EOL_BUF(0,mp->last_timestamp);
}

/*----------------------------------------------*/
/* Buffer output so that the files are cut on line boundaries ('\n' char) */

void logmanager_write(LOGMANAGER *mp, const char *buf, apr_off_t size
	, TIMESTAMP t)
{
int i;

CHECK_MP(mp);
CHECK_TIMESTAMP(mp,t);

if (!IS_OPEN(mp)) logmanager_open(mp,t);

DEBUG1(mp,2,"Starting logmanager_write (size=%" APR_OFF_T_FMT ")",size);
INCR_STAT_COUNT(mp,write);

if ((!buf) || (!size)) return;

if (mp->flags & LMGR_IGNORE_EOL)
	{
	write_level2(mp,buf,size,0,t);
	return;
	}

/* 1. If eol_buffer contains some data from a previous write, search a '\n'
from the beginning. If found, output the buffer and input data up to \n,
truncate data. If not found, append data to the buffer */

if (! EOL_BUF_IS_EMPTY())
	{
	for (i=0;i<size;i++)
		{
		if (buf[i]=='\n')
			{
			/* Append to eol buf first, so that the whole line is sent to write_level2 in only one call */
			/* So, rotation can occur. If we did it with 2 calls, we should disable rotation for the */
			/* second call, and file size could be exceeded */
			APPEND_TO_EOL_BUF(buf,(apr_off_t)(i+1));
			buf += (i+1);
			size -= (i+1);
			WRITE_EOL_BUF(0,t);
			break;
			}
		}

	if (! EOL_BUF_IS_EMPTY()) /* if not found, append to eol_buffer.buf */
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
			DEBUG1(mp,3,"Storing %" APR_OFF_T_FMT " bytes in eol buffer"
				,mp->eol_buffer.len);
			size=i+1;
			mp->eol_buffer.buf=duplicate_mem(&(buf[size]),mp->eol_buffer.len);
			}
		break;
		}
	}

/* 2.If something remains, write it */

if (size) write_level2(mp,buf,size,0,t);
}

/*----------------------------------------------*/

LIB_INTERNAL void write_level2(LOGMANAGER *mp, const char *buf, apr_off_t size
	,unsigned int flags, TIMESTAMP t)
{
DEBUG1(mp,2,"Starting write_level2 (size=%" APR_OFF_T_FMT ")",size);
INCR_STAT_COUNT(mp,write2);

if ((!buf) || (size==0) || (!IS_OPEN(mp))) return;

/*-- rotate/purge ? (before writing) */

if ((!(flags & LMGRW_CANNOT_ROTATE)) && should_rotate(mp,size,t))
	{
	logmanager_rotate(mp,t); /* includes a purge */
	}
else purge_backup_files(mp,size,t);

compress_and_write(mp,buf,size,t);	/*-- Write data */
}

/*----------------------------------------------*/

LIB_INTERNAL void write_level3(void *_mp, const char *buf, apr_off_t size)
{
LOGMANAGER *mp=(LOGMANAGER *)_mp;

INCR_STAT_COUNT(mp,write3);

mp->active.file->sum=update_checksum(mp->active.file->sum,buf,size);
file_write(mp->active.fp,buf,size,!(mp->flags & LMGR_IGNORE_ENOSPC));
mp->active.file->size=mp->active.fp->size;
}

/*------------------------------------------------------------------------*/
