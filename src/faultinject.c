
/* Do not edit. Automatically generated from content in faultinject.c.in */
#include "config.h"

#undef _FILE_OFFSET_BITS
#undef __USE_FILE_OFFSET64
#undef _GNU_SOURCE
#define _GNU_SOURCE
#include <dirent.h>
#include <dlfcn.h>
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define	FAULT_INJECT_API	__attribute__ ((visibility("default")))

/*
 * Save references to the C library implementations of the functions we are
 * overriding.
 */
typedef int  (*libc_close_t)(int fd);
typedef int  (*libc_closedir_t)(DIR *dirp);
typedef int  (*libc_fsync_t)(int fd);
typedef int  (*libc_fdatasync_t)(int fd);
typedef int  (*libc_truncate_t)(const char *path, off_t length);
typedef int  (*libc_ftruncate_t)(int fd, off_t length);
typedef void * (*libc_mmap_t)(void *addr, size_t length, int prot, int flags, int fd, off_t offset);
typedef int  (*libc_munmap_t)(void *addr, size_t length);
typedef int  (*libc_open_t)(const char *pathname, int oflag,...);
typedef int  (*libc_open64_t)(const char *pathname, int oflag,...);
typedef DIR * (*libc_opendir_t)(const char *name);
typedef ssize_t  (*libc_pread_t)(int fd, void *buf, size_t count, off_t offset);
typedef ssize_t  (*libc_pwrite_t)(int fd, const void *buf, size_t count, off_t offset);
typedef struct dirent * (*libc_readdir_t)(DIR *dirp);
typedef int  (*libc_remove_t)(const char *pathname);
typedef int  (*libc_rename_t)(const char *oldpath, const char *newpath);

static libc_close_t libc_close = NULL;
static libc_closedir_t libc_closedir = NULL;
static libc_fsync_t libc_fsync = NULL;
static libc_fdatasync_t libc_fdatasync = NULL;
static libc_truncate_t libc_truncate = NULL;
static libc_ftruncate_t libc_ftruncate = NULL;
static libc_mmap_t libc_mmap = NULL;
static libc_munmap_t libc_munmap = NULL;
static libc_open_t libc_open = NULL;
static libc_open64_t libc_open64 = NULL;
static libc_opendir_t libc_opendir = NULL;
static libc_pread_t libc_pread = NULL;
static libc_pwrite_t libc_pwrite = NULL;
static libc_readdir_t libc_readdir = NULL;
static libc_remove_t libc_remove = NULL;
static libc_rename_t libc_rename = NULL;

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


	libc_close = (libc_close_t)(intptr_t)dlsym(RTLD_NEXT, "close");
	if (libc_close == NULL || dlerror()) {
		fprintf(stderr, "Error initializing close\n");
		_exit(1);
	}

	libc_closedir = (libc_closedir_t)(intptr_t)dlsym(RTLD_NEXT, "closedir");
	if (libc_closedir == NULL || dlerror()) {
		fprintf(stderr, "Error initializing closedir\n");
		_exit(1);
	}

	libc_fsync = (libc_fsync_t)(intptr_t)dlsym(RTLD_NEXT, "fsync");
	if (libc_fsync == NULL || dlerror()) {
		fprintf(stderr, "Error initializing fsync\n");
		_exit(1);
	}

	libc_fdatasync = (libc_fdatasync_t)(intptr_t)dlsym(RTLD_NEXT, "fdatasync");
	if (libc_fdatasync == NULL || dlerror()) {
		fprintf(stderr, "Error initializing fdatasync\n");
		_exit(1);
	}

	libc_truncate = (libc_truncate_t)(intptr_t)dlsym(RTLD_NEXT, "truncate");
	if (libc_truncate == NULL || dlerror()) {
		fprintf(stderr, "Error initializing truncate\n");
		_exit(1);
	}

	libc_ftruncate = (libc_ftruncate_t)(intptr_t)dlsym(RTLD_NEXT, "ftruncate");
	if (libc_ftruncate == NULL || dlerror()) {
		fprintf(stderr, "Error initializing ftruncate\n");
		_exit(1);
	}

	libc_mmap = (libc_mmap_t)(intptr_t)dlsym(RTLD_NEXT, "mmap");
	if (libc_mmap == NULL || dlerror()) {
		fprintf(stderr, "Error initializing mmap\n");
		_exit(1);
	}

	libc_munmap = (libc_munmap_t)(intptr_t)dlsym(RTLD_NEXT, "munmap");
	if (libc_munmap == NULL || dlerror()) {
		fprintf(stderr, "Error initializing munmap\n");
		_exit(1);
	}

	libc_open = (libc_open_t)(intptr_t)dlsym(RTLD_NEXT, "open");
	if (libc_open == NULL || dlerror()) {
		fprintf(stderr, "Error initializing open\n");
		_exit(1);
	}

	libc_open64 = (libc_open64_t)(intptr_t)dlsym(RTLD_NEXT, "open64");
	if (libc_open64 == NULL || dlerror()) {
		fprintf(stderr, "Error initializing open64\n");
		_exit(1);
	}

	libc_opendir = (libc_opendir_t)(intptr_t)dlsym(RTLD_NEXT, "opendir");
	if (libc_opendir == NULL || dlerror()) {
		fprintf(stderr, "Error initializing opendir\n");
		_exit(1);
	}

	libc_pread = (libc_pread_t)(intptr_t)dlsym(RTLD_NEXT, "pread");
	if (libc_pread == NULL || dlerror()) {
		fprintf(stderr, "Error initializing pread\n");
		_exit(1);
	}

	libc_pwrite = (libc_pwrite_t)(intptr_t)dlsym(RTLD_NEXT, "pwrite");
	if (libc_pwrite == NULL || dlerror()) {
		fprintf(stderr, "Error initializing pwrite\n");
		_exit(1);
	}

	libc_readdir = (libc_readdir_t)(intptr_t)dlsym(RTLD_NEXT, "readdir");
	if (libc_readdir == NULL || dlerror()) {
		fprintf(stderr, "Error initializing readdir\n");
		_exit(1);
	}

	libc_remove = (libc_remove_t)(intptr_t)dlsym(RTLD_NEXT, "remove");
	if (libc_remove == NULL || dlerror()) {
		fprintf(stderr, "Error initializing remove\n");
		_exit(1);
	}

	libc_rename = (libc_rename_t)(intptr_t)dlsym(RTLD_NEXT, "rename");
	if (libc_rename == NULL || dlerror()) {
		fprintf(stderr, "Error initializing rename\n");
		_exit(1);
	}

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

#ifdef HAVE_TRACE
/* We may want to make this conditional at runtime. */
static int faultinject_trace_operation(void)
{
	return (1);
}
#endif


FAULT_INJECT_API int  close(int fd)
{
	int ret;
	FILE *log_fd;


#ifdef HAVE_TRACE
	if (faultinject_trace_operation()) {
		/* Log the operation */
		log_fd = fopen("/tmp/faultinject_close.log", "a");
		if (log_fd != NULL) {
			fprintf(log_fd, "Intercepted call to: close(");
			
		fprintf(log_fd, "fd:%d, ", fd);
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
	return (*libc_close)(fd);
}
		
FAULT_INJECT_API int  closedir(DIR *dirp)
{
	int ret;
	FILE *log_fd;


#ifdef HAVE_TRACE
	if (faultinject_trace_operation()) {
		/* Log the operation */
		log_fd = fopen("/tmp/faultinject_closedir.log", "a");
		if (log_fd != NULL) {
			fprintf(log_fd, "Intercepted call to: closedir(");
			
		fprintf(log_fd, "dirp:DIR *, ");
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
	return (*libc_closedir)(dirp);
}
		
FAULT_INJECT_API int  fsync(int fd)
{
	int ret;
	FILE *log_fd;


#ifdef HAVE_TRACE
	if (faultinject_trace_operation()) {
		/* Log the operation */
		log_fd = fopen("/tmp/faultinject_fsync.log", "a");
		if (log_fd != NULL) {
			fprintf(log_fd, "Intercepted call to: fsync(");
			
		fprintf(log_fd, "fd:%d, ", fd);
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
	return (*libc_fsync)(fd);
}
		
FAULT_INJECT_API int  fdatasync(int fd)
{
	int ret;
	FILE *log_fd;


#ifdef HAVE_TRACE
	if (faultinject_trace_operation()) {
		/* Log the operation */
		log_fd = fopen("/tmp/faultinject_fdatasync.log", "a");
		if (log_fd != NULL) {
			fprintf(log_fd, "Intercepted call to: fdatasync(");
			
		fprintf(log_fd, "fd:%d, ", fd);
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
	return (*libc_fdatasync)(fd);
}
		
FAULT_INJECT_API int  truncate(const char *path, off_t length)
{
	int ret;
	FILE *log_fd;


#ifdef HAVE_TRACE
	if (faultinject_trace_operation()) {
		/* Log the operation */
		log_fd = fopen("/tmp/faultinject_truncate.log", "a");
		if (log_fd != NULL) {
			fprintf(log_fd, "Intercepted call to: truncate(");
			
		fprintf(log_fd, "path:%s, ", path);
		fprintf(log_fd, "length:off_t, ");
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
	return (*libc_truncate)(path, length);
}
		
FAULT_INJECT_API int  ftruncate(int fd, off_t length)
{
	int ret;
	FILE *log_fd;


#ifdef HAVE_TRACE
	if (faultinject_trace_operation()) {
		/* Log the operation */
		log_fd = fopen("/tmp/faultinject_ftruncate.log", "a");
		if (log_fd != NULL) {
			fprintf(log_fd, "Intercepted call to: ftruncate(");
			
		fprintf(log_fd, "fd:%d, ", fd);
		fprintf(log_fd, "length:off_t, ");
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
	return (*libc_ftruncate)(fd, length);
}
		
FAULT_INJECT_API void * mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset)
{
	int ret;
	FILE *log_fd;


#ifdef HAVE_TRACE
	if (faultinject_trace_operation()) {
		/* Log the operation */
		log_fd = fopen("/tmp/faultinject_mmap.log", "a");
		if (log_fd != NULL) {
			fprintf(log_fd, "Intercepted call to: mmap(");
			
		fprintf(log_fd, "addr:void *, ");
		fprintf(log_fd, "length:size_t, ");
		fprintf(log_fd, "prot:%d, ", prot);
		fprintf(log_fd, "flags:%d, ", flags);
		fprintf(log_fd, "fd:%d, ", fd);
		fprintf(log_fd, "offset:off_t, ");
			fprintf(log_fd, ");\n");
			fclose(log_fd);
		}
	}
#else
	log_fd = NULL; log_fd = log_fd;
#endif
	if ((ret = faultinject_fail_operation()) != 0) {
		errno = ret;
		return (NULL);
	}
	return (*libc_mmap)(addr, length, prot, flags, fd, offset);
}
		
FAULT_INJECT_API int  munmap(void *addr, size_t length)
{
	int ret;
	FILE *log_fd;


#ifdef HAVE_TRACE
	if (faultinject_trace_operation()) {
		/* Log the operation */
		log_fd = fopen("/tmp/faultinject_munmap.log", "a");
		if (log_fd != NULL) {
			fprintf(log_fd, "Intercepted call to: munmap(");
			
		fprintf(log_fd, "addr:void *, ");
		fprintf(log_fd, "length:size_t, ");
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
	return (*libc_munmap)(addr, length);
}
		
FAULT_INJECT_API int  open(const char *pathname, int oflag,...)
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
		
FAULT_INJECT_API int  open64(const char *pathname, int oflag,...)
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
		
FAULT_INJECT_API DIR * opendir(const char *name)
{
	int ret;
	FILE *log_fd;


#ifdef HAVE_TRACE
	if (faultinject_trace_operation()) {
		/* Log the operation */
		log_fd = fopen("/tmp/faultinject_opendir.log", "a");
		if (log_fd != NULL) {
			fprintf(log_fd, "Intercepted call to: opendir(");
			
		fprintf(log_fd, "name:%s, ", name);
			fprintf(log_fd, ");\n");
			fclose(log_fd);
		}
	}
#else
	log_fd = NULL; log_fd = log_fd;
#endif
	if ((ret = faultinject_fail_operation()) != 0) {
		errno = ret;
		return (NULL);
	}
	return (*libc_opendir)(name);
}
		
FAULT_INJECT_API ssize_t  pread(int fd, void *buf, size_t count, off_t offset)
{
	int ret;
	FILE *log_fd;


#ifdef HAVE_TRACE
	if (faultinject_trace_operation()) {
		/* Log the operation */
		log_fd = fopen("/tmp/faultinject_pread.log", "a");
		if (log_fd != NULL) {
			fprintf(log_fd, "Intercepted call to: pread(");
			
		fprintf(log_fd, "fd:%d, ", fd);
		fprintf(log_fd, "buf:void *, ");
		fprintf(log_fd, "count:size_t, ");
		fprintf(log_fd, "offset:off_t, ");
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
	return (*libc_pread)(fd, buf, count, offset);
}
		
FAULT_INJECT_API ssize_t  pwrite(int fd, const void *buf, size_t count, off_t offset)
{
	int ret;
	FILE *log_fd;


#ifdef HAVE_TRACE
	if (faultinject_trace_operation()) {
		/* Log the operation */
		log_fd = fopen("/tmp/faultinject_pwrite.log", "a");
		if (log_fd != NULL) {
			fprintf(log_fd, "Intercepted call to: pwrite(");
			
		fprintf(log_fd, "fd:%d, ", fd);
		fprintf(log_fd, "buf:const void *, ");
		fprintf(log_fd, "count:size_t, ");
		fprintf(log_fd, "offset:off_t, ");
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
	return (*libc_pwrite)(fd, buf, count, offset);
}
		
FAULT_INJECT_API struct dirent * readdir(DIR *dirp)
{
	int ret;
	FILE *log_fd;


#ifdef HAVE_TRACE
	if (faultinject_trace_operation()) {
		/* Log the operation */
		log_fd = fopen("/tmp/faultinject_readdir.log", "a");
		if (log_fd != NULL) {
			fprintf(log_fd, "Intercepted call to: readdir(");
			
		fprintf(log_fd, "dirp:DIR *, ");
			fprintf(log_fd, ");\n");
			fclose(log_fd);
		}
	}
#else
	log_fd = NULL; log_fd = log_fd;
#endif
	if ((ret = faultinject_fail_operation()) != 0) {
		errno = ret;
		return (NULL);
	}
	return (*libc_readdir)(dirp);
}
		
FAULT_INJECT_API int  remove(const char *pathname)
{
	int ret;
	FILE *log_fd;


#ifdef HAVE_TRACE
	if (faultinject_trace_operation()) {
		/* Log the operation */
		log_fd = fopen("/tmp/faultinject_remove.log", "a");
		if (log_fd != NULL) {
			fprintf(log_fd, "Intercepted call to: remove(");
			
		fprintf(log_fd, "pathname:%s, ", pathname);
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
	return (*libc_remove)(pathname);
}
		
FAULT_INJECT_API int  rename(const char *oldpath, const char *newpath)
{
	int ret;
	FILE *log_fd;


#ifdef HAVE_TRACE
	if (faultinject_trace_operation()) {
		/* Log the operation */
		log_fd = fopen("/tmp/faultinject_rename.log", "a");
		if (log_fd != NULL) {
			fprintf(log_fd, "Intercepted call to: rename(");
			
		fprintf(log_fd, "oldpath:%s, ", oldpath);
		fprintf(log_fd, "newpath:%s, ", newpath);
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
	return (*libc_rename)(oldpath, newpath);
}
		