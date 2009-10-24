//
// RemoteConsole is an implementation of IConsole which spawns and communicates
// with a second ccons process for handling the user input.
//
// SerializedOutputConsole is used by the ccons process spawned by RemoteConsole
// to send its output in a serialized format.
//
// Part of ccons, the interactive console for the C programming language.
//
// Copyright (c) 2009 Alexei Svitkine. This file is distributed under the
// terms of MIT Open Source License. See file LICENSE for details.
//

#include "RemoteConsole.h"

#include <iostream>
#include <sstream>
#include <signal.h>

#include <llvm/ADT/StringExtras.h>

#include "popen2.h"

using std::string;

namespace ccons {

//
// Private Classes
//

namespace {

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


/// UnescapeString - Modify the argument string, turning two character sequences
/// @verbatim
/// like '\\' 'n' into '\n'.  This handles: \e \a \b \f \n \r \t \v \' \ and
/// \num (where num is a 1-3 byte octal value).
/// @endverbatim
void UnescapeString(std::string &Str) {
  for (unsigned i = 0; i != Str.size(); ++i) {
    if (Str[i] == '\\' && i != Str.size()-1) {
      switch (Str[i+1]) {
      default: continue;  // Don't execute the code after the switch.
      case 'a': Str[i] = '\a'; break;
      case 'b': Str[i] = '\b'; break;
      case 'e': Str[i] = 27; break;
      case 'f': Str[i] = '\f'; break;
      case 'n': Str[i] = '\n'; break;
      case 'r': Str[i] = '\r'; break;
      case 't': Str[i] = '\t'; break;
      case 'v': Str[i] = '\v'; break;
      case '"': Str[i] = '\"'; break;
      case '\'': Str[i] = '\''; break;
      case '\\': Str[i] = '\\'; break;
      }
      // Nuke the second character.
      Str.erase(Str.begin()+i+1);
    }
  }
}

/// EscapeString - Modify the argument string, turning '\\' and anything that
/// doesn't satisfy std::isprint into an escape sequence.
void EscapeString(std::string &Str) {
  for (unsigned i = 0; i != Str.size(); ++i) {
    if (Str[i] == '\\') {
      ++i;
      Str.insert(Str.begin()+i, '\\');
    } else if (Str[i] == '\t') {
      Str[i++] = '\\';
      Str.insert(Str.begin()+i, 't');
    } else if (Str[i] == '"') {
      Str.insert(Str.begin()+i++, '\\');
    } else if (Str[i] == '\n') {
      Str[i++] = '\\';
      Str.insert(Str.begin()+i, 'n');
    } else if (!std::isprint(Str[i])) {
      // Always expand to a 3-digit octal escape.
      unsigned Char = Str[i];
      Str[i++] = '\\';
      Str.insert(Str.begin()+i++, '0'+((Char/64) & 7));
      Str.insert(Str.begin()+i++, '0'+((Char/8)  & 7));
      Str.insert(Str.begin()+i  , '0'+( Char     & 7));
    }
  }
}

} // anon namespace


//
// C callback functions
//

void gotsig(int signo)
{
	string str = strsignal(signo);
	str += "\n";
	SerializedConsoleOutput sco("", str, "", "");
	sco.writeToString(&str);
	std::cout << str;
	std::cout.flush();
	exit(-1);
}

void goodbye(void)
{
	string str;
	SerializedConsoleOutput sco("", "", "", "");
	sco.writeToString(&str);
	std::cout << str;
	std::cout.flush();
}


//
// SerializedConsoleOutput
//

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
		UnescapeString(*str);
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
	EscapeString(temp);
	ss << temp << "\n";
	temp = _error;
	EscapeString(temp);
	ss << temp << "\n";
	temp = _prompt;
	EscapeString(temp);
	ss << temp << "\n";
	temp = _input;
	EscapeString(temp);
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


//
// RemoteConsole
//

RemoteConsole::RemoteConsole(bool DebugMode) :
	_DebugMode(DebugMode)
{
	signal(SIGCHLD, SIG_IGN);
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
	if (_DebugMode)
		popen2("./ccons --ccons-debug --ccons-use-std-io --ccons-serialized-output", &infp, &outfp);
	else
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


//
// SerializedOutputConsole
//

SerializedOutputConsole::SerializedOutputConsole(bool DebugMode)
	: _console(new Console(DebugMode, _ss_out, _ss_err))
	, _tmp_out(tmpfile())
	, _tmp_err(tmpfile())
{
	signal(SIGBUS, gotsig);
	signal(SIGSEGV, gotsig);
	signal(SIGABRT, gotsig);
	signal(SIGTRAP, gotsig);
	signal(SIGILL, gotsig);
	signal(SIGFPE, gotsig);
	signal(SIGSYS, gotsig);
	signal(SIGXCPU, gotsig);
	signal(SIGXFSZ, gotsig);
	atexit(goodbye);
}

SerializedOutputConsole::~SerializedOutputConsole()
{
}

const char * SerializedOutputConsole::prompt() const
{
	return _console->prompt();
}

const char * SerializedOutputConsole::input() const
{
	return _console->input();
}

namespace {

class StreamInterceptor {

public:
	
	StreamInterceptor(FILE **stream, FILE *tmp_file)
		: _stream(stream)
		, _orig_stream(*stream)
		, _tmp_file(tmp_file)
	{
		rewind(_tmp_file);
		ftruncate(fileno(_tmp_file), 0);
		*_stream = _tmp_file;
	}

	void getInterceptedOutput(std::stringstream& ss_out) {
		char buf[16 * 1024];
		*buf = '\0';
		rewind(_tmp_file);
		size_t nread = fread(buf, 1, sizeof(buf), _tmp_file);
		buf[nread] = '\0';
		ss_out << buf;
		*_stream = _orig_stream;
	}

private:

	FILE **_stream;
	FILE *_orig_stream;
	FILE *_tmp_file;

};

} // anon namespace

void SerializedOutputConsole::process(const char *line)
{
	_ss_out.str("");
	_ss_err.str("");
	StreamInterceptor iout(&stdout, _tmp_out);
	StreamInterceptor ierr(&stderr, _tmp_err);
	_console->process(line);
	iout.getInterceptedOutput(_ss_out);
	ierr.getInterceptedOutput(_ss_err);
	string str;
	SerializedConsoleOutput sco(_ss_out.str(),
	                            _ss_err.str(),
	                            _console->prompt(),
	                            _console->input());
	sco.writeToString(&str);
	std::cout << str;
	std::cout.flush();
}


} // namespace ccons
