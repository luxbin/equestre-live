#include "stdafx.h"
#include "WebSocketRunnable.h"
#include "Live-Equestre-Pro.h"
#include "base/stringutils.h"
#include "EquestreDb.h"
#include "MsgProcessor.h"

CString loadConfig() {
	CFile f;
	if (!PathFileExists(L"config.ini")) {
		return L"http://127.0.0.1:21741";
	}
	f.Open(L"config.ini", CFile::modeRead);
	char pBuf[100];
	int len = f.GetLength();
	f.Read(pBuf, len);
	pBuf[len] = 0;
	return CString(pBuf);
}

WebSockEventTarget::WebSockEventTarget(WebSocketRunnable* pRunnable)
{
	m_pRunnable = pRunnable;
}

void WebSockEventTarget::onUpdate(const void* pSender, Poco::JSON::Object::Ptr& arg)
{
	try {
//		Poco::JSON::Object& obj = arg[0];

// 		bool b = obj.isArray();
// 		b = obj.isArray();
// 		b = obj.isArray();
// 		b = obj.isArray();

		
		// std::string cmd = obj.isArray(); ["cmd"];
// 		std::string result = obj.getValue<std::string>("status");

// 		Poco::JSON::Object data = obj["data"]; //  arg->getObject("data");
// 
// 		if (cmd == "info") {
// 			// int id = data.getValue<int>("id");
// 			int k = 0;
// 		}
		int k = 0;
	}
	catch (Poco::Exception& ex) {
		int i = 0;
	}

}

void WebSockEventTarget::onDisconnect(const void* pSender, Poco::JSON::Object::Ptr& arg)
{
	int k = 0;
}

//////////////////////////////////////////////////////////////////////////
WebSocketRunnable::WebSocketRunnable()
	: m_bRunFlag(false)
	, m_bConnected(false)
	, m_bSendFlag(false)
	, m_sio(NULL)
	, m_sioTarget(this)
{
}


WebSocketRunnable::~WebSocketRunnable()
{
}

void WebSocketRunnable::run()
{
	CString strPath = loadConfig();
	string uri = ns_base::tostring(strPath.GetBuffer());

	while (m_bRunFlag) {
		// connecting
		DbgOutA(DEBUG_PATH, "%s: connecting to %s", __FUNCTION__, uri.c_str());
		m_sio = SIOClient::connect(uri);

		if (m_sio == NULL) {
			Poco::Thread::sleep(1000);
			continue;
		}

		ClearSendQueue();

		try {
			m_szServerUri = uri;
			m_bConnected = true;
			DbgOutA(DEBUG_PATH, "%s: connected", __FUNCTION__);

			m_sio->emit("subscribe", "provider");

// 			m_sio->on("push", &m_sioTarget, callback(&WebSockEventTarget::onUpdate));
// 			m_sio->on("disconnect", &m_sioTarget, callback(&WebSockEventTarget::onDisconnect));

			Poco::Timestamp tsPrev;
			while (m_bRunFlag) {
				MsgProcessorInst->SendPdf();

				if (FlushSendBuffer() == false) {
					DbgOutA(ERROR_PATH, "socketio failed");
					break;
				}

				Poco::Timestamp now;
				if (now - Poco::Timespan(5, 0) > tsPrev) {
					m_sio->send("heartbeat");
					tsPrev = now;
				}

				Poco::Thread::sleep(10);
			}

			DbgOutA(DEBUG_PATH, "%s: disconnect with normal", __FUNCTION__);
			m_sio->disconnect();
		}
		catch (Poco::Exception&) {
			DbgOutA(DEBUG_PATH, "%s: disconnect with exception", __FUNCTION__);
			m_sio->disconnect();
		}

		m_szServerUri = "";
		m_bConnected = false;
		ClearSendQueue();
	}
}


bool WebSocketRunnable::FlushSendBuffer()
{
	while (1) {
		string msg;
		if (GetFromSendQueue(msg) == false)
			break;
	
		//if (std::strstr(msg.c_str(), "run") == nullptr && std::strstr(msg.c_str(), "sync") == nullptr)
		//	DbgOutA(WEBAPI_PATH, ">>> %s", msg.c_str());

		try {
			if (MyEquestreDb->m_discipline == 0) // Jumping
				m_sio->emit("push", msg);
			else if(MyEquestreDb->m_discipline == 2) // Cross
				m_sio->emit("push_cross", msg);
			else if(MyEquestreDb->m_discipline == 3) // Dressage
				m_sio->emit("push", msg);
		}
		catch (Poco::Exception&) {
			return false;
		}
	}
	return true;
}

void	WebSocketRunnable::AddToSendQueue(string msg)
{
	if (m_bConnected == false)
		return;

	Poco::Mutex::ScopedLock lock(m_sendMutex);
	if (m_bSendFlag) {
		m_sendMessage.push_back(msg);
	}
}

void	WebSocketRunnable::ClearSendQueue()
{
	Poco::Mutex::ScopedLock lock(m_sendMutex);

	m_sendMessage.clear();
}

bool	WebSocketRunnable::GetFromSendQueue(string& msg)
{
	Poco::Mutex::ScopedLock lock(m_sendMutex);

	if (m_sendMessage.size() == 0)
		return false;

	msg = m_sendMessage.front();
	m_sendMessage.pop_front();
	return true;
}

