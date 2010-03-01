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

#define DISPLAY_COUNT(_mp,_item)	{ \
	file_write_string(fp,#_item " count : ",YES); \
	(void)apr_snprintf(buf,sizeof(buf),"%d",STAT_COUNT_ITEM(_mp,_item)); \
	file_write_string_nl(fp,buf,YES); \
	}

/*----------------------------------------------*/

void logmanager_display_stats(LOGMANAGER *mp)
{
OFILE *fp;
char buf[32];

fp=debug_fp(mp);

file_write_string_nl(fp,"================== logmanager statistics ==================",YES);

file_write_string(fp,"Base path : ",YES);
file_write_string_nl(fp,mp->base_path,YES);

file_write_string(fp,"Compression ratio : ",YES);
(void)apr_snprintf(buf,sizeof(buf),"%lu",mp->compress.ratio);
file_write_string_nl(fp,buf,YES);

file_write_string_nl(fp,"",YES);

DISPLAY_COUNT(mp,write);
DISPLAY_COUNT(mp,write2);
DISPLAY_COUNT(mp,write3);
DISPLAY_COUNT(mp,flush);
DISPLAY_COUNT(mp,link);
DISPLAY_COUNT(mp,refresh_backup_links);
DISPLAY_COUNT(mp,refresh_active_link);
DISPLAY_COUNT(mp,new_active_file);
DISPLAY_COUNT(mp,rotate);
DISPLAY_COUNT(mp,remove_oldest);
DISPLAY_COUNT(mp,dump);
DISPLAY_COUNT(mp,sync);

file_write_string_nl(fp,"===========================================================",YES);
}

/*------------------------------------------------------------------------*/

