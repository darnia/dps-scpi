#ifndef __LOG_H_
#define __LOG_H_

#ifdef WITH_SYSTEMD

#include "sd-helper.h"

#define log(format, args...) { \
    sd_journal_print(LOG_NOTICE, format, ##args); \
}

#define log_d(format, args...) { \
    sd_journal_print(LOG_DEBUG, format, ##args); \
}

#define log_e(format, args...) { \
    sd_journal_print(LOG_DEBUG, format, ##args); \
}

#else

#include <stdio.h>

#define log(format, args...) { \
    printf(format, ##args); \
}

#define log_d(format, args...) { \
    printf(format, ##args); \
}

#define log_e(format, args...) { \
    fprintf(stderr, format, ##args); \
}

#endif

#endif // __LOG_H_
