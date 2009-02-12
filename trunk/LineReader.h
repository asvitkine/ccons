#ifndef CCONS_LINE_READER_H
#define CCONS_LINE_READER_H

namespace ccons {

class LineReader {

public:

	virtual ~LineReader() {}
	virtual const char * readLine(const char *prompt, const char *input) = 0;

};

class StdInLineReader : public LineReader {

public:

	const char * readLine(const char *prompt, const char *input);

};

} // namespace ccons

#endif // CCONS_LINE_READER_H
