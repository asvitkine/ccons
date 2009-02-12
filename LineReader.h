#ifndef CCONS_LINE_READER_H
#define CCONS_LINE_READER_H

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
