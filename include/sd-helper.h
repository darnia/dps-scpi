#ifndef __SD_HELPER_H_
#define __SD_HELPER_H_

#include <stdio.h>
#include <systemd/sd-event.h>
#include <systemd/sd-daemon.h>
#include <systemd/sd-journal.h>

void sd_pad_watchdog();
void sd_service_ready(const char *status);
void sd_service_stopping(const char *status);

#endif // __SD_HELPER_H_