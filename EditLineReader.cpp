//
// EditLineReader is a LineReader that uses the libedit library to read
// input from the user.
//
// Part of ccons, the interactive console for the C programming language.
//
// Copyright (c) 2009 Alexei Svitkine. This file is distributed under the
// terms of MIT Open Source License. See file LICENSE for details.
//

#include "EditLineReader.h"

#include <string.h>
#include <limits.h>

#include "complete.h"

namespace ccons {

namespace {

static EditLineReader * reader;

static const char *ccons_prompt(EditLine *e)
{
	return (reader ? reader->getPrompt() : "??? ");
}

static unsigned char ccons_autocomplete(EditLine *e, int ch)
{
	const LineInfo *line = el_line(e);
	char path[1024], insert[1024];
	const char *p = line->buffer;

	if (strncmp(p, ":load", 5))
		return CC_ERROR;

	p += 5;
	while (isspace(*p)) p++;

	int len = std::max(0, std::min<int>(sizeof(path) - 1, line->cursor - p));
	strncpy(path, p, len);
	path[len] = '\0';
	
	if (complete(path, insert, sizeof(insert)) > 0) {
		if (el_insertstr(e, insert) != -1) {
			return CC_REFRESH;
		}
	}

	return CC_ERROR;
}

} // anon namespace

EditLineReader::EditLineReader()
	: _history(history_init())
	, _editLine(el_init("ccons", stdin, stdout, stderr))
{
	reader = this;
	history(_history, &_event, H_SETSIZE, INT_MAX);
	el_set(_editLine, EL_PROMPT, ccons_prompt);
	el_set(_editLine, EL_EDITOR, "emacs");
	el_set(_editLine, EL_HIST, history, _history);
	el_set(_editLine, EL_ADDFN, "ccons-ac", "autocomplete", ccons_autocomplete);
	el_set(_editLine, EL_BIND, "^I", "ccons-ac", NULL);
}

EditLineReader::~EditLineReader()
{
	el_end(_editLine);
	history_end(_history);
}

const char * EditLineReader::readLine(const char *prompt, const char *input)
{
	_prompt = prompt;

	el_push(_editLine, input);

	int n;
	const char *line = el_gets(_editLine, &n);

	if (line)
		history(_history, &_event, H_ENTER, line);

	return line;
}

const char * EditLineReader::getPrompt() const
{
	return _prompt.c_str();
}

} // namespace ccons
