//
// Functions to process built-in commands for ccons.
//
// Part of ccons, the interactive console for the C programming language.
//
// Copyright (c) 2009 Alexei Svitkine. This file is distributed under the
// terms of MIT Open Source License. See file LICENSE for details.
//

#include "InternalCommands.h"
#include "StringUtils.h"

#include <string.h>

#include <llvm/Support/DynamicLibrary.h>
#include <llvm/Support/Path.h>

namespace ccons {

// Loads the library that was specified. 
static void HandleLoadCommand(const char *arg, bool debugMode,
                              std::ostream& out, std::ostream& err)
{
	if (debugMode)
		oprintf(err, "Attempting to load external library '%s'.\n", arg);

	llvm::sys::Path path(arg);
	if (path.isDynamicLibrary()) {
		std::string errMsg;
		llvm::sys::DynamicLibrary::LoadLibraryPermanently(arg, &errMsg);
		if (!errMsg.empty()) {
			oprintf(err, "Error: %s\n", errMsg.c_str());
		} else {
			oprintf(out, "Dynamic library loaded.\n");
		}
	} else {
		oprintf(out, "No dynamic library found at the specified path.\n");
	}
}

// Handle an internal command if it was specified. If handled, returns
// true; otherwise the input did not correspond to an internal command.
bool HandleInternalCommand(const char *input, bool debugMode,
                           std::ostream& out, std::ostream& err)
{
	while (isspace(*input)) input++;

	if (*input == ':') {
		int i;
		char arg[4096], *argp = arg;
		input++;

		strncpy(arg, input + 5, sizeof(arg) - 1);
		arg[sizeof(arg) - 1] = '\0';

		while (isspace(*argp)) argp++;
		for (i = strlen(arg) - 1; i >= 0 && isspace(arg[i]); i--)
			arg[i] = '\0';

		if (!strncmp(input, "help", 4)) {
			oprintf(out, "The following commands are available:\n");
			oprintf(out, "  :help - displays this message\n");
			oprintf(out, "  :load <library path> - dynamically loads specified library\n");
			oprintf(out, "  :version - displays ccons version information\n");
		} else if (!strncmp(input, "version", 7)) {
			PrintVersionInformation(out);
		} else if (!strncmp(input, "load", 4)) {
			HandleLoadCommand(argp, debugMode, out, err);
		} else {
			oprintf(out, "Invalid command specified. Type :help for a list of commands.\n");
		}
		return true;
	}
	return false;
}

// Prints ccons version information to the specified ostream.
void PrintVersionInformation(std::ostream& out)
{
	oprintf(out, "ccons version 0.1 by Alexei Svitkine\n");
	oprintf(out, "Interactive Console for the C Programming Language\n");
	oprintf(out, "http://code.google.com/p/ccons\n");
}

// Prints ccons version information to std::out.
void PrintVersionInformation()
{
	PrintVersionInformation(std::cout);
}

} // namespace ccons
