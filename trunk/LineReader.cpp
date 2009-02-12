#include "LineReader.h"

#include <iostream>

namespace ccons {

StdInLineReader::StdInLineReader()
{
}

const char * StdInLineReader::readLine(const char *prompt, const char *input)
{
	if (std::getline(std::cin, _line))
		return _line.c_str();

	return NULL;
}

} // namespace ccons
