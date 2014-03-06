
/* Do not edit. Automatically generated from content in faultinject.c.in */
#include "config.h"

#undef _FILE_OFFSET_BITS
#undef __USE_FILE_OFFSET64
#undef _GNU_SOURCE
#define _GNU_SOURCE
#include <dlfcn.h>
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define	FAULT_INJECT_API	__attribute__ ((visibility("default")))

/*
 * Save references to the C library implementations of the functions we are
 * overriding.
 */
typedef int (*libc_open_t)(const char *pathname, int oflag,...);
typedef int (*libc_open64_t)(const char *pathname, int oflag,...);

static libc_open_t libc_open = NULL;
static libc_open64_t libc_open64 = NULL;

static uint64_t g_op_count = 0;
static uint64_t g_max_op_count = 0;

void __attribute__ ((constructor)) faultinject_constructor(void);

/*
 * Setup pointers to the C library version of functions we are overriding,
 * and initialize any global variables.
 */
void __attribute__ ((constructor)) faultinject_constructor(void)
{
	char *env_string;


	libc_open = (libc_open_t)(intptr_t)dlsym(RTLD_NEXT, "open");
	if (libc_open == NULL || dlerror())
		_exit(1);

	libc_open64 = (libc_open64_t)(intptr_t)dlsym(RTLD_NEXT, "open64");
	if (libc_open64 == NULL || dlerror())
		_exit(1);

	if ((env_string = getenv("FAULTINJECT_FAIL_COUNT")) != NULL &&
	    strlen(env_string) != 0) {
		errno = 0;
		g_max_op_count = strtoull(env_string, NULL, 10);
		printf("Set max op count to: %d\n", (int)g_max_op_count);
		if (errno != 0) {
			fprintf(stderr, "Failed to parse environment variable\n");
			g_max_op_count = 0;
		}
	}
}

static int faultinject_fail_operation(void)
{
	if (!libc_open)
		faultinject_constructor();


	printf("failing with op count: %d\n", (int)g_op_count);
	if (g_max_op_count > 0 && ++g_op_count > g_max_op_count)
		return (EFAULT);
	return (0);
}

/* We may want to make this conditional at runtime. */
static int faultinject_trace_operation(void)
{
	return (1);
}


int FAULT_INJECT_API open(const char *pathname, int oflag,...)
{
	int ret;
	FILE *log_fd;


	va_list ap;
	mode_t mode;

	va_start(ap, oflag);
#if SIZEOF_MODE_T < SIZEOF_INT
	mode = (mode_t)va_arg(ap, int);
#else
	mode = va_arg(ap, mode_t);
#endif
	va_end(ap);

	/*
	 * In pthread environments the dlsym call may call our open, we simply
	 * ignore it because libc is already loaded.
	 */
	if (!libc_open) {
		errno = EFAULT;
		return (-1);
	}

#ifdef HAVE_TRACE
	if (faultinject_trace_operation()) {
		/* Log the operation */
		log_fd = fopen("/tmp/faultinject_open.log", "a");
		if (log_fd != NULL) {
			fprintf(log_fd, "Intercepted call to: open(");
			
		fprintf(log_fd, "pathname:%s, ", pathname);
		fprintf(log_fd, "oflag:%d, ", oflag);
		fprintf(log_fd, "...");
			fprintf(log_fd, ");\n");
			fclose(log_fd);
		}
	}
#else
	log_fd = NULL; log_fd = log_fd;
#endif
	if ((ret = faultinject_fail_operation()) != 0) {
		errno = ret;
		return (-1);
	}
	return (*libc_open)(pathname, oflag, mode);
}
		
int FAULT_INJECT_API open64(const char *pathname, int oflag,...)
{
	int ret;
	FILE *log_fd;


	va_list ap;
	mode_t mode;

	va_start(ap, oflag);
#if SIZEOF_MODE_T < SIZEOF_INT
	mode = (mode_t)va_arg(ap, int);
#else
	mode = va_arg(ap, mode_t);
#endif
	va_end(ap);

	/*
	 * In pthread environments the dlsym call may call our open, we simply
	 * ignore it because libc is already loaded.
	 */
	if (!libc_open64) {
		errno = EFAULT;
		return (-1);
	}

#ifdef HAVE_TRACE
	if (faultinject_trace_operation()) {
		/* Log the operation */
		log_fd = fopen("/tmp/faultinject_open64.log", "a");
		if (log_fd != NULL) {
			fprintf(log_fd, "Intercepted call to: open64(");
			
		fprintf(log_fd, "pathname:%s, ", pathname);
		fprintf(log_fd, "oflag:%d, ", oflag);
		fprintf(log_fd, "...");
			fprintf(log_fd, ");\n");
			fclose(log_fd);
		}
	}
#else
	log_fd = NULL; log_fd = log_fd;
#endif
	if ((ret = faultinject_fail_operation()) != 0) {
		errno = ret;
		return (-1);
	}
	return (*libc_open64)(pathname, oflag, mode);
}
		