#ifndef APP_MAIN_H
#define APP_MAIN_H

#include "typedef.h"


struct app_info {
    int (*run)();
};


extern const struct app_info main_app_list[];















#endif
