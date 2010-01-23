
#ifndef __LOGFILE_H
#define __LOGFILE_H

/*----------------------------------------------*/

#include "global.h"

/*----------------------------------------------*/

typedef enum { CANNOT_ROTATE, CAN_ROTATE } rotate_flag;

/*----------------------------------------------*/

extern void logfile_flush(void);
extern void logfile_init(char *path,int compress,int compress_level
	,apr_off_t maxsize);
extern void logfile_shutdown(void);
extern void logfile_rotate(void);
extern apr_off_t logfile_size(void);
extern void logfile_write_bin(char *buf, apr_size_t size, rotate_flag can_rotate);
extern void logfile_write(char *str);

/*----------------------------------------------*/

#endif
