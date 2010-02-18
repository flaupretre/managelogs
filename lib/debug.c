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

LIB_INTERNAL OFILE *debug_fp(LOGMANAGER *mp)
{
if (! mp->debug.fp) 
	{
	mp->debug.fp=file_open_for_append((mp->debug.path ? mp->debug.path
		: "stderr"),(apr_int32_t)DEBUGFILE_MODE);
	}

return mp->debug.fp;
}

/*----------------------------------------------*/

LIB_INTERNAL apr_file_t *debug_fd(LOGMANAGER *mp)
{
return debug_fp(mp)->fd;
}

/*----------------------------------------------*/

LIB_INTERNAL void debug_close(LOGMANAGER *mp)
{
if (mp->debug.fp) mp->debug.fp=file_close(mp->debug.fp);
}

/*----------------------------------------------*/
