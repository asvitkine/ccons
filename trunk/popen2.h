#ifndef CCONS_POPEN2_H
#define CCONS_POPEN2_H

#ifdef __cplusplus
extern "C" {
#endif

#include <unistd.h>

pid_t popen2(const char *command, int *infp, int *outfp);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // CCONS_POPEN2_H
