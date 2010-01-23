
#include "compress.h"
#include "plain.h"

/*----------------------------------------------*/

static void plain_compute_paths(char *logpath, char *oldpath);
static void plain_start(int level);
static void plain_end(void);
static void plain_predict_compressed_size(apr_size_t *size);
static void plain_compress_and_write(char *buf, apr_size_t size);

/*----------------------------------------------*/

COMPRESS_DEFS plain_compress_defs=
	{
	plain_compute_paths,
	plain_start,
	plain_end,
	plain_predict_compressed_size,
	plain_compress_and_write
	};

/*----------------------------------------------*/

static void plain_compute_paths(char *logpath, char *oldpath)
{
}

/*----------------------------------------------*/

static void plain_start(int level)
{
}

/*----------------------------------------------*/

static void plain_end()
{
}

/*----------------------------------------------*/

static void plain_predict_compressed_size(apr_size_t *sizep)
{
}

/*----------------------------------------------*/

static void plain_compress_and_write(char *buf, apr_size_t size)
{
logfile_write_bin_raw(buf,size);
}

/*----------------------------------------------*/







