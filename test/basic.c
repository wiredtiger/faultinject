#include <stdio.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

int main(int argc, char **argv)
{
	int test_fd;

	(void)argc;
	(void)argv;
	printf("Basic tester for faultinject DLL.\n");

	test_fd = open("/tmp/fault_inject_tester.txt", O_CREAT | O_WRONLY);
	if (test_fd == -1)
		printf("failed to open file: %d\n", errno);
	else
		printf("opened file\n");
	close(test_fd);
	test_fd = open("/tmp/fault_inject_tester.txt", O_CREAT | O_WRONLY);
	if (test_fd == -1)
		printf("failed to open file: %d\n", errno);
	else
		printf("opened file\n");
	close(test_fd);
	test_fd = open("/tmp/fault_inject_tester.txt", O_CREAT | O_WRONLY);
	if (test_fd == -1)
		printf("failed to open file: %d\n", errno);
	else
		printf("opened file\n");
	close(test_fd);
	return (0);
}
