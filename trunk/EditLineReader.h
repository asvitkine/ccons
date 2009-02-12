#ifndef CCONS_EDIT_LINE_READER_H
#define CCONS_EDIT_LINE_READER_H

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
