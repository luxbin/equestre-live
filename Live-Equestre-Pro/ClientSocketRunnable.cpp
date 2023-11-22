#include "stdafx.h"
#include <Poco/Exception.h>
#include "ClientSocketRunnable.h"
#include "MsgProcessor.h"

ClientSocketRunnable::ClientSocketRunnable()
	: m_bRunFlag(false)
	, m_bConnected(false)
{
}


ClientSocketRunnable::~ClientSocketRunnable()
{
}

void ClientSocketRunnable::run()
{
	string szCurServerAddr = "";

	while (m_bRunFlag) {

		// connect to server
		if (m_szServerAddress.empty()) {
			Poco::Thread::sleep(1000);
			continue;
		}

		try {
			Poco::Net::SocketAddress sa(Poco::Net::SocketAddress::Family::IPv4, m_szServerAddress.c_str(), 5408);
			m_socket.connect(sa);
		}
		catch (Poco::Exception& exc) {
			DbgOutA(ERROR_PATH, exc.displayText().c_str());
			Poco::Thread::sleep(1000);
			continue;
		}

		m_bConnected = true;
		szCurServerAddr = m_szServerAddress;
		MsgProcessorInst->Connected();

		// streamprocessor loop
		Poco::Timespan timeOut(0, 10000);   // 10 ms
		while (m_bRunFlag) {
			// flush send buffer
			FlushSendBuffer();

			if (szCurServerAddr != m_szServerAddress) {
				DbgOutA(ERROR_PATH, "Server address changed.");
				break;
			}

			char incommingBuffer[4096];

			if (m_socket.poll(timeOut, Poco::Net::Socket::SELECT_READ) == false) {
				continue;
			}

			// read stream
			int nBytes = -1;

			try {
				nBytes = m_socket.receiveBytes(incommingBuffer, sizeof(incommingBuffer));
				// DbgOutHex(PACKET_PATH, incommingBuffer, nBytes);
			}
			catch (Poco::Exception& exc) {
				//Handle your network errors.
				DbgOutA(ERROR_PATH, exc.displayText().c_str());
				break;
			}

			if (nBytes == 0) {
				DbgOutA(ERROR_PATH, "Client closes connection!");
				break;
			}

			// process recieved data
			MsgProcessorInst->AddToStream(incommingBuffer, nBytes);

		}

		MsgProcessorInst->Disconnected();
		// disconnect from server
		m_socket.close();
		m_bConnected = false;
	}
}



void ClientSocketRunnable::FlushSendBuffer()
{
	// There is no sending data now

// 	while (1) {
// 		string msg;
// 		if (m_pClient->getFromSendQueue(msg) == false)
// 			break;
// 
// 		m_socket.sendBytes(msg.c_str(), msg.length());
// 		m_socket.sendBytes("\x0A", 1);
// 	}

}
