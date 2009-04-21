//
// Implementation of StdInLineReader which simply reads input
// lines from the standard input stream.
//
// Part of ccons, the interactive console for the C programming language.
//
// Copyright (c) 2009 Alexei Svitkine. This file is distributed under the
// terms of MIT Open Source License. See file LICENSE for details.
//

#include "LineReader.h"

#include <iostream>

namespace ccons {

StdInLineReader::StdInLineReader()
{
}

const char * StdInLineReader::readLine(const char *prompt, const char *input)
{
	if (std::getline(std::cin, _line)) {
		_line += "\n";
		return _line.c_str();
	}

	return NULL;
}

} // namespace ccons
