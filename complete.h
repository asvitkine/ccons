#ifndef CCONS_COMPLETE_H
#define CCONS_COMPLETE_H

//
// Header file for complete.c, which provides file system path auto-completion.
//
// Part of ccons, the interactive console for the C programming language.
//
// Copyright (c) 2009 Alexei Svitkine. This file is distributed under the
// terms of MIT Open Source License. See file LICENSE for details.
//

#ifdef __cplusplus
extern "C" {
#endif

unsigned complete(const char *path, char *suggest, unsigned suggest_length);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // CCONS_COMPLETE_H
