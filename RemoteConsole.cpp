#include "RemoteConsole.h"

#include <iostream>
#include <sstream>

#include <llvm/ADT/StringExtras.h>

#include "popen2.h"

using std::string;

namespace ccons {

RemoteConsole::RemoteConsole()
{
	reset();
}

RemoteConsole::~RemoteConsole()
{
	cleanup();
}

void RemoteConsole::cleanup()
{
	if (_ostream) {
		fclose(_ostream);
		_ostream = NULL;
	}
	if (_istream) {
		fclose(_istream);
		_istream = NULL;
	}
}

void RemoteConsole::reset()
{
	// TODO: check for errors!
	int infp, outfp;
	popen2("./ccons --ccons-use-std-io --ccons-serialized-output", &infp, &outfp);
	_ostream = fdopen(infp, "w");
	setlinebuf(_ostream);
	_istream = fdopen(outfp, "r");
	setlinebuf(_istream);
	_prompt = ">>> ";
	_input = "";
}

const char * RemoteConsole::prompt() const
{
	return _prompt.c_str();
}

const char * RemoteConsole::input() const
{
	return _input.c_str();
}

void RemoteConsole::process(const char *line)
{
	fputs(line, _ostream);

	SerializedConsoleOutput sco;
	bool success = sco.readFromStream(_istream);
	if (success) {
		if (sco.prompt().empty() && sco.output().empty() && sco.error().empty())
			exit(0);
		std::cout << sco.output();
		std::cerr << sco.error();
	}

	if (!success || sco.prompt().empty()) {
		cleanup();
		reset();
		std::cout << "\nNOTE: Interpreter restarted.\n";
		return;
	}

	_prompt = sco.prompt();
	_input = sco.input();
}


SerializedConsoleOutput::SerializedConsoleOutput()
{
}

SerializedConsoleOutput::SerializedConsoleOutput(const std::string& output,
                                                 const std::string& error,
                                                 const std::string& prompt,
                                                 const std::string& input)
	: _output(output)
	, _error(error)
	, _prompt(prompt)
	, _input(input)
{
}

bool SerializedConsoleOutput::readEscapedString(FILE *stream, std::string *str)
{
	char buf[1024];
	if (fgets(buf, sizeof(buf), stream)) {
		buf[strlen(buf) - 1] = '\0';
		*str = buf;
		llvm::UnescapeString(*str);
		return true;
	}
	return false;
}

bool SerializedConsoleOutput::readFromStream(FILE *stream)
{
	return readEscapedString(stream, &_output) &&
	       readEscapedString(stream, &_error) &&
	       readEscapedString(stream, &_prompt) &&
	       readEscapedString(stream, &_input);
}

void SerializedConsoleOutput::writeToString(std::string *str) const
{
	std::stringstream ss;
	string temp;
	temp = _output;
	llvm::EscapeString(temp);
	ss << temp << "\n";
	temp = _error;
	llvm::EscapeString(temp);
	ss << temp << "\n";
	temp = _prompt;
	llvm::EscapeString(temp);
	ss << temp << "\n";
	temp = _input;
	llvm::EscapeString(temp);
	ss << temp << "\n";
	*str = ss.str();
}

const std::string& SerializedConsoleOutput::output() const
{
	return _output;
}

const std::string& SerializedConsoleOutput::error() const
{
	return _error;
}

const std::string& SerializedConsoleOutput::prompt() const
{
	return _prompt;
}

const std::string& SerializedConsoleOutput::input() const
{
	return _input;
}

} // namespace ccons
