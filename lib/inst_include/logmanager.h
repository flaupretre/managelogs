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

/* This file contains the log manager API */

#ifndef __LOGMANAGER_H
#define __LOGMANAGER_H

#include <apr.h>
#include <apr_file_io.h>
#include <apr_time.h>

#if APR_HAVE_STRING_H
#include <string.h>
#endif

/*----------------------------------------------*/

#ifndef NOW

/** Timestamp (apr_time_t value or 'NOW')
*
* The logmanager library functions receive their time information from the
* calling program, instead of getting it from the system clock. This way, the
* calling program controls the way time sent to a log manager varies,
* independantly from the 'real' (system) time. Different log managers can
* receive different timestamps as they are independant from each other.
*
* This allows, for instance, to process some old data with the time
* information extracted from each data line.
*
* @note Once a timestamp is provided in a function call, a subsequent call
* cannot provide a smaller timestamp value (no step back in time). If a
* timestamp value smaller than the previous one is provided, the previous one
* is used to ensure that the rotation/purge logic remains functional.
*/

typedef apr_time_t TIMESTAMP;

/** Use the system current time
*
* When a function receives 'NOW' as timestamp argument, it converts
* it into the current system time.
*/

#define NOW	(TIMESTAMP)0
#endif

/*===== LOGMANAGER option flags =====*/

/** LOGMANAGER_OPTIONS flag - Maintain a link to the active log file */

#define LMGR_ACTIVE_LINK	0x01	

/** LOGMANAGER_OPTIONS flag -  Maintain links to backup logs */

#define LMGR_BACKUP_LINKS	0x02

/** LOGMANAGER_OPTIONS flag -  Create hard links instead of symbolic links */

#define LMGR_HARD_LINKS		0x04

/** LOGMANAGER_OPTIONS flag -  Don't rotate on eol only
*
* This flag inhibits the default buffering mechanism ensuring that rotations
* occur on end of lines only (no truncated last line in log files)
 */

#define LMGR_IGNORE_EOL		0x08

/** LOGMANAGER_OPTIONS flag -  ignore 'no more space' (ENOSPC) system errors
*
* When this flag is set, the log manager ignores 'no more space/file system
* full' errors, silently discarding new data when no more space remains
* in the log file system.
*
* Before setting this flag, you must be sure that you prefer discarding data
* than crashing the service. Can be used as a 'last chance' protection against
* DOS attacks.
*
* @todo Should be able to send an alarm via an external system on ENOSPC
*       ignored errors.
*/

#define LMGR_IGNORE_ENOSPC	0x10

/** LOGMANAGER_OPTIONS flag -  Maintain a PID file as 'base_path'.pid */

#define LMGR_PID_FILE		0x20

/*----------------------------------------------*/

/** The API version */

#define LOGMANAGER_API_VERSION	3

/*----------------------------------------------*/
/** Log manager configuration provided at creation time as an argument
*   to new_logmanager(). */

typedef struct
	{
	
	unsigned int api_version;	/**< Must be set to LOGMANAGER_API_VERSION */
	char *base_path;			/**< The base path.
									 This is the prefix used to compute link, status,
									 and pid file paths. If log path not set,
									 also used to compute log file paths */
	unsigned int flags;			/**< An OR-ed combination of LMGR_* flags */
	struct
		{
		char *type;				/**< Compression type : 'gz' or 'bz2' */
		char *level;			/**< Compression level as a string.
									 Possible values depend on the compression
									 type */
		} compress;
	apr_off_t file_maxsize;		/**< Max log file individual size.
									 When returning from new_logmanager(), the
									 value can be modified if it does not fit
									 internal constraints.
									 Default = 0 = no limit */
	apr_off_t global_maxsize;	/**< Global size limit (sum).
									 Default = 0 = no limit */
	unsigned int keep_count;	/**< Maximum number of log files to keep.
									 Default = 0 = no limit */
	apr_fileperms_t create_mode;/**< File creation mode
									 @note mode is a unix-style permission
										expressed as an hexadecimal number
										(example: 0x644).
									 Default = 0 => 0x644 */
	char *debug_file;			/**< Path to debug file.
									 Default = NULL = no debug file */
	int debug_level;			/**< Debug level (>= 0).
									 Default = 0 = no debug output */
	char *rotate_cmd;			/**< Command to run on each rotation */
	/* API version 1 stops here */
	TIMESTAMP rotate_delay;		/**< Max delay between rotations (in seconds).
									 Default = 0 = not set */
	TIMESTAMP purge_delay;		/**< Max log file age (in seconds).
									 Default = 0 = not set */
	/* API version 2 stops here */
	char *log_path;				/**< If set, overrides base path to compute
									 log file paths.
									 Default (NULL) => use base path */
	/* API version 3 stops here */
	} LOGMANAGER_OPTIONS;

/*----------------------------------------------*/

#ifdef IN_LMGR_LIB
#include "../include/logmanager_int.h"
#else
/** The structure identifying a log manager
*
* This is an opaque structure whose elements are not directly accessible
* from the calling program */

typedef struct { int dummy; } LOGMANAGER; /* Opaque to client */
#endif

/*----------------------------------------------*/
/* Functions */

/** Creates a log manager.
*
* This function creates a log manager object from a set of options. The
* options define its logic and the paths it will maintain.
*
* @note This function does not write anything to the file system. It only
*		allocates and initializes a LOGMANAGER struct in memory. Before
*		sending data to the newly-created log manager, you must open it
*		via a call to logmanager_open().
*
* @param[in] opts A pointer to the log manager's options.
*		This struct and the strings/pointers it may contain should be
*		freed on return (every data needed by the log manager for internal
*		use are transparently allocated/freed by the log manager).
*
* @return LOGMANAGER* A pointer to a newly allocated LOGMANAGER struct.
*		This is the pointer to use in subsequent function calls.
*/

extern LOGMANAGER *new_logmanager(LOGMANAGER_OPTIONS *opts);

/*----------------------------------------------*/
/** Deletes a log manager.
*
* Also frees the data allocated by new_logmanager()
*
* @param[in] mp Pointer to a LOGMANAGER struct previously returned
*               by new_logmanager().
*/

extern void logmanager_destroy(LOGMANAGER *mp);

/*----------------------------------------------*/
/** Opens a log manager.
*
* Called after new_logmanager() before writing to a log manager
*
* @param[in] mp Pointer to a LOGMANAGER struct previously returned
*               by new_logmanager().
* @param[in] t Timestamp (apr_time_t value or 'NOW')
*/

extern void logmanager_open(LOGMANAGER *mp,TIMESTAMP t);

/*----------------------------------------------*/
/** Closes a log manager.
*
* The log manager can then be reopened via logmanager_open() or
* destroyed via logmanager_destroy().
*
* @param[in] mp Pointer to a LOGMANAGER struct previously returned
*               by new_logmanager().
*/

extern void logmanager_close(LOGMANAGER *mp);

/*----------------------------------------------*/
/** Sends data to a log manager.
*
* @param[in] mp Pointer to a LOGMANAGER struct previously returned
*               by new_logmanager().
* @param[in] buf The data to write.
* @param[in] size The size in bytes of the data buffer to write
* @param[in] t Timestamp (apr_time_t value or 'NOW')
*/

extern void logmanager_write(LOGMANAGER *mp, const char *buf, apr_off_t size
	, TIMESTAMP t);

/*----------------------------------------------*/
/** Forces a flush to the log file.
*
* Flushes the remaining data to the file system. Mostly interesting
* to flush compressed streams so that they can be uncompressed by
* an external mechanism.
*
* @note The log manager remains open.
*
* @param[in] mp Pointer to a LOGMANAGER struct previously returned
*               by new_logmanager().
*/

extern void logmanager_flush(LOGMANAGER *mp);

/*----------------------------------------------*/
/** Triggers a rotation.
*
* Triggers an immediate log rotation. Can also trigger a purge if the
* rotation causes the purge constraints to be exceeded.
*
* @param[in] mp Pointer to a LOGMANAGER struct previously returned
*               by new_logmanager().
* @param[in] t Timestamp (apr_time_t value or 'NOW')
*/

extern void logmanager_rotate(LOGMANAGER *mp,TIMESTAMP t);

/*----------------------------------------------*/
/** Returns the list of supported compression schemes.
*
* @return A dynamically allocated string containing the list of supported
* compression types, separated with ',' characters.
*
* The caller has the responsibility to free the returned string.
*/

extern char *logmanager_compression_list(void);

/*----------------------------------------------*/
/** Returns the version of the library.
*
* @return the version of the managelogs package the library belongs to.
*
* The caller has the responsibility to free the returned string.
*/

extern char *logmanager_version(void);

/*----------------------------------------------*/
/** Displays internal stats
*
* Utility function displaying some internal stats and counters. Used for
* debugging and tests only.
*
* @param[in] mp Pointer to a LOGMANAGER struct previously returned
*               by new_logmanager().
*/

extern void logmanager_display_stats(LOGMANAGER *mp);

/*----------------------------------------------*/
/** \example example.c */
#endif	/* __LOGMANAGER_H */
