#include "stdafx.h"
#include "MsgProcessor.h"
#include "base/stringutils.h"
#include "EquestreLogic.h"
#include "EquestreDb.h"
#include "Poco/DateTimeFormatter.h"
#include "Poco/Base64Encoder.h"
#include "Poco/Base64Decoder.h"


MsgProcessor::MsgProcessor()
{
	m_nNumber = 0;
	m_nLane = 1;
	m_bRunning = false;
	m_nStartTime = 0;
	m_nCurTime = m_nSyncUpdatedTick = 0;
	m_bJustLoaded = false;
}


MsgProcessor::~MsgProcessor()
{
}

MsgProcessor* MsgProcessor::getInstance()
{
	static MsgProcessor inst;
	return &inst;
}

DWORD g_rnd;
void _srand(DWORD seed)
{
	g_rnd = seed;
	g_rnd = 22695477 * g_rnd + 1;
}

int _rand()
{
	g_rnd = 22695477 * g_rnd + 1;
	return HIWORD(g_rnd) & 0x7FFF;
}

void MsgProcessor::AddToStream(char* buff, int length)
{
	m_arrBuffer.insert(m_arrBuffer.end(), buff, buff + length);

	while (1) {
		vector<char>::iterator it = std::find(m_arrBuffer.begin(), m_arrBuffer.end(), 0x0A);
		if (it == m_arrBuffer.end())
			break;

		vector<char> packet(m_arrBuffer.begin(), it + 1);
		m_arrBuffer.erase(m_arrBuffer.begin(), it + 1);

		// decrypt
		
		_srand(272727);
		for (int i = 0; i < packet.size() - 1; i++) {
			// int rnd = _rand();
			int rnd = 1 + (int)((long double)_rand() / (long double)(RAND_MAX + 1) * 3);
			// int k = rnd;
			packet[i] = packet[i] - rnd;
		}
		packet[packet.size() - 1] = 0;
		//DbgOutA(COMM_PATH, "packet=%s", packet.data());


		string msg((char *)packet.data());
		vector<string>   segments;
		ns_base::split(msg, ';', segments);

		if (segments.size() == 0) {
			continue;
		}

		if (segments[0] != "RUNC" && segments[0] != "SYNC") 
			DbgOutA(COMM_PATH, "%d,packet=%s", MyEquestreDb->m_discipline, packet.data());

		switch (MyEquestreDb->m_discipline)
		{
		case 0: // Jumping
			Process(segments[0], segments);
			break;
		case 2: // Cross
			ProcessCross(segments[0], segments);
			break;
		case 3: // Dressage
			Process(segments[0], segments);
			break;
		default:
			break;
		}

	};
}

//************** Jumping Process Start ********************************//

void MsgProcessor::Process(string command, vector<string> params)
{

	if (command == "LOADTI") {
		OnLoadTI(params);
	}
	else if (command == "RUN") {
		OnRun(params);
	}
	else if (command == "SYNC") {
		OnSync(params);
	}
	else if (command == "TI0") {
		OnTI0(params);
	}
	else if (command == "TI1") {
		OnTI1(params);
	}
	else if (command == "ATSTART") {
		OnAtStart(params);
	}
	else if (command == "DNF") {
		OnDNF(params);
	}
}

// LOADTI; timer start
void MsgProcessor::OnLoadTI(vector<string> params)
{
	MyEquestreDb->Reload();

	m_bJustLoaded = true;

	// send event info
	SendEventInfo();
	
	Poco::Thread::sleep(500);

	// start list
	SendStartList();

	// team list
	SendTeamList();

	// send horse list
	SendHorseList();

	// send rider list
	SendRiderList();


	// send ranking list
	SendRankingList();
}

void MsgProcessor::OnAtStart(vector<string> params)
{
	if (!m_bRunning)
		return;

	// ready to racing
//	MyEquestreDb->SetAtStart(params);

	Object::Ptr obj = new Object();
	obj->set("cmd", "atstart");
	Array	arr;
	for (int i = 1; i < params.size(); i++) {
		arr.add(params[i]);
	}
	obj->set("list", arr);

	SendToWebSocket(obj);
}

// DNF;number;dnf_code;
// dnf_code: 1(classified), 2(not present), 3(not started), 4(retired), 5(eliminated), 6(off-course), 7(disqualified)
void MsgProcessor::OnDNF(vector<string> params)
{
	int nNumber = stoi(params[1]);
	int nCode = stoi(params[2]);

	Object::Ptr obj = new Object();
	obj->set("cmd", "dnf");
	obj->set("num", nNumber);
	obj->set("pos", MyEquestreDb->getPosFromNum(nNumber));
	obj->set("code", nCode);
	
	SendToWebSocket(obj);
}


// RUN; number; isready; final_time; start_time; current_time(when paused); point; ___; lane; ___; sig; 
void MsgProcessor::OnRun(vector<string> params)
{
	m_nNumber = stoi(params[1]);
	int nPos = MyEquestreDb->getPosFromNum(m_nNumber);

 	m_nStartTime = stoi(params[4]);
	int nPausedTime = stoi(params[5]);
  	int bFinal = stoi(params[10]);
 	bool isRunning = (m_nStartTime != 0);
	m_nLane = abs(stoi(params[8]));
	//m_nLane = (stoi(params[8]) < 0) ? 1 : 2;

	int nTimeDelta = 0;
	if(m_nSyncUpdatedTick != 0)
		nTimeDelta = GetTickCount() - m_nSyncUpdatedTick;
	
	int nPoint = stoi(params[6]);

	if (bFinal) {
		m_bRunning = false;

		int nFinalTime = stoi(params[3]);

		Object::Ptr obj = new Object();
		obj->set("cmd", "final");
		obj->set("num", m_nNumber);
		obj->set("pos", nPos);
		obj->set("lane", m_nLane);
		obj->set("point", nPoint);
		obj->set("time", nFinalTime);

		SendToWebSocket(obj);

	}
	else if (m_nStartTime != 0) {
		m_bRunning = true;

		Object::Ptr obj = new Object();
		obj->set("cmd", "run");
		obj->set("num", m_nNumber);
		obj->set("pos", nPos);
		obj->set("lane", m_nLane);
		obj->set("point", nPoint);
		obj->set("time", m_nCurTime - m_nStartTime + nTimeDelta);
		obj->set("startTime", m_nStartTime);
		obj->set("pauseTime", nPausedTime);

		SendToWebSocket(obj);

	}
	else {
		m_bRunning = false;
		if (m_bJustLoaded) {

			Object::Ptr obj = new Object();
			obj->set("cmd", "ready");
			obj->set("num", m_nNumber);
			obj->set("pos", nPos);
			obj->set("lane", m_nLane);

			SendToWebSocket(obj);

			m_bJustLoaded = false;
		}
	}

}

// SYNC; cur_time;
void MsgProcessor::OnSync(vector<string> params)
{
	m_nCurTime = stoi(params[1]);
	m_nSyncUpdatedTick = GetTickCount();

// 	if (!m_bRunning)
// 		return; 	
// 	
// 	int duration = m_nCurTime - m_nStartTime;
// 	
// 	Object::Ptr obj = new Object();
// 	obj->set("cmd", "sync");
// 	obj->set("num", m_nNumber);
// 	obj->set("lane", m_nLane);
// 	obj->set("curTime", m_nCurTime);
// 	obj->set("time", duration);
// 
// 	SendToWebSocket(obj);

}

//[13144] packet = TI0;6;1;15400;0;0;0;0;0;0;

//[13144] packet=TI0;6;1;15400;0;0;0;0;0;1;
//[13144] packet=TI1;6;1;18190;0;0;0;0;0;1;


// TI0; 15; 0; 0; 0; 0; 0; 0; 0; 1;
void MsgProcessor::OnTI0(vector<string> params)
{
// 	m_pDlg->m_rcRunning.nNum = stoi(params[1]);

	SendRankingList();
}

// TI1; 15; 1; final_time; time_penalty; 0; 0; course_point; passing_point; 1;
void MsgProcessor::OnTI1(vector<string> params)
{
	int nNum = stoi(params[1]);
	int nPos = MyEquestreDb->getPosFromNum(nNum);

	int nFinalTime = stoi(params[3]);
	int nTimePenalty = stoi(params[4]);
	int nCoursePenalty = stoi(params[7]);
	int nFinalPoint = stoi(params[8]);

	Object::Ptr obj = new Object();
	obj->set("cmd", "timer1");
	obj->set("num", m_nNumber);
	obj->set("pos", nPos);
	obj->set("lane", m_nLane);
	obj->set("time", nFinalTime);
	obj->set("timePenalty", nTimePenalty);
	obj->set("point", nFinalPoint);
	obj->set("pointPenalty", nCoursePenalty);

	SendToWebSocket(obj);
	
}

//************ Jumping Process End *********************************//

//************ Cross Process Start *********************************//

void MsgProcessor::ProcessCross(string command, vector<string> params)
{
	if (command == "S") {
		OnCrossS(params);
	}
	else if (command == "RUNC") {
		OnCrossRunc(params);
	}
	else if (command == "SYNC") {
		OnCrossSync(params);
	}
	else if (command == "R") {
		OnCrossR(params);
	}
	else if (command == "F") {
		OnCrossF(params);
	}
	else if (command == "TI0") {
		OnCrossTI0(params);
	}
	else if (command == "NR") {
		OnCrossNR(params);
	}
	else if (command == "START_I") {
		OnCrossSTART_I(params);
	}
	else if (command == "END_I") {
		OnCrossEND_I(params);
	}

}

void MsgProcessor::OnCrossSTART_I(vector<string> params)
{
	Object::Ptr obj = new Object();
	obj->set("cmd", "CrossSTART_I");

	SendToWebSocket(obj);

}

void MsgProcessor::OnCrossEND_I(vector<string> params)
{
	Object::Ptr obj = new Object();
	obj->set("cmd", "CrossEND_I");

	SendToWebSocket(obj);

}


//RUNC; 2(num); 1; 0(course time finish set); 77469810(start time); 14500(pause time); 264800(total points); 17000(time penalty); 256800(passing point | passing time); 0; -1; 0; 0;
void MsgProcessor::OnCrossRunc(vector<string> params)
{

}

void MsgProcessor::OnCrossSync(vector<string> params)
{
	Object::Ptr obj = new Object();
	obj->set("cmd", "CrossSYNC");
	obj->set("sync_time", params[1]);

	if (MyEquestreDb->LoadCoursePoints()) {
		Array	list;
		map<int, int>::iterator it = MyEquestreDb->m_coursePoints.begin();
		for (; it != MyEquestreDb->m_coursePoints.end(); it++) {

			Object::Ptr record = new Object();

			record->set("num", it->first);
			record->set("course_point", it->second);

			list.add(record);
		}

		obj->set("list", list);
	}


	SendToWebSocket(obj);
}

/*
start list
S; 0; 1; 5; 6; 7; 8; 9; 11; 12; 13; 14; 15; 16; 17; 18; 19; 20; 21; 22; 23; 24; 25; 27; 28; 29; 31; 32; 33; 34; 35; 36; 37; 38; 39; 40; 41; 42; 43; 44; 45; 47; 48; 49; 50; 51; 52; 53; 54; 55; 56; 57;
running list
S; 1; 1; 4; 10; 3;
S; 2; 1;
S; 3; 1;
S; 4; 1;
S; 5; 1;
S; 6; 1;
S; 7; 1;
S; 8; 1;
S; 9; 1;
S; 10; 1;
*/
void MsgProcessor::OnCrossS(vector<string> params)
{
	if (params[1] != "0" && params[1] != "1")
		return;

	Object::Ptr obj = new Object();
	obj->set("cmd", "CrossS");
	obj->set("section", params[1]);

	Array	list;

	int nIdx = 0;
	Object::Ptr record = NULL;

	for (int i = 3; i < params.size(); i++) {

		record = new Object();
		record->set("num", params[i]);

		list.add(record);
	}

	obj->set("list", list);

	SendToWebSocket(obj);
}

// R;4;71441167(19:50:41 start time);10;71404102;3;71431919;
void MsgProcessor::OnCrossR(vector<string> params)
{
	Object::Ptr obj = new Object();
	obj->set("cmd", "CrossR");

	Array	list;

	int nIdx = 0;
	Object::Ptr record = NULL;

	for (int i = 1; i < params.size(); i++) {

		if (nIdx % 2 == 0) {
			record = new Object();
			record->set("num", params[i]);
		}
		else {
			record->set("start_time", params[i]);

			list.add(record);
		}
		nIdx++;
	}

	obj->set("list", list);

	SendToWebSocket(obj);
}

// F;2;71534622(19:52:14 finish time);1;71518006;
void MsgProcessor::OnCrossF(vector<string> params)
{
	Object::Ptr obj = new Object();
	obj->set("cmd", "CrossF");

	Array	list;

	int nIdx = 0;
	Object::Ptr record = NULL;

	for (int i = 1; i < params.size(); i++) {

		if (nIdx % 2 == 0) {
			record = new Object();
			record->set("num", params[i]);
		}
		else {
			record->set("finish_time", params[i]);

			list.add(record);
		}
		nIdx++;
	}

	obj->set("list", list);

	SendToWebSocket(obj);
}

/*
START_I
TI0; 1; 1; 101000(1:41 course time); 0; 0; 1099000(18:19 passing time); 0; 1084000(18:4); 0;
TI0; 2; 1; 108000(1:48s course tim); 0; 0; 1092000(18:12 passing time); 0; 1077000; 0;
TI0; 3; 0; 0; 0; 0; 0; 0; 0; 0;
TI0; 4; 0; 0; 0; 0; 0; 0; 0; 0;
TI0; 10; 0; 0; 0; 0; 0; 0; 0; 0;
END_I
*/
void MsgProcessor::OnCrossTI0(vector<string> params)
{
	Object::Ptr obj = new Object();
	obj->set("cmd", "CrossTI0");
	obj->set("num", params[1]);
	obj->set("is_finish", params[2]);
	obj->set("course_time", params[3]);
	obj->set("passing_time", params[6]);
	obj->set("anonymouse_time", params[8]);
	
	SendToWebSocket(obj);

	SendRankingList();
}
/*
IR;
NR; 0; 1; 10; 0; 0; 71404102000; 0; 0;; 0;;; // first start player start time
NR; 1; 1; 1; 0; 0; 71518006000; 0; 0;; 0;;;  // first end player end time
ER;
*/
void MsgProcessor::OnCrossNR(vector<string> params)
{
	/*
	Object::Ptr obj = new Object();
	obj->set("cmd", "CrossNR");
	obj->set("num", params[1]);

	SendToWebSocket(obj);
	*/
}

//************ Cross Process End *********************************//

void MsgProcessor::SendEventInfo()
{
	if (MyEquestreDb->m_bLoaded) {
		Object::Ptr obj = new Object();
		obj->set("cmd", "info");	
		obj->set("title", MyEquestreDb->m_szTitle);
		obj->set("eventTitle", MyEquestreDb->m_szEvent);
		obj->set("gameBeginTime", MyEquestreDb->m_gameBeginTime);
		obj->set("startDate", Poco::DateTimeFormatter::format(MyEquestreDb->m_dateStart, "%Y-%m-%d %H:%M:%S"));
		obj->set("endDate", Poco::DateTimeFormatter::format(MyEquestreDb->m_dateEnd, "%Y-%m-%d %H:%M:%S"));
		obj->set("eventDate", Poco::DateTimeFormatter::format(MyEquestreDb->m_dateEvent, "%Y-%m-%d %H:%M:%S"));
		obj->set("roundNumber", MyEquestreDb->m_roundNumber);
		obj->set("jumpoffNumber", MyEquestreDb->m_jumpoffNumber);
		obj->set("twoPhaseIntegrated", MyEquestreDb->m_twoPhaseIntegrated);
		obj->set("twoPhaseDiffered", MyEquestreDb->m_twoPhaseDiffered);
		obj->set("round", MyEquestreDb->m_round);
		obj->set("discipline", MyEquestreDb->m_discipline);
		obj->set("eventing", MyEquestreDb->m_eventing);
		obj->set("jumpoff", MyEquestreDb->m_jumpoff);
		obj->set("country", MyEquestreDb->m_country);

		obj->set("crossOptimumTime", MyEquestreDb->m_crossOptimumTime);
		obj->set("crossMinimumTime", MyEquestreDb->m_crossMinimumTime);
		obj->set("crossLimitTime", MyEquestreDb->m_crossLimitTime);

		obj->set("crossSurpassingBaseTime", MyEquestreDb->m_crossSurpassingBaseTime);
		obj->set("crossSurpassingPoints", MyEquestreDb->m_crossSurpassingPoints);
		obj->set("crossSurpassingMinBaseTime", MyEquestreDb->m_crossSurpassingMinBaseTime);
		obj->set("crossSurpassingMinPoints", MyEquestreDb->m_crossSurpassingMinPoints);

		obj->set("schedulerNumber", MyEquestreDb->m_schedulerNumber);
		obj->set("category", MyEquestreDb->m_category);
		obj->set("notes", MyEquestreDb->m_notes);
		obj->set("height", MyEquestreDb->m_height);
		obj->set("initAwardListAmount", MyEquestreDb->m_initAwardListAmount);

		int timeEvent = MyEquestreDb->m_timeEvent / 1000 / 60;
		char szStr[256];
		sprintf(szStr, "%02d:%02d", timeEvent / 60, timeEvent % 60);
		obj->set("eventTime", szStr);



		obj->set("modeTeam", MyEquestreDb->m_modeTeam); // Nation Cup
		obj->set("modeTeamRelay", MyEquestreDb->m_modeTeamRelay); // Team Cup


		SendToWebSocket(obj);

		DbgOutA(COMM_PATH, "******* SendEventInfo");
		
	}
}


void MsgProcessor::SendHorseList()
{
	if (!MyEquestreDb->m_bLoaded)
		return;

	Object::Ptr obj = new Object();
	obj->set("cmd", "horses");

	Array	list;
	map<int, Horse>::iterator it = MyEquestreDb->m_horses.begin();
	for (; it != MyEquestreDb->m_horses.end(); it++) {
		Object::Ptr record = new Object();

		record->set("idx", it->second.m_nIdx);
		record->set("name", it->second.m_szName);
		record->set("age", it->second.m_nAge);
		record->set("birthday", it->second.m_dateBirthday);
		record->set("owner", it->second.m_szOwner);
		record->set("passport", it->second.m_szFnchPassport);
		record->set("gender", it->second.m_szGender);
		record->set("father", it->second.m_szFather);
		record->set("mother", it->second.m_szMother);
		record->set("fatherOfMother", it->second.m_szFatherOfMother);
		record->set("signalementLabel", it->second.m_szSignalementLabel);

		list.add(record);
	}

	obj->set("list", list);
	SendToWebSocket(obj);

	DbgOutA(COMM_PATH, "******* SendHorseList");
	
}


void MsgProcessor::SendRiderList()
{
	if (!MyEquestreDb->m_bLoaded)
		return;

	Object::Ptr obj = new Object();
	obj->set("cmd", "riders");

	Array	list;
	map<int, Rider>::iterator it = MyEquestreDb->m_riders.begin();
	for (; it != MyEquestreDb->m_riders.end(); it++) {
		Object::Ptr record = new Object();

		record->set("idx", it->second.m_nIdx);
		record->set("firstName", it->second.m_szFirstName);
		record->set("lastName", it->second.m_szLastName);
		record->set("nation", it->second.m_szNation);
		record->set("birthday", it->second.m_dateBirthday);
		record->set("city", it->second.m_szCity);
		record->set("license", it->second.m_szLicense);
		record->set("club", it->second.m_szClub);

		list.add(record);
	}

	obj->set("list", list);
	SendToWebSocket(obj);

	DbgOutA(COMM_PATH, "******* SendRiderList");
	
}

void MsgProcessor::SendJudgeList()
{
	if (!MyEquestreDb->m_bLoaded || MyEquestreDb->m_discipline != 3/*Dressage*/)
		return;

	Object::Ptr obj = new Object();
	obj->set("cmd", "judges");

	Array	list;
	map<int, Judge>::iterator it = MyEquestreDb->m_judges.begin();
	for (; it != MyEquestreDb->m_judges.end(); it++) {
		Object::Ptr record = new Object();

		record->set("index", it->second.m_nIndex);
		record->set("index", it->second.m_nIndex);
		record->set("position", it->second.m_szPosition);
		record->set("name", it->second.m_szName);
		record->set("nation", it->second.m_szNation);
		record->set("code", it->second.m_nCode);

		list.add(record);
	}

	obj->set("list", list);
	SendToWebSocket(obj);

	DbgOutA(COMM_PATH, "******* SendRiderList");
}

void MsgProcessor::SendStartList()
{
	if (!MyEquestreDb->m_bLoaded)
		return;

	Object::Ptr obj = new Object();
	obj->set("cmd", "startlist");

	Array	list;
	int index = 0;
	std::vector<StartListEntry>::iterator it = MyEquestreDb->m_startlist.begin();
	for (; it != MyEquestreDb->m_startlist.end(); it++) {
		Object::Ptr record = new Object();
		record->set("pos", index++);
		record->set("num", it->m_nNumber);
		record->set("horse_idx", it->m_nHorseIdx);
		record->set("rider_idx", it->m_nRiderIdx);
		record->set("start_time", it->m_nStartTime);

		list.add(record);
	}

	Array lstCompetitors;
	index = 0;

	map<int, NumberToIndexEntry>::iterator it1 = MyEquestreDb->m_numberIndice.begin();
	for (; it1 != MyEquestreDb->m_numberIndice.end(); it1++) {
		Object::Ptr record = new Object();
		record->set("num", it1->second.m_nNumber);
		record->set("horse_idx", it1->second.m_nHorseIdx);
		record->set("rider_idx", it1->second.m_nRiderIdx);

		lstCompetitors.add(record);
	}

	obj->set("list", list);
	obj->set("competitors", lstCompetitors);

	SendToWebSocket(obj);

	DbgOutA(COMM_PATH, "******* SendStartList");
	
}

void MsgProcessor::SendTeamList()
{
	if (!MyEquestreDb->m_bLoaded)
		return;

	Object::Ptr obj = new Object();
	obj->set("cmd", "teams");

	Array	list;
	map<string, Team>::iterator it = MyEquestreDb->m_teams.begin();
	for (; it != MyEquestreDb->m_teams.end(); it++) {
		Object::Ptr record = new Object();

		Array members;
		for (int i = 0; i < 5; i++) {
			members.add(it->second.m_nMembers[i]);
		}

		record->set("num", it->second.m_nNum * 100);
		record->set("name", it->second.m_szName);
		record->set("members", members);

		list.add(record);
	}

	obj->set("list", list);
	SendToWebSocket(obj);

	DbgOutA(COMM_PATH, "******* SendTeamList");

}

void MsgProcessor::SendCountry(string country)
{
	if (!MyEquestreDb->m_bLoaded)
		return;
	Object::Ptr obj = new Object();
	obj->set("cmd", "country");
	obj->set("country", country);
	SendToWebSocket(obj);
}

string MsgProcessor::GetRankingList()
{

	MyEquestreDb->Reload();

	if (!MyEquestreDb->m_bLoaded)
		return "";

	MyEquestreDb->UpdateRanking();

	Object::Ptr obj = new Object();
	obj->set("cmd", "ranking");

	Array   roundScore;
	Array   jumpoffScore;

	int     roundCount = MyEquestreDb->m_roundNumber;
	int     jumpoffCount = MyEquestreDb->m_jumpoffNumber;

	for (int i = 0; i < 4; i++) {
		map<int, Ranking>::iterator it = MyEquestreDb->m_roundRanks[i].begin();
		Array rScore;
		for (; it != MyEquestreDb->m_roundRanks[i].end(); it++) {
			if (it->second.m_nStatus == 0) { continue; }
			Object::Ptr record = new Object();

			record->set("pos", MyEquestreDb->getPosFromNum(it->second.m_nNum));
			record->set("num", it->second.m_nNum);

			record->set("time", it->second.m_nTimes);
			record->set("timePlus", it->second.m_nTimePenalty);

			record->set("figureMark", it->second.m_nFigureMark);

			record->set("point", it->second.m_nPoints);
			record->set("pointPlus", it->second.m_nPointsSurpassing);
			if (it->second.m_nStatus > 1) {
				record->set("point", -it->second.m_nStatus);
			}
			record->set("status", it->second.m_nStatus);
			record->set("judgeScores", it->second.m_szJudgeScores);

			rScore.add(record);
		}
		roundScore.add(rScore);

		it = MyEquestreDb->m_jumpoffRanks[i].begin();
		Array jScore;
		for (; it != MyEquestreDb->m_jumpoffRanks[i].end(); it++) {
			if (it->second.m_nStatus == 0) { continue; }
			Object::Ptr record = new Object();

			record->set("pos", MyEquestreDb->getPosFromNum(it->second.m_nNum));
			record->set("num", it->second.m_nNum);

			record->set("time", it->second.m_nTimes);
			record->set("timePlus", it->second.m_nTimePenalty);

			record->set("point", it->second.m_nPoints);
			record->set("pointPlus", it->second.m_nPointsSurpassing);
			if (it->second.m_nStatus > 1) {
				record->set("point", -it->second.m_nStatus);
			}
			jScore.add(record);
		}
		jumpoffScore.add(jScore);
	}

	Array roundTableTypes;
	Array jumpoffTableTypes;
	Array againstTimeClockRounds;
	Array againstTimeClockJumpoffs;
	Array allowedTimeRounds;
	Array allowedTimeJumpoffs;

	for (int i = 0; i < 4; i++) {
		roundTableTypes.add(MyEquestreDb->m_roundTableTypes[i]);
		jumpoffTableTypes.add(MyEquestreDb->m_jumpoffTableTypes[i]);
		againstTimeClockRounds.add(MyEquestreDb->m_againstTimeClockRounds[i]);
		againstTimeClockJumpoffs.add(MyEquestreDb->m_againstTimeClockJumpoffs[i]);
		allowedTimeRounds.add(MyEquestreDb->m_allowedTimeRounds[i]);
		allowedTimeJumpoffs.add(MyEquestreDb->m_allowedTimeJumpoffs[i]);
	}

	obj->set("roundScore", roundScore);
	obj->set("jumpoffScore", jumpoffScore);
	obj->set("roundCount", roundCount);
	obj->set("jumpoffCount", jumpoffCount);
	obj->set("roundTableTypes", roundTableTypes);
	obj->set("jumpoffTableTypes", jumpoffTableTypes);
	obj->set("allowedTimeRounds", allowedTimeRounds);
	obj->set("allowedTimeJumpoffs", allowedTimeJumpoffs);
	obj->set("againstTimeClockRounds", againstTimeClockRounds);
	obj->set("againstTimeClockJumpoffs", againstTimeClockJumpoffs);
	obj->set("round", MyEquestreDb->m_round);
	obj->set("jumpoff", MyEquestreDb->m_jumpoff);
	obj->set("discipline", MyEquestreDb->m_discipline);
	obj->set("twoPhaseIntegrated", MyEquestreDb->m_twoPhaseIntegrated);
	obj->set("twoPhaseDiffered", MyEquestreDb->m_twoPhaseDiffered);

	std::ostringstream oss;
	obj->stringify(oss, 2);

	return oss.str();
}

void MsgProcessor::SendRankingList()
{
	if (!MyEquestreDb->m_bLoaded)
		return;

	{
		MyEquestreDb->UpdateRanking();

		Object::Ptr obj = new Object();
		obj->set("cmd", "ranking");

		Array   roundScoreList;
		Array   jumpoffScoreList;
		int     roundCount = MyEquestreDb->m_roundNumber;
		int     jumpoffCount = MyEquestreDb->m_jumpoffNumber;

		for (int i = 0; i < 4; i++) {
			map<int, Ranking>::iterator it = MyEquestreDb->m_roundRanks[i].begin();
			Array	roundScore;
			for (; it != MyEquestreDb->m_roundRanks[i].end(); it++) {
				if (it->second.m_nStatus == 0) { continue; }
				Object::Ptr record = new Object();

				record->set("pos", MyEquestreDb->getPosFromNum(it->second.m_nNum));
				record->set("num", it->second.m_nNum);

				record->set("time", it->second.m_nTimes);
				record->set("timePlus", it->second.m_nTimePenalty);

				record->set("figureMark", it->second.m_nFigureMark);

				record->set("point", it->second.m_nPoints);
				record->set("pointPlus", it->second.m_nPointsSurpassing);
				if (it->second.m_nStatus > 1) {
					int nStatus = it->second.m_nStatus;
					if (nStatus == 6) nStatus = 2;
					else if (nStatus == 2) nStatus = 6;
					record->set("point", -nStatus);
				}
				record->set("status", it->second.m_nStatus);
				record->set("judgeScores", it->second.m_szJudgeScores);

				roundScore.add(record);
			}
			roundScoreList.add(roundScore);

			it = MyEquestreDb->m_jumpoffRanks[i].begin();
			Array	jumpoffScore;
			for (; it != MyEquestreDb->m_jumpoffRanks[i].end(); it++) {
				if (it->second.m_nStatus == 0) { continue; }
				Object::Ptr record = new Object();

				record->set("pos", MyEquestreDb->getPosFromNum(it->second.m_nNum));
				record->set("num", it->second.m_nNum);

				record->set("time", it->second.m_nTimes);
				record->set("timePlus", it->second.m_nTimePenalty);

				record->set("point", it->second.m_nPoints);
				record->set("pointPlus", it->second.m_nPointsSurpassing);
				if (it->second.m_nStatus > 1) {
					int nStatus = it->second.m_nStatus;
					if (nStatus == 6) nStatus = 2;
					else if (nStatus == 2) nStatus = 6;
					record->set("point", -nStatus);
				}
				jumpoffScore.add(record);
			}
			jumpoffScoreList.add(jumpoffScore);
		}

		Array roundTableTypes;
		Array jumpoffTableTypes;
		Array againstTimeClockRounds;
		Array againstTimeClockJumpoffs;
		Array allowedTimeRounds;
		Array allowedTimeJumpoffs;
		for (int i = 0; i < 4; i++) {
			roundTableTypes.add(MyEquestreDb->m_roundTableTypes[i]);
			jumpoffTableTypes.add(MyEquestreDb->m_jumpoffTableTypes[i]);
			againstTimeClockRounds.add(MyEquestreDb->m_againstTimeClockRounds[i]);
			againstTimeClockJumpoffs.add(MyEquestreDb->m_againstTimeClockJumpoffs[i]);
			allowedTimeRounds.add(MyEquestreDb->m_allowedTimeRounds[i]);
			allowedTimeJumpoffs.add(MyEquestreDb->m_allowedTimeJumpoffs[i]);
		}

		obj->set("round_score", roundScoreList);
		obj->set("jumpoff_score", jumpoffScoreList);
		obj->set("round_count", roundCount);
		obj->set("jumpoff_count", jumpoffCount);
		obj->set("round_table_types", roundTableTypes);
		obj->set("jumpoff_table_types", jumpoffTableTypes);
		obj->set("allowed_time_rounds", allowedTimeRounds);
		obj->set("allowed_time_jumpoffs", allowedTimeJumpoffs);
		obj->set("against_time_clock_rounds", againstTimeClockRounds);
		obj->set("against_time_clock_jumpoffs", againstTimeClockJumpoffs);
		obj->set("round", MyEquestreDb->m_round);
		obj->set("jumpoff", MyEquestreDb->m_jumpoff);
		obj->set("discipline", MyEquestreDb->m_discipline);
		obj->set("two_phase_integrated", MyEquestreDb->m_twoPhaseIntegrated);
		obj->set("two_phase_differed", MyEquestreDb->m_twoPhaseDiffered);
		SendToWebSocket(obj);


		DbgOutA(COMM_PATH, "******* SendRankingList");

	}

	if (!MyEquestreDb->m_eventing) return;

	{
		MyEquestreDb->UpdateCCRanking();

		Object::Ptr obj = new Object();
		obj->set("cmd", "cc-ranking");
		{
			map<int, CCRanking>::iterator it = MyEquestreDb->m_ccRanks.begin();
			Array	ccRanks;
			for (; it != MyEquestreDb->m_ccRanks.end(); it++) {
				//if (it->second.m_nJumpStatus <= 1 && it->second.m_nCrossStatus <= 1 && it->second.m_nDressageStatus) {
					//if (it->second.m_nStatus == 0) { continue; }
					Object::Ptr record = new Object();

					record->set("num", it->second.m_nNum);
					record->set("horse_idx", it->second.m_nHorseIdx);
					record->set("rider_idx", it->second.m_nRiderIdx);
					record->set("jump_point", it->second.m_nJumpPoints);
					record->set("jump_time", it->second.m_nJumpTimes);
					record->set("jump_status", it->second.m_nJumpStatus);
					record->set("cross_point", it->second.m_nCrossPoints);
					record->set("cross_time", it->second.m_nCrossTimes);
					record->set("cross_status", it->second.m_nCrossStatus);
					record->set("dressage_point", 100000 - it->second.m_nDressagePoints);
					record->set("dressage_status", it->second.m_nDressageStatus);
					int nTotalPoint = 100000 - it->second.m_nDressagePoints + it->second.m_nJumpPoints + it->second.m_nCrossPoints;
					record->set("total_point", nTotalPoint);

					ccRanks.add(record);
				//}
			}
			obj->set("ccRanks", ccRanks);
		}

		SendToWebSocket(obj);

		DbgOutA(COMM_PATH, "******* SendCCRankingList");
	}

}
#include <fstream>
void MsgProcessor::SendXlsData(std::string name, std::string xlsContents)
{
	Object::Ptr obj = new Object();
	obj->set("cmd", "xls");
	obj->set("name", name);
	obj->set("contents", xlsContents);

	SendToWebSocket(obj);
}

void MsgProcessor::SendPdf()
{
	int printnum = 0;
	std::string title;
	std::string contents;

	if (!MyEquestreDb->GetUploadedPdfInfo(printnum, title, contents)) return;

	Object::Ptr obj = new Object();
	obj->set("cmd", "pdf");
	obj->set("discipline", MyEquestreDb->m_discipline);
	obj->set("printnum", printnum);
	obj->set("title", title);
	obj->set("contents", contents);

	SendToWebSocket(obj);
}

void MsgProcessor::SendPdf(string eventId, string filename, string contents)
{
	Object::Ptr obj = new Object();
	obj->set("cmd", "pdf");
	obj->set("type", "another");
	
	obj->set("eventid", eventId);
	obj->set("title", filename);
	obj->set("contents", contents);

	SendToWebSocket(obj);
}

void MsgProcessor::SendToWebSocket(Object::Ptr obj)
{
	std::ostringstream oss;
	obj->stringify(oss, 2);

	MyEquestreLogic->GetWebSocketRunnable().AddToSendQueue(oss.str());

}

void MsgProcessor::Disconnected()
{
	Object::Ptr obj = new Object();
	obj->set("cmd", "exit");

	SendToWebSocket(obj);
}


void MsgProcessor::Connected()
{
	MyEquestreDb->Reload();
	

	Poco::Thread::sleep(1000);

	// send event info
	SendEventInfo();

	Poco::Thread::sleep(5000);
	

	SendStartList();

	// send start list
	SendTeamList();

	// send horse list
	SendHorseList();

	// send rider list
	SendRiderList();

	// send judge list for Dressage
	SendJudgeList();

	// send ranking list
	SendRankingList();
}
