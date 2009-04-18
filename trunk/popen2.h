#ifndef CCONS_POPEN2_H
#define CCONS_POPEN2_H

//
// Header file for popen2.c, which declares the popen2() function as
// a bi-directional variant of popen() function.
//
// Part of ccons, the interactive console for the C programming language.
//
// Copyright (c) 2009 Alexei Svitkine. This file is distributed under the
// terms of MIT Open Source License. See file LICENSE for details.
//

#ifdef __cplusplus
extern "C" {
#endif

#include <unistd.h>

pid_t popen2(const char *command, int *infp, int *outfp);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // CCONS_POPEN2_H
