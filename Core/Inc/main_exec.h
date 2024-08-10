#ifndef MAIN_EXEC_H_
#define MAIN_EXEC_H_

#ifdef __cplusplus
extern "C"
{
#endif
#include "main.h"

    void setup();
    void resetDriver();
    void timerCallback();

#ifdef __cplusplus
};
#endif

const int pp = 14; // pole pairs

#endif // MAIN_EXEC_H_