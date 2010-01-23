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

#ifndef __COMPRESS_H
#define __COMPRESS_H

#include <apr.h>

/*----------------------------------------------*/

extern void logfile_write_bin_raw(char *buf, apr_size_t size);

/*----------------------------------------------*/

typedef struct
	{
	void (*compute_paths)(char *logpath, char *oldpath);
	void (*start)(int level);
	void (*end)();
	void (*predict_compressed_size)(apr_size_t *size);
	void (*compress_and_write)(char *buf, apr_size_t size);
	} COMPRESS_DEFS;

/*----------------------------------------------*/
#endif
