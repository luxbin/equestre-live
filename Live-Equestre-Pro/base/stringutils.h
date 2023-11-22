#pragma once
	
/************************************************************************/
// Update History
// - 2017-03-14
//		* add local string to unicode wstring
//		* add unicode wstring to local string
/************************************************************************/

#include <string>
#include <sstream>
#include <algorithm>
#include <functional> 
#include <cctype>
#include <locale>
#include <stdarg.h>  // for va_start, etc
#include <vector>

using namespace std;

#ifndef UNICODE  
typedef std::string String; 
#else
typedef std::wstring String; 
#endif

namespace ns_base {

	UINT LocalToUnicode(UINT codepage, const char* pszA, WCHAR **szStr);
	UINT UnicodeToLocal(UINT codepage, const WCHAR * pszW, char** ppszA);
	inline bool LocalToUnicodeS(UINT page, const char* a1, WCHAR *a2, UINT sz)	{
		WCHAR* out;
		LocalToUnicode(page, (a1), &out);
		if(out) {
			wcsncpy_s((a2), (sz), out, (sz));
			free(out);
			return true;
		}
		return false;
	}
	inline bool  UnicodeToLocalS(UINT page, const WCHAR * a1, char* a2, UINT sz)	{
		char* out;
		UnicodeToLocal(page, (a1), &out);
		if(out) {
			strncpy_s((a2), (sz), out, (sz));
			free(out);
			return true;
		}
		return false;
	}

	inline CString LoadResString(UINT nResID) {
		CString strRes;
		strRes.LoadString(nResID);
		return strRes;
	};

#define AnsiToUnicode(a1, a2)	LocalToUnicode(CP_ACP, a1, a2)
#define UnicodeToAnsi(a1, a2)	UnicodeToLocal(CP_ACP, a1, a2)
#define Utf8ToUnicode(a1, a2)	LocalToUnicode(CP_UTF8, a1, a2)
#define UnicodeToUtf8(a1, a2)	UnicodeToLocal(CP_UTF8, a1, a2)

#define AnsiToUnicodeS(a1, a2, sz)	LocalToUnicodeS(CP_ACP, a1, a2, sz)
#define UnicodeToAnsiS(a1, a2, sz)	UnicodeToLocalS(CP_ACP, a1, a2, sz)
#define Utf8ToUnicodeS(a1, a2, sz)	LocalToUnicodeS(CP_UTF8, a1, a2, sz)
#define UnicodeToUtf8S(a1, a2, sz)	UnicodeToLocalS(CP_UTF8, a1, a2, sz)

	inline string tostring(wstring wstr) {
		string s( wstr.begin(), wstr.end() );
		return s;
	}

	string tostring(wstring wstr, UINT codePage);

	inline wstring towstring(string str) {
		wstring s( str.begin(), str.end() );
		return s;
	}

	wstring towstring(string str, UINT codePage);

	inline string tostring(string str) {
		return str;
	}

	inline wstring towstring(wstring wstr) {
		return wstr;
	}

	inline String toString(string str)
	{
		String s(str.begin(), str.end());
		return s;
	}

	inline String toString(wstring str)
	{
		String s(str.begin(), str.end());
		return s;
	}

	inline string tostring(char *sz) {
		string s(sz);
		return s;
	}

	inline string tostring(wchar_t* wsz) {
		wstring wstr(wsz);
		return tostring(wstr);
	}

	inline wstring towstring(char* sz) {
		string str(sz);
		return towstring(str);
	}

	inline String toString(char *sz) {
		string s(sz);
		return toString(s);
	}

	inline String toString(wchar_t* wsz) {
		wstring s(wsz);
		return toString(s);
	}

	inline string tostring(double val) {
		std::ostringstream s;
		s << val;
		return s.str();
	}

	inline wstring towstring(double val) {
		return towstring(tostring(val));
	}

	inline String toString(double val) {
		return toString(tostring(val));
	}

	inline string tostring(int val) {
		std::ostringstream s;
		s << val;
		return s.str();
	}

	inline wstring towstring(int val) {
		return towstring(tostring(val));
	}

	inline String toString(int val) {
		return toString(tostring(val));
	}

	// trim from start
	static inline string &ltrim(string &s) {
		s.erase(s.begin(), std::find_if(s.begin(), s.end(), std::not1(std::ptr_fun<int, int>(std::isspace))));
		return s;
	}

	// trim from end
	static inline string &rtrim(string &s) {
		s.erase(std::find_if(s.rbegin(), s.rend(), std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s.end());
		return s;
	}

	// trim from both ends
	static inline string &trim(string &s) {
		return ltrim(rtrim(s));
	}

	// trim from start
	static inline wstring &ltrim(wstring &s) {
		s.erase(s.begin(), std::find_if(s.begin(), s.end(), std::not1(std::ptr_fun<int, int>(std::isspace))));
		return s;
	}

	// trim from end
	static inline wstring &rtrim(wstring &s) {
		s.erase(std::find_if(s.rbegin(), s.rend(), std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s.end());
		return s;
	}

	// trim from both ends
	static inline wstring &trim(wstring &s) {
		return ltrim(rtrim(s));
	}

	std::string string_format(const std::string fmt_str, ...);
	std::wstring string_format(const std::wstring fmt_str, ...);
	std::vector<std::string> &split(const std::string &s, char delim, std::vector<std::string> &elems, bool bskipempty = false);
	std::vector<std::wstring> &split(const std::wstring &s, wchar_t delim, std::vector<std::wstring> &elems, bool bskipempty = false);
	std::string		getsegment(string &s, char delim, bool btrim);
	std::wstring	getsegment(wstring &s, wchar_t delim, bool btrim);


}
