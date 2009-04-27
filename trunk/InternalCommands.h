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

// Handle an internal command if it was specified. If handled, returns
// true; otherwise the input did not correspond to an internal command.
bool HandleInternalCommand(const char *input, bool debugMode,
                           std::ostream& out, std::ostream& err);

// Prints ccons version information to the specified ostream.
void PrintVersionInformation(std::ostream& out);

// Prints ccons version information to std::out.
void PrintVersionInformation();

} // namespace ccons

#endif // CCONS_INTERNAL_COMMANDS_H
