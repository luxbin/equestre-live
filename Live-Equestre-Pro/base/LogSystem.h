#pragma once

/************************************************************************
//	Lib for Logging into file
//	Update History
//		- 2017-03-14
//			* remove unicode to/from local functions
//			* remove hierarchical folder creation
************************************************************************/
#include <stdio.h>
#include <share.h>
#include <atltime.h>

// 
namespace ns_logmanager
{
	enum TimeDisplayType
	{
		time_noVisible = 0,
		time_FullVisible = 1,
		time_ShortVisible = 2
	};

	#define		LOG_TIME_TYPE			time_ShortVisible
	#define		LOGBUFFER_SIZE			(8 * 1024)

	class CLogSystem
	{
	public:
		CLogSystem() {  InitializeCriticalSection(&m_csLock); }
		~CLogSystem() {};

		static CLogSystem*	Instance();

		CStringA getEgPath() { return m_egPath; };
		void setEgPath(CStringA path) { m_egPath = path; };

	protected:
		CStringA		m_egPath;

		__declspec( thread )	static	char		m_strlog[LOGBUFFER_SIZE];
		__declspec( thread )	static	wchar_t		m_wstrlog[LOGBUFFER_SIZE];


	protected:
		void logFromUnicode(CStringA strLogFileName, LPWSTR strLog, int nLength, int nTimeVisibleType, bool bCRLF = true);		
		void log(CStringA strLogFileName, LPCSTR strLog, int nLength, int nTimeVisibleType, bool bCRLF = true);		
		void log_Hex(CStringA strLogFileName,const BYTE* strLog, int nLength , int nTimeVisibleType, bool bCRLF = true);

	public:
		int debug_printA(const char* format, ...);
		int debug_printW(const wchar_t* format, ...);

		void write_logA(const char* szFileName, const char* format, ...);
		void write_logW(const char* szFileName, const wchar_t* format, ...);
		void write_loghex(const char* szFileName,const void* strLog, int nLength);

		void write_logAEx(const char* szFileName, int nTimeVisible, bool bCRLF, const char* format, ...);
		void write_logWEx(const char* szFileName, int nTimeVisible, bool bCRLF, const wchar_t* format, ...);
		void write_loghexEx(const char* szFileName, int nTimeVisible, bool bCRLF, const void* strLog, int nLength);

		void write_logbinary(const char* szFileName,const void* strLog, int nLength);

		void debug_msgA(const char* format, ...);
		void debug_msgW(const wchar_t* format, ...);

		void write_eg_logA(const char* szGameID, const char* szFileName, const char* format, ...);
		void write_eg_logW(const char* szGameID, const char* szFileName, const wchar_t* format, ...);
		void write_eg_loghex(const char* szGameID, const char* szFileName,const void* strLog, int nLength);

		void write_eg_logAEx(const char* szGameID,  const char* szFileName, int nTimeVisible, bool bCRLF, const char* format, ...);
		void write_eg_logWEx(const char* szGameID, const char* szFileName, int nTimeVisible, bool bCRLF, const wchar_t* format, ...);
		void write_eg_loghexEx(const char* szGameID,  const char* szFileName, int nTimeVisible, bool bCRLF, const void* strLog, int nLength);

		void write_eg_logbinary(const char* szGameID,  const char* szFileName,const void* strLog, int nLength);

		CRITICAL_SECTION	m_csLock;
	};

	#ifdef LOG_ENABLE
		#define DbgOutMsgA(szformat, ...)			 CLogSystem::Instance()->debug_msgA(szformat, __VA_ARGS__)
		#define DbgOutMsgW(szformat, ...)			 CLogSystem::Instance()->debug_msgW(szformat,__VA_ARGS__)
		#define DbgPrintA(szformat, ...)			 CLogSystem::Instance()->debug_printA(szformat, __VA_ARGS__)
		#define DbgPrintW(szformat, ...)			 CLogSystem::Instance()->debug_printW(szformat, __VA_ARGS__)

		//#define DbgOutA(szfilename, szformat, ...)	 CLogSystem::Instance()->write_logA(szfilename, szformat, __VA_ARGS__)
		#define DbgOutA(szfilename, szformat, ...)	 CLogSystem::Instance()->debug_printA(szformat, __VA_ARGS__)
		#define DbgOutW(szfilename, szformat, ...)	 CLogSystem::Instance()->write_logW(szfilename, szformat, __VA_ARGS__)
		#define DbgOutHex(szfilename, szlog, length) CLogSystem::Instance()->write_loghex(szfilename, szlog, length)	

		#define DbgOutAEx(szfilename, nTimeVisible, bCRLF, szformat, ...) CLogSystem::Instance()->write_logAEx(szfilename, nTimeVisible, bCRLF, szformat, __VA_ARGS__)
		#define DbgOutWEx(szfilename, nTimeVisible, bCRLF, szformat, ...) CLogSystem::Instance()->write_logWEx(szfilename, nTimeVisible, bCRLF, szformat, __VA_ARGS__)
		#define DbgOutHexEx(szfilename, nTimeVisible, bCRLF, szformat, ...) CLogSystem::Instance()->write_loghexEx(szfilename, nTimeVisible, bCRLF, szformat, __VA_ARGS__)

		#define DbgOutBinary(szfilename, szlog, length)			CLogSystem::Instance()->write_logbinary(szfilename, szlog, length)

		#define EgDbgOutA(szgameid, szfilename, szformat, ...)	 CLogSystem::Instance()->write_eg_logA(szgameid, szfilename, szformat, __VA_ARGS__)
		#define EgDbgOutW(szgameid, szfilename, szformat, ...)	 CLogSystem::Instance()->write_eg_logW(szgameid, szfilename, szformat, __VA_ARGS__)
		#define EgDbgOutHex(szgameid, szfilename, szlog, length) CLogSystem::Instance()->write_eg_loghex(szgameid, szfilename, szlog, length)	

		#define EgDbgOutAEx(szgameid, szfilename, nTimeVisible, bCRLF, szformat, ...) CLogSystem::Instance()->write_eg_logAEx(szgameid, szfilename, nTimeVisible, bCRLF, szformat, __VA_ARGS__)
		#define EgDbgOutWEx(szgameid, szfilename, nTimeVisible, bCRLF, szformat, ...)	 CLogSystem::Instance()->write_eg_logWEx(szgameid, szfilename, nTimeVisible, bCRLF, szformat, __VA_ARGS__)
		#define EgDbgOutHexEx(szgameid, szfilename, nTimeVisible, bCRLF, szformat, ...) CLogSystem::Instance()->write_eg_loghexEx(szgameid, szfilename, nTimeVisible, bCRLF, szformat, __VA_ARGS__)

		#define EgDbgOutBinary(szgameid, szfilename, szlog, length)			CLogSystem::Instance()->write_eg_logbinary(szgameid, szfilename, szlog, length)
	#else
		#define DbgOutMsgA(szformat, ...)
		#define DbgOutMsgW(szformat, ...)
		#define DbgPrintA(szformat, ...)
		#define DbgPrintW(szformat, ...)

		#define DbgOutA(szfilename, szformat, ...)
		#define DbgOutW(szfilename, szformat, ...)
		#define DbgOutHex(szfilename, szlog, length)

		#define DbgOutAEx(szfilename, nTimeVisible, bCRLF, szformat, ...)
		#define DbgOutWEx(szfilename, nTimeVisible, bCRLF, szformat, ...)
		#define DbgOutHexEx(szfilename, nTimeVisible, bCRLF, szformat, ...)

		#define DbgOutBinary(szfilename, szlog, length)

		#define EgDbgOutA(szgameid, szfilename, szformat, ...)
		#define EgDbgOutW(szgameid, szfilename, szformat, ...)
		#define EgDbgOutHex(szgameid, szfilename, szlog, length)

		#define EgDbgOutAEx(szgameid, szfilename, nTimeVisible, bCRLF, szformat, ...)
		#define EgDbgOutWEx(szgameid, szfilename, nTimeVisible, bCRLF, szformat, ...)
		#define EgDbgOutHexEx(szgameid, szfilename, nTimeVisible, bCRLF, szformat, ...)

		#define EgDbgOutBinary(szgameid, szfilename, szlog, length)
	#endif
}

