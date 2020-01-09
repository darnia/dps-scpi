#include "sd-helper.h"

void sd_pad_watchdog() {
    sd_notify(0, "WATCHDOG=1");
}

void sd_service_ready(const char *status) {
    char message[256];
    sprintf(message, "READY=1\nSTATUS=%s", status);
    sd_notify(0, message);
}

void sd_service_stopping(const char *status) {
    char message[256];
    sprintf(message, "STOPPING=1\nSTATUS=%s", status);
    sd_notify(0, message);
}