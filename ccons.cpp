#include <iostream>
#include <vector>
#include <google/template.h>
#include <editline/readline.h>

void build_output(const std::string& code, const std::vector<std::string>& headers, std::string *output)
{
	google::TemplateDictionary dict("source");
	dict.SetValue("CODE", code);
	for (std::vector<std::string>::const_iterator it = headers.begin(); it != headers.end(); ++it)
		dict.AddSectionDictionary("HEADERS")->SetValue("NAME", *it);
	google::Template *tpl = google::Template::GetTemplate("skeleton.tpl", google::DO_NOT_STRIP);
	tpl->Expand(output, &dict);
}

char **ccons_completion(const char *text, int start, int end)
{
   return NULL;
}

int main(const int argc, const char **argv)
{
	std::string code = "";
	std::vector<std::string> headers;

	rl_readline_name = "ccons";
	rl_attempted_completion_function = ccons_completion;

	bool done = false;

	const char *prompt = "ccons: ";
	char *line = readline(prompt);
	while (line) {
		code += line;
		code += "\n";
		line = readline(prompt);
	}

	std::string output;
	build_output(code, headers, &output);
	std::cout << output;
	return 0;
}
