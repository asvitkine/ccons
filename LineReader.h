#ifndef CCONS_LINE_READER_H
#define CCONS_LINE_READER_H

//
// LineReader defines an interface to be implemented by derived classes for
// reading line input from the users.
//
// Part of ccons, the interactive console for the C programming language.
//
// Copyright (c) 2009 Alexei Svitkine. This file is distributed under the
// terms of MIT Open Source License. See file LICENSE for details.
//

#include <string>

namespace ccons {

class LineReader {

public:

	virtual ~LineReader() {}
	virtual const char * readLine(const char *prompt, const char *input) = 0;

};

class StdInLineReader : public LineReader {

public:

	StdInLineReader();

	const char * readLine(const char *prompt, const char *input);

private:

	std::string _line;

};

} // namespace ccons

#endif // CCONS_LINE_READER_H
