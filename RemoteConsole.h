#ifndef CCONS_REMOTE_CONSOLE_H
#define CCONS_REMOTE_CONSOLE_H

#include <stdio.h>
#include <string>

#include "Console.h"

namespace ccons {

class RemoteConsole : public IConsole {

public:

	RemoteConsole();
	virtual ~RemoteConsole();

	const char * prompt() const;
	const char * input() const;
	void process(const char *line);

private:

	FILE *_stream;
	std::string _prompt;
	std::string _input;

	void reset();

};

class SerializedConsoleOutput {

public:

	SerializedConsoleOutput();
	SerializedConsoleOutput(const std::string& output,
	                        const std::string& error,
	                        const std::string& prompt,
	                        const std::string& input);

	bool readFromStream(FILE *stream);
	void writeToString(std::string *str) const;
	
	const std::string& output() const;
	const std::string& error() const;
	const std::string& prompt() const;
	const std::string& input() const;

private:

	std::string _output;
	std::string _error;
	std::string _prompt;
	std::string _input;

	static bool readEscapedString(FILE *stream, std::string *str);

};

} // namespace ccons

#endif // CCONS_REMOTE_CONSOLE_H
