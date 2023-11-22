#include "stdafx.h"
#include "EquestreLogic.h"


EquestreLogic::EquestreLogic()
{
}


EquestreLogic::~EquestreLogic()
{
	StopClientSocketThread();
	StopWebSocketThread();
}


void EquestreLogic::StartClientSocketThread()
{
	if (!m_threadClientSocket.isRunning()) {
		m_runableClientSocket.SetRunFlag(true);
		m_threadClientSocket.start(m_runableClientSocket);
	}
}


void EquestreLogic::StopClientSocketThread()
{
	if (m_threadClientSocket.isRunning()) {
		m_runableClientSocket.SetRunFlag(false);
		m_threadClientSocket.join();
	}
}


bool EquestreLogic::IsClientSocketConnected()
{
	return m_runableClientSocket.IsConnected();
}


void EquestreLogic::SetClientSocketServerAddr(string szAddress)
{
	m_runableClientSocket.SetServerAddress(szAddress);
}


EquestreLogic* EquestreLogic::getInstance()
{
	static EquestreLogic inst;
	return &inst;
}

////
void EquestreLogic::StartWebSocketThread()
{
	if (!m_threadWebSocket.isRunning()) {
		m_runableWebSocket.SetRunFlag(true);
		m_threadWebSocket.start(m_runableWebSocket);
	}
}

void EquestreLogic::StopWebSocketThread()
{
	if (m_threadWebSocket.isRunning()) {
		m_runableWebSocket.SetRunFlag(false);
		m_threadWebSocket.join();
	}
}

bool EquestreLogic::IsWebSocketConnected()
{
	return m_runableWebSocket.IsConnected();
}


void EquestreLogic::Start()
{
	m_runableWebSocket.SetSendFlag(true);
}


void EquestreLogic::Stop()
{
	m_runableWebSocket.SetSendFlag(false);
}


bool EquestreLogic::IsStarted()
{
	return m_runableWebSocket.IsSendFlag();
}
