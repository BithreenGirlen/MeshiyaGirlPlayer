
#include <shlwapi.h>

#include "win_text.h"

namespace win_text
{
	/*•¶Žš’uŠ·*/
	char* ReplaceString(char* src, const char* pzOld, const char* pzNew)
	{
		if (src == nullptr)return nullptr;
		size_t nSrcLen = strlen(src);

		size_t nOldLen = strlen(pzOld);
		if (nOldLen == 0)return nullptr;

		size_t nNewLen = strlen(pzNew);

		char* p = nullptr;
		char* pp = src;
		int iCount = 0;

		for (;;)
		{
			p = strstr(pp, pzOld);
			if (p == nullptr)break;

			pp = p + nOldLen;
			++iCount;
		}

		size_t nSize = nSrcLen + iCount * (nNewLen - nOldLen) + 1;
		char* pResult = static_cast<char*>(malloc(nSize));
		if (pResult == nullptr)return nullptr;

		size_t nPos = 0;
		size_t nLen = 0;
		pp = src;
		for (;;)
		{
			p = strstr(pp, pzOld);
			if (p == nullptr)
			{
				nLen = nSrcLen - (pp - src);
				memcpy(pResult + nPos, pp, nLen);
				nPos += nLen;
				break;
			}

			nLen = p - pp;
			memcpy(pResult + nPos, pp, nLen);
			nPos += nLen;
			memcpy(pResult + nPos, pzNew, nNewLen);
			nPos += nNewLen;
			pp = p + nOldLen;
		}

		*(pResult + nPos) = '\0';
		return pResult;
	}
}

/*std::string to std::wstring*/
std::wstring win_text::WidenUtf8(const std::string& str)
{
	if (!str.empty())
	{
		int iLen = ::MultiByteToWideChar(CP_UTF8, 0, str.c_str(), static_cast<int>(str.length()), nullptr, 0);
		if (iLen > 0)
		{
			std::wstring wstr(iLen, 0);
			::MultiByteToWideChar(CP_UTF8, 0, str.c_str(), static_cast<int>(str.length()), &wstr[0], iLen);
			return wstr;
		}
	}

	return std::wstring();
}
/*std::wstring to std::string*/
std::string win_text::NarrowUtf8(const std::wstring& wstr)
{
	if (!wstr.empty())
	{
		int iLen = ::WideCharToMultiByte(CP_OEMCP, 0, wstr.c_str(), static_cast<int>(wstr.length()), nullptr, 0, nullptr, nullptr);
		if (iLen > 0)
		{
			std::string str(iLen, 0);
			::WideCharToMultiByte(CP_OEMCP, 0, wstr.c_str(), static_cast<int>(wstr.length()), &str[0], iLen, nullptr, nullptr);
			return str;
		}
	}
	return std::string();
}

std::string win_text::ReplaceStr(const std::string& src, const char* pzOld, const char* pzNew)
{
	char* pBuffer = ReplaceString(const_cast<char*>(&src[0]), pzOld, pzNew);
	if (pBuffer != nullptr)
	{
		std::string strResult = pBuffer;
		free(pBuffer);
		return strResult;
	}
	return std::string();
}
