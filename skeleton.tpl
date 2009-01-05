{{#HEADERS}}
#include <{{NAME}}>{{/HEADERS}}

int printf(const char * restrict format, ...);

int main(void)
{
	{{CODE}}
	return 0;
}
