#include "stdafx.h"
#include "stringutils.h"

namespace ns_base {
	std::string string_format(const std::string fmt_str, ...) {
		int size = ((int)fmt_str.size()) * 2 + 50;   // use a rubric appropriate for your code
		std::string str;
		va_list ap;
		while (1) {     // maximum 2 passes on a POSIX system...
			str.resize(size);
			va_start(ap, fmt_str);
			int n = _vsnprintf_s((char *)str.data(), size, size, fmt_str.c_str(), ap);
			va_end(ap);
			if (n > -1 && n < size) {  // everything worked
				str.resize(n);
				return str;
			}
			if (n > -1)  // needed size returned
				size = n + 1;   // for null char
			else
				size *= 2;      // guess at a larger size (o/s specific)
		}
		return str;
	}

	std::wstring string_format(const std::wstring fmt_str, ...) {
		int size = ((int)fmt_str.size()) * 2 + 50;   // use a rubric appropriate for your code
		std::wstring str;
		va_list ap;
		while (1) {     // maximum 2 passes on a POSIX system...
			str.resize(size);
			va_start(ap, fmt_str);
			int n = _vsnwprintf_s((wchar_t *)str.data(), size, size, fmt_str.c_str(), ap);
			va_end(ap);
			if (n > -1 && n < size) {  // everything worked
				str.resize(n);
				return str;
			}
			if (n > -1)  // needed size returned
				size = n + 1;   // for null char
			else
				size *= 2;      // guess at a larger size (o/s specific)
		}
		return str;
	}

	std::vector<std::string> &split(const std::string &s, char delim, std::vector<std::string> &elems, bool bskipempty) {
		std::stringstream ss(s);
		std::string item;
		while (std::getline(ss, item, delim)) {
			if(bskipempty && item.empty())
				continue;
			elems.push_back(item);
		}
		return elems;
	}

	std::vector<std::wstring> &split(const std::wstring &s, wchar_t delim, std::vector<std::wstring> &elems, bool bskipempty) {
		std::wstringstream ss(s);
		std::wstring item;
		while (std::getline(ss, item, delim)) {
			if(bskipempty && item.empty())
				continue;
			elems.push_back(item);
		}
		return elems;
	}

	string	getsegment(string &s, char delim, bool btrim) {
		size_t found = s.find(delim);
		string ret = s.substr(0, found);
		if(btrim) {
			s = s.substr(ret.length() + 1);
		}
		return ret;
	}

	wstring	getsegment(wstring &s, wchar_t delim, bool btrim) {
		size_t found = s.find(delim);
		wstring ret = s.substr(0, found);
		if(btrim) {
			s = s.substr(ret.length() + 1);
		}
		return ret;
	}

	UINT UnicodeToLocal(UINT codepage, const WCHAR * pszW, char** ppszA)
	{
		ULONG cbAnsi, cCharacters;
		UINT dwResult;
		if (pszW == NULL) 
		{
			*ppszA = 0;
			return 0;
		}

		cCharacters = wcslen(pszW) + 1;
		dwResult = cCharacters;
		cbAnsi = cCharacters * 3;
		*ppszA = (LPSTR) malloc(cbAnsi);

		if (NULL == ppszA)   return 0;
		if (0 == WideCharToMultiByte(codepage, 0, pszW, cCharacters, *ppszA,
			cbAnsi, NULL, NULL))
		{
			free(*ppszA);
			*ppszA = 0;
			return 0;
		}

		return dwResult;
	}

	// ansi to unicode
	UINT LocalToUnicode(UINT codepage, const char* pszA, WCHAR **szStr)
	{
		UINT dwResult;
		ULONG cCharacters;

		// If input is null then just return the same.
		if (NULL == pszA) {
			*szStr = 0;
			return 0;
		}

		cCharacters =  strlen(pszA)+1;
		dwResult = cCharacters * 2;
		*szStr = (WCHAR*) malloc(cCharacters*2);
		if (NULL == *szStr) return 0;
		// Covert to Unicode.
		if (0 == MultiByteToWideChar(codepage, 0, pszA, cCharacters,
			*szStr, cCharacters))
		{
			free(*szStr);
			*szStr = NULL;
			return 0;
		}

		return dwResult;
	}

	string tostring(wstring wstr, UINT codePage)
	{
		string out;
		int size = (wstr.length() + 1) * 3;
		out.resize(size);
		UnicodeToLocalS(codePage, wstr.c_str(), (char *)out.data(), size);
		return out.c_str();
	}

	wstring towstring(string str, UINT codePage)
	{
		wstring out;
		int size = (str.length() + 1);
		out.resize(size);
		LocalToUnicodeS(codePage, str.c_str(), (wchar_t *)out.data(), size);
		return out.c_str();
	}

}
