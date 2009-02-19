#include "StringUtils.h"

#include <stdlib.h>
#include <stdarg.h>

namespace ccons {

void vstring_printf(std::string *dst, const char *fmt, va_list ap)
{
	char *s;
	if (vasprintf(&s, fmt, ap) > 0) {
		dst->assign(s);
		free(s);
	}
}

void string_printf(std::string *dst, const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	vstring_printf(dst, fmt, ap);
	va_end(ap);  
}

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
