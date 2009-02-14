#include "RemoteConsole.h"

#include <iostream>

#include <google/protobuf/text_format.h>

#include "ccons.pb.h"

using std::string;

namespace ccons {

RemoteConsole::RemoteConsole()
	: _stream(popen("./ccons --use-std-io --proto-buf-output", "r+"))
	, _prompt(">>> ")
{
	assert(_stream && "Could not popen!");
}

RemoteConsole::~RemoteConsole()
{
	pclose(_stream);
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
	fputs(line, _stream);
	ConsoleOutput output;
	string str;
	char buf1[1000];
	fgets(buf1, sizeof(buf1), _stream); str += buf1;
	fgets(buf1, sizeof(buf1), _stream); str += buf1;
	fgets(buf1, sizeof(buf1), _stream); str += buf1;
	google::protobuf::TextFormat::ParseFromString(str, &output);
	std::cout << output.output();
	_prompt = output.prompt();
	_input = output.input();
}

} // namespace ccons
