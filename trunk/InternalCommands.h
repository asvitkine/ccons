#ifndef CCONS_INTERNAL_COMMANDS_H
#define CCONS_INTERNAL_COMMANDS_H

#include <iostream>

namespace ccons {

bool HandleInternalCommand(const char *input, bool debugMode,
                           std::ostream& out, std::ostream& err);

} // namespace ccons

#endif // CCONS_INTERNAL_COMMANDS_H
