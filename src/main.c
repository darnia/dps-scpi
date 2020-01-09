
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <netinet/in.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <arpa/inet.h>
#include <unistd.h>

#include "log.h"
#include "scpi-def.h"
#ifdef WITH_SYSTEMD
#include "sd-helper.h"
#endif

#define SCPI_PORT 5025

static char *serial_device = "/dev/ttyUSB0";
static int baudrate = B115200;
static bool verbose = false;

void print_usage(char *program) {
	fprintf(stderr, "Usage: %s [-v] [-d device] [-b baudrate]\n", program);
}

size_t SCPI_Write(scpi_t * context, const char * data, size_t len) {
    (void) context;

    if (context->user_context != NULL) {
        int fd = *(int *) (context->user_context);
        return write(fd, data, len);
    }
    return 0;
}

scpi_result_t SCPI_Flush(scpi_t * context) {
    (void) context;

    return SCPI_RES_OK;
}

int SCPI_Error(scpi_t * context, int_fast16_t err) {
    (void) context;
    /* BEEP */
    log_e("** Error: %d, \"%s\"\r\n", (int16_t) err, SCPI_ErrorTranslate(err));
    return 0;
}

scpi_result_t SCPI_Control(scpi_t * context, scpi_ctrl_name_t ctrl, scpi_reg_val_t val) {
    (void) context;

    if (SCPI_CTRL_SRQ == ctrl) {
        log_e("**SRQ: 0x%X (%d)\r\n", val, val);
    } else {
        log_e("**CTRL %02x: 0x%X (%d)\r\n", ctrl, val, val);
    }
    return SCPI_RES_OK;
}

scpi_result_t SCPI_Reset(scpi_t * context) {
    (void) context;
    log_d("**Reset\r\n");

    return SCPI_RES_OK;
}

scpi_result_t SCPI_SystemCommTcpipControlQ(scpi_t * context) {
    (void) context;

    return SCPI_RES_ERR;
}

static int createServer(int port) {
    int fd;
    int rc;
    int on = 1;
    struct sockaddr_in6 servaddr;

    /* Configure TCP Server */
    memset(&servaddr, 0, sizeof (servaddr));
    servaddr.sin6_family = AF_INET6;
    servaddr.sin6_addr = in6addr_any;
    servaddr.sin6_port = htons(port);

    /* Create socket */
    fd = socket(AF_INET6, SOCK_STREAM, 0);
    if (fd < 0) {
        perror("socket() failed");
        exit(-1);
    }

    /* Set address reuse enable */
    rc = setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (char *) &on, sizeof (on));
    if (rc < 0) {
        perror("setsockopt() failed");
        close(fd);
        exit(-1);
    }

    /* Bind to socket */
    rc = bind(fd, (struct sockaddr *) &servaddr, sizeof (servaddr));
    if (rc < 0) {
        perror("bind() failed");
        close(fd);
        exit(-1);
    }

    /* Listen on socket */
    listen(fd, 1);
    if (rc < 0) {
        perror("listen() failed");
        close(fd);
        exit(-1);
    }

    return fd;
}

static int waitServer(int fd) {
    fd_set fds;
    struct timeval timeout;
    int rc;
    int max_fd;

    FD_ZERO(&fds);
    max_fd = fd;
    FD_SET(fd, &fds);

    timeout.tv_sec = 5;
    timeout.tv_usec = 0;

    rc = select(max_fd + 1, &fds, NULL, NULL, &timeout);

    return rc;
}

/*
 *
 */
int main(int argc, char** argv) {
    (void) argc;
    (void) argv;
    int rc;
    int listenfd;
    char smbuffer[10];
	int opt;

	while ((opt = getopt(argc, argv, "b:d:v")) != -1) {
		switch(opt) {
			case 'b':
				baudrate = atoi(optarg);
				break;
			case 'd':
				serial_device = optarg;
				break;
			case 'v':
				verbose = true;
				break;
			default: 
				print_usage(argv[0]);
				exit(EXIT_FAILURE);
		}
	}

    rc = dps_init(serial_device, baudrate, verbose);
    if (rc < 0) {
        log_e("Failed to init DPS.\n");
        exit(-1);
    }

    /* user_context will be pointer to socket */
    scpi_context.user_context = NULL;

    SCPI_Init(&scpi_context,
            scpi_commands,
            &scpi_interface,
            scpi_units_def,
            SCPI_IDN1, SCPI_IDN2, SCPI_IDN3, SCPI_IDN4,
            scpi_input_buffer, SCPI_INPUT_BUFFER_LENGTH,
            scpi_error_queue_data, SCPI_ERROR_QUEUE_SIZE);

    listenfd = createServer(SCPI_PORT);
    log("Listening on port: %d\n", SCPI_PORT);
    log("Ready for client connect().\n");

#ifdef WITH_SYSTEMD
    sd_service_ready("Init completed, DPS SCPI daemon listening.");
#endif

    while (1) {
        int clifd;
        struct sockaddr_in6 cliaddr;
        socklen_t clilen;
        char str[INET6_ADDRSTRLEN];
        clilen = sizeof (cliaddr);

        rc = waitServer(listenfd);
        if (rc < 0) { /* failed */
            perror("  recv() failed");
            continue;
        }
        if (rc == 0) { /* timeout */
#ifdef WITH_SYSTEMD
            sd_pad_watchdog();
#endif
            continue;
        }
        clifd = accept(listenfd, NULL, NULL);
        if (clifd < 0) continue;

        getpeername(clifd, (struct sockaddr *)&cliaddr, &clilen);
        if(inet_ntop(AF_INET6, &cliaddr.sin6_addr, str, sizeof(str))) {
            log("Connection established from %s:%d\n", str, ntohs(cliaddr.sin6_port));
        }

        scpi_context.user_context = &clifd;

        while (1) {
            rc = waitServer(clifd);
            if (rc < 0) { /* failed */
                perror("  recv() failed");
                break;
            }
            if (rc == 0) { /* timeout */
#ifdef WITH_SYSTEMD
                sd_pad_watchdog();
#endif
                SCPI_Input(&scpi_context, NULL, 0);
            }
            if (rc > 0) { /* something to read */
                rc = recv(clifd, smbuffer, sizeof (smbuffer), 0);
                if (rc < 0) {
                    if (errno != EWOULDBLOCK) {
                        perror("  recv() failed");
                        break;
                    }
                } else if (rc == 0) {
                    log("Connection closed\r\n");
                    break;
                } else {
                    SCPI_Input(&scpi_context, smbuffer, rc);
                }
            }
        }

        close(clifd);
    }
#ifdef WITH_SYSTEMD
    sd_service_stopping("Stopped DPS SCPI daemon.");
#endif 

    return (EXIT_SUCCESS);
}

