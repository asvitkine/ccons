#ifndef CCONS_INTERNAL_COMMANDS_H
#define CCONS_INTERNAL_COMMANDS_H

//
// Header file for InternalCommands.cpp, which is used for processing built-in
// ccons commands.
//
// Part of ccons, the interactive console for the C programming language.
//
// Copyright (c) 2009 Alexei Svitkine. This file is distributed under the
// terms of MIT Open Source License. See file LICENSE for details.
//

#include <iostream>

namespace ccons {

bool HandleInternalCommand(const char *input, bool debugMode,
                           std::ostream& out, std::ostream& err);

} // namespace ccons

#endif // CCONS_INTERNAL_COMMANDS_H
