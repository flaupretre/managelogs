
#ifndef __GLOBALS_H
#define __GLOBALS_H

#include <apr.h>

#if APR_HAVE_UNISTD_H
#include <unistd.h>
#endif
#if APR_HAVE_STDLIB_H
#include <stdlib.h>
#endif
#if APR_HAVE_STRING_H
#include <string.h>
#endif
#if APR_HAVE_STRINGS_H
#include <strings.h>
#endif

#ifndef MAX_PATH
#define MAX_PATH		1024
#endif

#endif
