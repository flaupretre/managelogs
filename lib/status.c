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
/* Return absolute path of status file */

LIB_INTERNAL char *status_path(LOGMANAGER mp)
{
char *p;
int len;

p=allocate(NULL,len=(strlen(mp->base_path)+8));
(void)apr_snprintf(p,len,"%s.status",mp->base_path);

return p;
}

/*----------------------------------------------*/
/* NB: the order of backup files must be preserved from file to mem */

LIB_INTERNAL void get_status_from_file(LOGMANAGER mp)
{
char *buf,*p,*p2,*val;
LOGFILE *lp;

DEBUG1(mp,1,"Reading status from file (%s)",mp->status_path);

lp=(LOGFILE *)0; /* Just to remove a warning at compile time */

mp->active.fp=(OFILE *)0;
mp->active.file=(LOGFILE *)0;
ARRAY_CLEAR(mp->backup.files);
mp->backup.size=0;

if (file_exists(mp->status_path))
	{
	p=buf=file_get_contents(mp->status_path,NULL);
	while ((p2=strchr(p,'\n'))!=NULL)
		{
		(*p2)='\0';
		val=p+2;
		switch (*p)
			{
			case 'A':
				if (strval_to_ulong(val) < LOGMANAGER_STATUS_VERSION)
					FATAL_ERROR1("This status file was created by \
an old program version and cannot be read.\nPlease remove the \
status file (%s) and restart the program.",mp->status_path);
				break;

			case 'a':
				lp=NEW_LOGFILE();
				lp->path=ut_absolute_path(mp->root_dir,val);
				mp->active.file=lp;
				break;

			case 'b':
				lp=NEW_LOGFILE();
				lp->path=ut_absolute_path(mp->root_dir,val);
				ARRAY_INCR_COUNT(mp->backup.files);
				OLDEST_BACKUP_FILE(mp)=lp;
				break;

			case 'L':
				if (!lp) break;	/* Security against invalid file */
				lp->link=ut_absolute_path(mp->root_dir,val);
				break;

			case 'C':
				if (strcmp(val,mp->compress.handler->name))
					FATAL_ERROR2("Cannot continue from another compression engine (previous: %s ; new: %s)"
						,val,mp->compress.handler->name);
				break;

			case 's':
				if (!lp) break;	/* Security against invalid file */
				lp->start=strval_to_time(val);
				break;

			case 'e':
				if (!lp) break;	/* Security against invalid file */
				lp->end=strval_to_time(val);
				break;

			case 'i':
				if (!lp) break;	/* Security against invalid file */
				lp->size=strval_to_apr_off_t(val);
				break;

			case 'o':
				if (!lp) break;	/* Security against invalid file */
				lp->osize=strval_to_apr_off_t(val);
				break;
			/* Ignore other values */
			}
		p=p2+1;
		}

	FREE_P(buf);
	}
}

/*----------------------------------------------*/
/* Dump info about active and backup log files to a status file.
* File size is dumped as it allows to check for file corruption */

#define DUMP_FILE(_lp,_type)	{ \
	if (_lp) \
		{ \
		file_write_string(fp,_type " ",YES);	/* Path */ \
		file_write_string_nl(fp,ut_basename((_lp)->path),YES); \
		if ((_lp)->link) \
			{ \
			file_write_string(fp,"L ",YES);	/* Link */ \
			file_write_string_nl(fp,ut_basename((_lp)->link),YES); \
			} \
		file_write_string(fp,"s ",YES);			/* Start */ \
		(void)apr_snprintf(buf,sizeof(buf),"%lu",(_lp)->start); \
		file_write_string_nl(fp,buf,YES); \
		file_write_string(fp,"e ",YES);			/* End */ \
		(void)apr_snprintf(buf,sizeof(buf),"%lu",(_lp)->end); \
		file_write_string_nl(fp,buf,YES); \
		file_write_string(fp,"i ",YES);			/* Compressed size */ \
		(void)apr_snprintf(buf,sizeof(buf),"%" APR_OFF_T_FMT,(_lp)->size); \
		file_write_string_nl(fp,buf,YES); \
		file_write_string(fp,"o ",YES);			/* Uncompressed size */ \
		(void)apr_snprintf(buf,sizeof(buf),"%" APR_OFF_T_FMT,(_lp)->osize); \
		file_write_string_nl(fp,buf,YES); \
		} \
	}

LIB_INTERNAL void dump_status_to_file(LOGMANAGER mp)
{
OFILE *fp;
char buf[32];
unsigned int i;

DEBUG1(mp,1,"Writing status to file (%s)",mp->status_path);
INCR_STAT_COUNT(mp,dump);

fp=file_create(mp->status_path,(apr_int32_t)STATUSFILE_MODE);

file_write_string_nl(fp,"#==== Managelogs status data ===",YES);

file_write_string(fp,"A ",YES);
(void)apr_snprintf(buf,sizeof(buf),"%d",LOGMANAGER_STATUS_VERSION);
file_write_string_nl(fp,buf,YES);

file_write_string_nl(fp,"V " PACKAGE_VERSION,YES);

file_write_string(fp,"C ",YES); /* Compression type */
file_write_string_nl(fp,mp->compress.handler->name,YES);

DUMP_FILE(mp->active.file,"a");

if (BACKUP_COUNT(mp))
	{
	for (i=0;i<BACKUP_COUNT(mp);i++) DUMP_FILE(BACKUP_FILES(mp)[i],"b");
	}

(void)file_close(fp);
}

/*------------------------------------------------------------------------*/
