#ifndef CCONS_EDIT_LINE_READER_H
#define CCONS_EDIT_LINE_READER_H

//
// EditLineReader is a LineReader that uses the libedit library to read
// input from the user.
//
// Part of ccons, the interactive console for the C programming language.
//
// Copyright (c) 2009 Alexei Svitkine. This file is distributed under the
// terms of MIT Open Source License. See file LICENSE for details.
//

#include <string>

#include <histedit.h>

#include "LineReader.h"

namespace ccons {

class EditLineReader : public LineReader {

public:

	EditLineReader();
	virtual ~EditLineReader();

	const char * readLine(const char *prompt, const char *input);
	const char * getPrompt() const;

private:

	std::string _prompt;
	HistEvent _event;
	History *_history;
	EditLine *_editLine;

};

} // namespace ccons

#endif // CCONS_EDIT_LINE_READER_H
