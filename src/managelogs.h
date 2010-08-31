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

#ifndef __MANAGELOGS_H
#define __MANAGELOGS_H

#include "../config.h"

/*----------------------------------------------*/

#define MANAGELOGS_BANNER	"\
" PACKAGE_STRING "\n\
(C) 2008-2010 - Francois Laupretre <francois@tekwire.net>\n\
For more info: http://managelogs.tekwire.net\n\
Please report bugs to " PACKAGE_BUGREPORT "\n"

#define LOGFILE_MODE	0x644

/*----------------------------------------------*/

#ifdef ALLOCATE
#define GLOBAL
#else
#define GLOBAL extern
#endif

typedef struct
	{
	int count;
	LOGMANAGER **items;
	} LOGMANAGER_ARRAY;

/*----------------------------------------------*/

GLOBAL LOGMANAGER_ARRAY mgrs
#ifdef ALLOCATE
={ 0, (LOGMANAGER **)0 }
#endif
;

GLOBAL TIMESTAMP timestamp
#ifdef ALLOCATE
=NOW
#endif
;

GLOBAL int stats_toggle
#ifdef ALLOCATE
=NO
#endif
;

GLOBAL int refresh_only
#ifdef ALLOCATE
=NO
#endif
;

GLOBAL char *input_path
#ifdef ALLOCATE
=NULL
#endif
;

/*----------------------------------------------*/

extern void exit_proc(int status);

/*----------------------------------------------*/
#endif	/* __MANAGELOGS_H */
