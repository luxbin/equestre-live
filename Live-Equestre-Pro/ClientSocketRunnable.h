#pragma once

#include "Poco/Thread.h"
#include "Poco/Runnable.h"
#include "Poco/ErrorHandler.h"
#include <Poco/Net/StreamSocket.h>
#include <Poco/Net/SocketAddress.h>

#include <iostream>
#include <string>
using namespace std;

class ClientSocketRunnable : public Poco::Runnable
{
public:
	ClientSocketRunnable();
	virtual ~ClientSocketRunnable();

public:


protected:
	virtual void run();
	bool m_bRunFlag;
	bool m_bConnected;
	string		m_szServerAddress;
	Poco::Net::StreamSocket m_socket;
	
public:
	void SetRunFlag(bool bRun) { m_bRunFlag = bRun;  };
	bool IsConnected() { return m_bConnected; };

	void SetServerAddress(string szAddress)
	{
		m_szServerAddress = szAddress;
	}
	void FlushSendBuffer();
};

