#ifndef WIN_TEXT_H_
#define WIN_TEXT_H_

#include <string>

namespace win_text
{
	std::wstring WidenUtf8(const std::string& str);
	std::string NarrowUtf8(const std::wstring& wstr);
	std::string ReplaceStr(const std::string& src, const char* pzOld, const char* pzNew);
}

#endif //WIN_TEXT_H_

