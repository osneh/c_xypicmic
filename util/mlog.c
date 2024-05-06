#include "mlog.h"

FILE  *log_info_stream;
FILE  *log_warn_stream ;
FILE  *log_error_stream;

static void log_info_stream_construct (void) __attribute__((constructor(101)));
static void log_info_stream_construct (void) { log_info_stream = stdout; }

static void log_warn_stream_construct (void) __attribute__((constructor(101)));
static void log_warn_stream_construct (void) { log_warn_stream = stdout; }

static void log_error_stream_construct (void) __attribute__((constructor(101)));
static void log_error_stream_construct (void) { log_error_stream = stdout; }
