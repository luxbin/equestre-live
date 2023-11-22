#pragma once

#include "Poco/Thread.h"
#include "Poco/Runnable.h"
#include "Poco/ErrorHandler.h"
#include <Poco/JSON/Object.h>
#include "../SocketIO-poco/SIOClient.h"
#include <list>
#include <string>
using namespace std;

class WebSocketRunnable;

class WebSockEventTarget : public SIOEventTarget
{
public:
	WebSockEventTarget(WebSocketRunnable* pRunnable);

	void onUpdate(const void* pSender, Poco::JSON::Object::Ptr& arg);
	void onDisconnect(const void* pSender, Poco::JSON::Object::Ptr& arg);

protected:
	WebSocketRunnable*	m_pRunnable;

};

//////////////////////////////////////////////////////////////////////////

class WebSocketRunnable : public Poco::Runnable
{
public:
	WebSocketRunnable();
	~WebSocketRunnable();

protected:
	void run();

	bool m_bRunFlag;
	bool m_bConnected;
	string	m_szServerUri;

	SIOClient *m_sio;
	WebSockEventTarget	m_sioTarget;

public:

	void SetRunFlag(bool bRun) { m_bRunFlag = bRun; };
	bool IsConnected() { return m_bConnected; };
	string GetServerUri() {
		return m_szServerUri;
	};

protected:
	bool m_bSendFlag;
public:

	void SetSendFlag(bool bSend)
	{
		m_bSendFlag = bSend;
	}

	bool IsSendFlag()
	{
		return m_bSendFlag;
	}

public:
	void	AddToSendQueue(string msg);
	void	ClearSendQueue();

protected:
	bool	FlushSendBuffer();
	bool	GetFromSendQueue(string& msg);
	list<string>			m_sendMessage;
	Poco::Mutex				m_sendMutex;
};

