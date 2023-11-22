#pragma once
#include "ClientSocketRunnable.h"
#include "WebSocketRunnable.h"

#include <string>
using namespace std;

class EquestreLogic
{
protected:
	EquestreLogic();
	virtual ~EquestreLogic();


public:
	static EquestreLogic* getInstance();

	void StartClientSocketThread();
	void StopClientSocketThread();
	bool IsClientSocketConnected();
	void SetClientSocketServerAddr(string szAddress);

protected:

	Poco::Thread			m_threadClientSocket;
	ClientSocketRunnable	m_runableClientSocket;

public:
	void StartWebSocketThread();
	void StopWebSocketThread();
	bool IsWebSocketConnected();
	bool IsWebSocketThreadStarted() { return m_threadWebSocket.isRunning(); };

	string GetWebSocketUri() { return m_runableWebSocket.GetServerUri(); };
	WebSocketRunnable& GetWebSocketRunnable() { return m_runableWebSocket; };

protected:
	Poco::Thread			m_threadWebSocket;
	WebSocketRunnable		m_runableWebSocket;

public:
	void Start();
	void Stop();
	bool IsStarted();
};

#define MyEquestreLogic		EquestreLogic::getInstance()


