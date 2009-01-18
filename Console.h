#ifndef CCONS_CONSOLE_H
#define CCONS_CONSOLE_H

#include <string>

#include "Parser.h"

namespace ccons {

class Console {

public:

	Console();
	virtual ~Console();

	const char * prompt() const;
	void process(const char * line);

private:

	std::string _includes;
	std::string _prompt;
	std::string _body;
	clang::LangOptions _options;
	Parser _parser;

};

} // namespace ccons

#endif // CCONS_CONSOLE_H
