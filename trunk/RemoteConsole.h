#ifndef CCONS_REMOTE_CONSOLE_H
#define CCONS_REMOTE_CONSOLE_H

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

#include <stdio.h>
#include <string>
#include <sstream>

#include "Console.h"

namespace ccons {

//
// RemoteConsole
//

class RemoteConsole : public IConsole {

public:

	explicit RemoteConsole(bool DebugMode);
	virtual ~RemoteConsole();

	const char * prompt() const;
	const char * input() const;
	void process(const char *line);

private:

	bool _DebugMode;
	FILE *_ostream;
	FILE *_istream;
	std::string _prompt;
	std::string _input;

	void cleanup();
	void reset();

};

//
// SerializedOutputConsole
//

class SerializedOutputConsole : public IConsole {

public:

	explicit SerializedOutputConsole(bool DebugMode);
	virtual ~SerializedOutputConsole();

	const char * prompt() const;
	const char * input() const;
	void process(const char *line);

private:

	llvm::OwningPtr<IConsole> _console;
	std::stringstream _ss_out;
	std::stringstream _ss_err;
	FILE *_tmp_out;
	FILE *_tmp_err;

};

} // namespace ccons

#endif // CCONS_REMOTE_CONSOLE_H
