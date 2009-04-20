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
	const char *path = line->buffer;
	char insert[1024];

	if (strncmp(path, ":load", 5))
		return CC_ERROR;

	path += 5;
	while (isspace(*path)) path++;
	
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

	const char *line = el_gets(_editLine, NULL);

	if (line)
		history(_history, &_event, H_ENTER, line);

	return line;
}

const char * EditLineReader::getPrompt() const
{
	return _prompt.c_str();
}

} // namespace ccons
