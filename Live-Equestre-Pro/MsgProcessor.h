#pragma once

#include <Poco/JSON/Object.h>
#include <string>
#include <vector>
using namespace std;
using Poco::JSON::Object;

class MsgProcessor
{
public:
	static MsgProcessor* getInstance();

	void AddToStream(char* buff, int length);

protected:
	MsgProcessor();
	virtual ~MsgProcessor();

	// for jumping
	void OnRun(vector<string> params);
	void OnSync(vector<string> params);
	void OnTI0(vector<string> params);
	void OnTI1(vector<string> params);
	void OnLoadTI(vector<string> params);
	void OnAtStart(vector<string> params);
	void OnDNF(vector<string> params);
	void	Process(string command, vector<string> params);

	// for cross
	void OnCrossSTART_I(vector<string> params);
	void OnCrossEND_I(vector<string> params);

	void OnCrossRunc(vector<string> params);
	void OnCrossSync(vector<string> params);
	void OnCrossS(vector<string> params);
	void OnCrossR(vector<string> params);
	void OnCrossF(vector<string> params);
	void OnCrossNR(vector<string> params);
	void OnCrossTI0(vector<string> params);
	void	ProcessCross(string command, vector<string> params);

	vector<char>      m_arrBuffer;

	// status
	int		m_nNumber;
	int		m_nLane;
	bool	m_bRunning;
	int		m_nStartTime;
	int		m_nCurTime;
	int		m_nSyncUpdatedTick;
	bool	m_bJustLoaded;

public:
	void SendToWebSocket(Object::Ptr obj);
	void SendEventInfo();
	void SendHorseList();
	void SendRiderList();
	void SendStartList();
	void SendTeamList();
	void SendJudgeList();
	void SendCountry(std::string country);
	void SendRankingList();
	void SendXlsData(std::string name, std::string xlsContents);
	void SendPdf();
	void SendPdf(string eventId, string filename, string contents);
	string GetRankingList();
	void Disconnected();
	void Connected();
};

#define MsgProcessorInst		MsgProcessor::getInstance()


