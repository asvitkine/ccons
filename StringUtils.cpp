//
// Useful string utility functions used by ccons.
//
// Part of ccons, the interactive console for the C programming language.
//
// Copyright (c) 2009 Alexei Svitkine. This file is distributed under the
// terms of MIT Open Source License. See file LICENSE for details.
//

#include "StringUtils.h"

#include <stdlib.h>
#include <stdarg.h>

namespace ccons {

// Like vsprintf(), but to a std::string.
void vstring_printf(std::string *dst, const char *fmt, va_list ap)
{
	char *s;
	if (vasprintf(&s, fmt, ap) > 0) {
		dst->assign(s);
		free(s);
	}
}

// Like sprintf(), but to a std::string.
void string_printf(std::string *dst, const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	vstring_printf(dst, fmt, ap);
	va_end(ap);
}

// Like printf(), but to a std::ostream.
void oprintf(std::ostream& out, const char *fmt, ...)
{
	std::string str;
	va_list ap;
	va_start(ap, fmt);
	vstring_printf(&str, fmt, ap);
	va_end(ap);  
	out << str;
}

} // namespace ccons
