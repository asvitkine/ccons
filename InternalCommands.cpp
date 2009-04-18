#include "InternalCommands.h"
#include "StringUtils.h"

#include <llvm/System/DynamicLibrary.h>

namespace ccons {

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
		} else if (!strncmp(input, "load", 4)) {
			HandleLoadCommand(argp, debugMode, out, err);
		} else {
			oprintf(out, "Invalid command specified. Type :help for a list of commands.\n");
		}
		return true;
	}
	return false;
}

} // namespace ccons
