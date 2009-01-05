#include <iostream>
#include <vector>
#include <google/template.h>

void build_output(const std::string& code, const std::vector<std::string>& headers, std::string* output)
{
	google::TemplateDictionary dict("source");
	dict.SetValue("CODE", code);
	for (std::vector<std::string>::const_iterator it = headers.begin(); it != headers.end(); ++it)
		dict.AddSectionDictionary("HEADERS")->SetValue("NAME", *it);
	google::Template* tpl = google::Template::GetTemplate("skeleton.tpl", google::DO_NOT_STRIP);
	tpl->Expand(output, &dict);
}

int main(const int argc, const char **argv)
{
	std::string code = "";
	std::vector<std::string> headers;

	std::string output;
	build_output(code, headers, &output);
	std::cout << output;
	return 0;
}
