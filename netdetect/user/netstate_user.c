#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/select.h>
#include <sys/time.h>
#include <unistd.h>
#include "netstate_user.h"

#define DEVICE_PATH "/dev/netstate"

int main(void)
{
    int fd;
    char buf[32];
    ssize_t ret;
	opt_register_script("/etc/init.d/mcproxy", STAT_UP, OPT_RELOAD);

	fd_set readfds;
	FD_ZERO(&readfds);

    fd = open(DEVICE_PATH, O_RDONLY);
    if (fd < 0) {
        perror("Failed to open device");
        return EXIT_FAILURE;
    }
	FD_SET(fd, &readfds);

    printf("Monitoring network state changes...\n");
    printf("Press Ctrl+C to exit\n");
    while (1) {
		int ret = select(fd + 1, &readfds, NULL, NULL, NULL);
		if(ret > 0) {
        	memset(buf, 0, sizeof(buf));
        	ret = read(fd, buf, sizeof(buf));
        	if (ret < 0) {
				perror("Read failed");
				close(fd);
				return EXIT_FAILURE;
        	}
        	printf("Network state change: %s\n", buf);
			if (strstr(buf, "eth0")) {
				if(strstr(buf, "UP")) {
					run_script(STAT_UP);
				} else {
					run_script(STAT_DOWN);
				}
			}
		}
    }
    close(fd);
    return EXIT_SUCCESS;
}

