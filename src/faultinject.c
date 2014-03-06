
/* Do not edit. Automatically generated from content in faultinject.c.in */

#undef _FILE_OFFSET_BITS
#undef __USE_FILE_OFFSET64
#include <errno.h>
#include <fcntl.h>
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

void __attribute__ ((constructor)) faultinject_constructor(void);

/*
 * Setup pointers to the C library version of functions we are overriding.
 */
void __attribute__ ((constructor)) faultinject_constructor(void)
{

	libc_open = (libc_open_t)(intptr_t)dlsym(RTLD_NEXT, open);
	if (libc_open == NULL || dlerror())
		_exit(1);

	libc_open64 = (libc_open64_t)(intptr_t)dlsym(RTLD_NEXT, open64);
	if (libc_open64 == NULL || dlerror())
		_exit(1);
}

static int faultinject_fail_operation(void)
{
	if (!libc_open)
		faultinject_constructor();

	return (0);
}


int FAULT_INJECT_API open(const char *pathname, int oflag,...)
{
	int ret;

	va_list ap;
	mode_t mode;

	va_start(ap, flags);
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

	if ((ret = faultinject_fail_operation()) != 0) {
		errno = ret;
		return (ret);
	}
	return (*libc_open(pathname, oflag, mode));
}
		
int FAULT_INJECT_API open64(const char *pathname, int oflag,...)
{
	int ret;

	va_list ap;
	mode_t mode;

	va_start(ap, flags);
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

	if ((ret = faultinject_fail_operation()) != 0) {
		errno = ret;
		return (ret);
	}
	return (*libc_open64(pathname, oflag, mode));
}
		
