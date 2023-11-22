#include "stdafx.h"
#include "LogSystem.h"

#include "stringutils.h"
using namespace ns_base;

namespace ns_logmanager
{
	__declspec( thread )	char		CLogSystem::m_strlog[LOGBUFFER_SIZE];
	__declspec( thread )	wchar_t		CLogSystem::m_wstrlog[LOGBUFFER_SIZE];

	CLogSystem*	CLogSystem::Instance()
	{
		static CLogSystem inst;
		return &inst;
	}

	//////////////////////////////////////////////////////////////////////////
	void CLogSystem::logFromUnicode(CStringA strLogFileName, LPWSTR strLog, int nLength, int nTimeVisibleType, bool bCRLF)
	{
		char	*szBuf_Char;

		UnicodeToAnsi(strLog, &szBuf_Char);

		CStringA	strLogTemp;
		strLogTemp.Format("%s", szBuf_Char);
		CLogSystem::log(strLogFileName, strLogTemp, strLogTemp.GetLength(), nTimeVisibleType, bCRLF);
		free(szBuf_Char);

	}
	////////////////////////////////////////////////////////////////////////////////////////////////////////
	void CLogSystem::log(CStringA strLogFileName, LPCSTR strLog, int nLength, int nTimeVisibleType , bool bCRLF)
	{
		FILE* fp = 0 ;
		EnterCriticalSection(&m_csLock);

		if ((fp=_fsopen(strLogFileName, "ab+", _SH_DENYNO)) != NULL) 
		{
			if (nTimeVisibleType != time_noVisible)
			{
				CStringA strTime;
				SYSTEMTIME st;
				GetLocalTime(&st);

				if (nTimeVisibleType == time_FullVisible)
				{
					strTime.Format("(%04d-%02d-%02d %02d:%02d:%02d) ", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
				}
				else if (nTimeVisibleType == time_ShortVisible)
				{
					strTime.Format("(%02d:%02d:%02d:%03d) ", st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);
				}
				fwrite(strTime, 1, strTime.GetLength(), fp);
			}

			CStringA strEnd("\x0D\x0A");

			fwrite(strLog, 1, nLength, fp);
			if (bCRLF == true) fwrite(strEnd, 1, strEnd.GetLength(), fp);

			fclose(fp);
		}
		LeaveCriticalSection(&m_csLock);
	}

	void CLogSystem::log_Hex(CStringA strLogFileName,const BYTE* strLog, int nLength , int nTimeVisibleType, bool bCRLF)
	{
		if (IsBadReadPtr((LPVOID)strLog, nLength)) 		return;

		CStringA strData, strtm;
		for (int i = 0; i < nLength; i++)
		{
			strtm.Format("%02X ", strLog[i]);
			strData = strData+strtm;
		}

		log(strLogFileName, strData.GetBuffer(), strData.GetLength(), nTimeVisibleType, bCRLF);
	}

	int CLogSystem::debug_printA(const char* format, ...)
	{
		va_list		argptr;

		va_start(argptr, format);
		vsnprintf_s(m_strlog, LOGBUFFER_SIZE - 1, _TRUNCATE, format, argptr);
		va_end(argptr);

		::OutputDebugStringA(m_strlog);
		return 0;
	}

	int CLogSystem::debug_printW(const wchar_t* format, ...)
	{
		va_list		argptr;

		va_start(argptr, format);
		_vsnwprintf_s(m_wstrlog, LOGBUFFER_SIZE - 1, _TRUNCATE, format, argptr);
		va_end(argptr);

		::OutputDebugStringW(m_wstrlog);
		return 0;
	}

	void CLogSystem::write_logA(const char* szFileName, const char* format, ...)	
	{
		va_list		argptr;
 		CStringA	strLogFileName;
		CStringA	strLog;

		va_start(argptr, format);
		vsnprintf_s(m_strlog, LOGBUFFER_SIZE - 1, _TRUNCATE, format, argptr);
		va_end(argptr);
 
 		strLogFileName.Format("%s",szFileName);
		strLog.Format("%s", m_strlog);
 
 		CLogSystem::log(strLogFileName, strLog.GetBuffer(0), strLog.GetLength(), LOG_TIME_TYPE);
	}

	void CLogSystem::write_logW(const char* szFileName, const wchar_t* format, ...)
	{
		CStringA	strLogFileName;
		CStringW	strLog;
		va_list		argptr;

		va_start(argptr, format);
		_vsnwprintf_s(m_wstrlog, LOGBUFFER_SIZE - 1, _TRUNCATE, format, argptr);
		va_end(argptr);

		strLogFileName.Format("%s",szFileName);
		strLog.Format(L"%s", m_wstrlog);

		CLogSystem::logFromUnicode(strLogFileName,strLog.GetBuffer(0), strLog.GetLength(), LOG_TIME_TYPE);
	}

	void CLogSystem::write_loghex(const char* szFileName,const void* strLog, int nLength)
	{
		CStringA	strLogFileName;
		strLogFileName.Format("%s",szFileName);
		CLogSystem::log_Hex(strLogFileName, (BYTE*)strLog, nLength, LOG_TIME_TYPE);
	}

	void CLogSystem::write_logAEx(const char* szFileName, int nTimeVisible, bool bCRLF, const char* format, ...)
	{
		va_list		argptr;
		CStringA	strLogFileName;
		CStringA	strLog;

		va_start(argptr, format);
		vsnprintf_s(m_strlog, LOGBUFFER_SIZE - 1, _TRUNCATE, format, argptr);
		va_end(argptr);

		strLogFileName.Format("%s",szFileName);
		strLog.Format("%s", m_strlog);

		CLogSystem::log(strLogFileName,strLog.GetBuffer(0), strLog.GetLength(), nTimeVisible, bCRLF);
	}

	void CLogSystem::write_logWEx(const char* szFileName, int nTimeVisible, bool bCRLF, const wchar_t* format, ...)
	{
		CStringA	strLogFileName;
		CStringW	strLog;
		va_list		argptr;

		va_start(argptr, format);
		_vsnwprintf_s(m_wstrlog, LOGBUFFER_SIZE - 1, _TRUNCATE, format, argptr);
		va_end(argptr);

		strLogFileName.Format("%s",szFileName);
		strLog.Format(L"%s", m_wstrlog);

		CLogSystem::logFromUnicode(strLogFileName,strLog.GetBuffer(0), strLog.GetLength(), nTimeVisible, bCRLF);
	}

	void CLogSystem::write_loghexEx(const char* szFileName, int nTimeVisible, bool bCRLF, const void* strLog, int nLength)
	{
		CStringA	strLogFileName;
		strLogFileName.Format("%s",szFileName);
		CLogSystem::log_Hex(strLogFileName, (BYTE*)strLog, nLength, nTimeVisible, bCRLF);
	}

	void CLogSystem::write_logbinary(const char* szFileName,const void* strLog, int nLength)
	{
		CStringA	strLogFileName;
		strLogFileName.Format("%s",szFileName);
		CLogSystem::log(strLogFileName, (LPCSTR)strLog, nLength, time_noVisible, false);
	}

	void CLogSystem::debug_msgA(const char* format, ...)
	{
		va_list		argptr;
		CStringA	strLog;

		va_start(argptr, format);
		vsnprintf_s(m_strlog, LOGBUFFER_SIZE - 1, _TRUNCATE, format, argptr);
		va_end(argptr);

		strLog.Format("%s", m_strlog);

		::MessageBoxA(NULL, strLog.GetBuffer(),"DbgMsgA", MB_OK);
	}

	void CLogSystem::debug_msgW(const wchar_t* format, ...)
	{
		CStringW	strLog;
		va_list		argptr;

		va_start(argptr, format);
		_vsnwprintf_s(m_wstrlog, LOGBUFFER_SIZE - 1, _TRUNCATE, format, argptr);
		va_end(argptr);

		strLog.Format(L"%s", m_wstrlog);
		::MessageBoxW(NULL, strLog.GetBuffer(), _T("DbgMsgW"), MB_OK);
	}


	void CLogSystem::write_eg_logA(const char* szGameID, const char* szFileName, const char* format, ...)
	{
		va_list		argptr;
		CStringA	strLogFileName;
		CStringA	strLog;

		va_start(argptr, format);
		vsnprintf_s(m_strlog, LOGBUFFER_SIZE - 1, _TRUNCATE, format, argptr);
		va_end(argptr);

		strLogFileName.Format(szFileName, szGameID);
		strLog.Format("%s", m_strlog);

		CLogSystem::log(strLogFileName, strLog.GetBuffer(0), strLog.GetLength(), LOG_TIME_TYPE);
	}

	void CLogSystem::write_eg_logW(const char* szGameID, const char* szFileName, const wchar_t* format, ...)
	{
		CStringA	strLogFileName;
		CStringW	strLog;
		va_list		argptr;

		va_start(argptr, format);
		_vsnwprintf_s(m_wstrlog, LOGBUFFER_SIZE - 1, _TRUNCATE, format, argptr);
		va_end(argptr);

		strLogFileName.Format(szFileName, szGameID);
		strLog.Format(L"%s", m_wstrlog);

		CLogSystem::logFromUnicode(strLogFileName,strLog.GetBuffer(0), strLog.GetLength(), LOG_TIME_TYPE);
	}

	void CLogSystem::write_eg_loghex(const char* szGameID, const char* szFileName,const void* strLog, int nLength)
	{
		CStringA	strLogFileName;
		strLogFileName.Format(szFileName, szGameID);
		CLogSystem::log_Hex(strLogFileName, (BYTE*)strLog, nLength, LOG_TIME_TYPE);
	}

	void CLogSystem::write_eg_logAEx(const char* szGameID,  const char* szFileName, int nTimeVisible, bool bCRLF, const char* format, ...)
	{
		va_list		argptr;
		CStringA	strLogFileName;
		CStringA	strLog;

		va_start(argptr, format);
		vsnprintf_s(m_strlog, LOGBUFFER_SIZE - 1, _TRUNCATE, format, argptr);
		va_end(argptr);

		strLogFileName.Format(szFileName, szGameID);
		strLog.Format("%s", m_strlog);

		CLogSystem::log(strLogFileName,strLog.GetBuffer(0), strLog.GetLength(), nTimeVisible, bCRLF);

	}

	void CLogSystem::write_eg_logWEx(const char* szGameID,  const char* szFileName, int nTimeVisible, bool bCRLF, const wchar_t* format, ...)
	{
		CStringA	strLogFileName;
		CStringW	strLog;
		va_list		argptr;

		va_start(argptr, format);
		_vsnwprintf_s(m_wstrlog, LOGBUFFER_SIZE - 1, _TRUNCATE, format, argptr);
		va_end(argptr);

		strLogFileName.Format(szFileName, szGameID);
		strLog.Format(L"%s", m_wstrlog);

		CLogSystem::logFromUnicode(strLogFileName,strLog.GetBuffer(0), strLog.GetLength(), nTimeVisible, bCRLF);
	}

	void CLogSystem::write_eg_loghexEx(const char* szGameID,  const char* szFileName, int nTimeVisible, bool bCRLF, const void* strLog, int nLength)
	{
		CStringA	strLogFileName;
		strLogFileName.Format(szFileName, szGameID);
		CLogSystem::log_Hex(strLogFileName, (BYTE*)strLog, nLength, nTimeVisible, bCRLF);
	}

	void CLogSystem::write_eg_logbinary(const char* szGameID,  const char* szFileName,const void* strLog, int nLength)
	{
		CStringA	strLogFileName;
		strLogFileName.Format(szFileName, szGameID);
		CLogSystem::log(strLogFileName, (LPCSTR)strLog, nLength, time_noVisible, false);
	}

}
