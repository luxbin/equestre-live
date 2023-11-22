#include "stdafx.h"
#include "EquestreDb.h"

#include "Poco/Data/Session.h"
#include <Poco/Exception.h>
#include "Poco/Data/SQLite/Connector.h"
#include "Poco/File.h"
#include <vector>
#include <iostream>
#include <regex>

#include "base/stringutils.h"

using namespace Poco::Data::Keywords;
using Poco::Data::Session;
using Poco::Data::Statement;
using namespace Poco;

EquestreDb::EquestreDb()
	: m_bLoaded(false)
	, m_eventNo(0)
{
	m_manualDiscipline = 0;
	m_publishUploadFiles = 0;
	Poco::Data::SQLite::Connector::registerConnector();
}


EquestreDb::~EquestreDb()
{
}

#include <fstream>
#include "Poco/Base64Encoder.h"

bool EquestreDb::GetUploadedPdfInfo(int& printNum, std::string& title, std::string& contents)
{
	int curFiles = GetPublishFilesUpload();

	if (m_publishUploadFiles == curFiles) return false;

	m_publishUploadFiles = curFiles;

	if (!m_bLoaded) return false;

	// open main meeting file

	try {

		string fileName;

		string db = ns_base::string_format("\\Show%03d.scdb", m_showNo);
		string subFile = m_szDbFolder + db;
		Session sessionSub("SQLite", subFile);

		typedef Tuple<int, string, string> FilesUploadTuple;
		std::vector<FilesUploadTuple> files;
		sessionSub << "SELECT C_PRINT_NUM, C_PRINT_TITLE, C_FILENAME FROM TPUBLISH_FILES_UPLOAD ORDER BY C_DATE_ADDED DESC, C_HOUR_ADDED DESC;", into(files), now;

		for each (FilesUploadTuple record in files)
		{
			printNum = record.get<0>();
			title = record.get<1>();
			fileName = record.get<2>();
			
			break;
		}

		std::string source = m_szDbFolder + "\\" + fileName;

		std::ifstream in(source, ios::binary); //
		//std::istringstream in(source);
		std::ostringstream out;
		Poco::Base64Encoder b64out(out);

		//std::copy(std::istream_iterator<char>{in},
		//	std::istream_iterator<char>{},
		//	std::ostream_iterator<char>(b64out)
		//);


		while (!in.eof()) {
			char ch;
			in.get(ch);
			b64out << ch;
		}

		/*std::copy(std::istreambuf_iterator< char >(in),
			std::istreambuf_iterator< char >(),
			std::ostreambuf_iterator< char >(b64out));
			*/

		
		b64out.close(); // always call this at the end!

		contents = out.str();

		return true;
	}
	catch (Exception& e) {
		
	}

	return false;
}

int EquestreDb::GetPublishFilesUpload()
{
	if (!m_bLoaded) return 0;

	// open main meeting file

	int publishUploadFiles = 0;

	try {

		string db = ns_base::string_format("\\Show%03d.scdb", m_showNo);
		string subFile = m_szDbFolder + db;
		Session sessionSub("SQLite", subFile);

		typedef Tuple<int> FilesUploadTuple;
		std::vector<FilesUploadTuple> files;
		sessionSub << "SELECT UPDATEVALUE FROM TUPDATEVALUES WHERE TABLENAME='TPUBLISH_FILES_UPLOAD'", into(files), now;

		for each (FilesUploadTuple record in files)
		{
			publishUploadFiles = record.get<0>();
		}
	}
	catch (Exception& e) {
		return 0;
	}

	return publishUploadFiles;
}

bool EquestreDb::Load(std::string folder)
{
	
	try {
		// check file exist
		Poco::File file(folder + "\\Online.scdb");
		if (!file.exists()) {
			m_bLoaded = false;
			return false;
		}


		Session session("SQLite", folder + "\\Online.scdb");
		session << "SELECT C_VALUE from TPARAMETERS where C_PARAM='FILE'", into(m_mainFile), now;
		session << "SELECT C_VALUE from TPARAMETERS where C_PARAM='EVENT'", into(m_eventNo), now;
		session << "SELECT C_VALUE from TPARAMETERS where C_PARAM='SHOW'", into(m_showNo), now;
		session << "SELECT C_VALUE from TPARAMETERS where C_PARAM='ROUND'", into(m_round), now;
		session << "SELECT C_VALUE from TPARAMETERS where C_PARAM='JUMPOFF'", into(m_jumpoff), now;
		session << "SELECT C_VALUE from TPARAMETERS where C_PARAM='DISCIPLINE'", into(m_discipline), now;

		// open meeting file
		Session sessionMain("SQLite", m_mainFile);
		int date1, date2, dateEvent, timeEvent;

		sessionMain << "SELECT C_VALUE from TPARAMETERS where C_PARAM='TITLE1'", into(m_szTitle), now;
		sessionMain << "SELECT C_VALUE from TPARAMETERS where C_PARAM='DATE1'", into(date1), now;
		sessionMain << "SELECT C_VALUE from TPARAMETERS where C_PARAM='DATE2'", into(date2), now;
		char szSql[MAX_PATH];
		sprintf(szSql, "SELECT C_VALUE from TPARAMETERS where C_PARAM='EVENT%02d_STARTHOUR'", m_eventNo);
		sessionMain << szSql, into(m_timeEvent), now;

		

		string param1 = ns_base::string_format("EVENT%02d_TITLE", m_eventNo), param2 = ns_base::string_format("EVENT%02d_DATE", m_eventNo);

		sessionMain << "SELECT C_VALUE from TPARAMETERS where C_PARAM=?", use(param1), into(m_szEvent), now;
		sessionMain << "SELECT C_VALUE from TPARAMETERS where C_PARAM=?", use(param2), into(dateEvent), now;

		string table1 = ns_base::string_format("TPARAMETERS_EVENT%02d", m_eventNo);

		sessionMain << "SELECT C_VALUE from " + table1 + " where C_PARAM='ROUND_NUMBER'", into(m_roundNumber), now;
		sessionMain << "SELECT C_VALUE from " + table1 + " where C_PARAM='JUMPOFF_NUMBER'", into(m_jumpoffNumber), now;
		sessionMain << "SELECT C_VALUE from " + table1 + " where C_PARAM='TWO_PHASES_INTEGRED'", into(m_twoPhaseIntegrated), now;
		sessionMain << "SELECT C_VALUE from " + table1 + " where C_PARAM='TWO_PHASES_DIFFERED'", into(m_twoPhaseDiffered), now;

		sessionMain << "SELECT C_VALUE from " + table1 + " where C_PARAM='CROSS_LIMIT_TIME'", into(m_crossLimitTime), now;
		sessionMain << "SELECT C_VALUE from " + table1 + " where C_PARAM='CROSS_OPTIMUM_TIME'", into(m_crossOptimumTime), now;
		sessionMain << "SELECT C_VALUE from " + table1 + " where C_PARAM='CROSS_MINIMUM_TIME'", into(m_crossMinimumTime), now;

		sessionMain << "SELECT C_VALUE from " + table1 + " where C_PARAM='CROSS_SURPASSING_BASE_TIME'", into(m_crossSurpassingBaseTime), now;
		sessionMain << "SELECT C_VALUE from " + table1 + " where C_PARAM='CROSS_SURPASSING_POINTS'", into(m_crossSurpassingPoints), now;
		sessionMain << "SELECT C_VALUE from " + table1 + " where C_PARAM='CROSS_SURPASSING_MIN_BASE_TIME'", into(m_crossSurpassingMinBaseTime), now;
		sessionMain << "SELECT C_VALUE from " + table1 + " where C_PARAM='CROSS_SURPASSING_MIN_POINTS'", into(m_crossSurpassingMinPoints), now;

		sessionMain << "SELECT C_VALUE from " + table1 + " where C_PARAM='MODE_TEAM'", into(m_modeTeam), now;
		sessionMain << "SELECT C_VALUE from " + table1 + " where C_PARAM='MODE_TEAM_RELAY'", into(m_modeTeamRelay), now;

		sessionMain << "SELECT C_VALUE from " + table1 + " where C_PARAM='FEI_ScheduleCompetitionNR'", into(m_schedulerNumber), now;
		sessionMain << "SELECT C_VALUE from " + table1 + " where C_PARAM='FNCH_CATEGORY'", into(m_category), now;
		sessionMain << "SELECT C_VALUE from " + table1 + " where C_PARAM='COMMENTS'", into(m_notes), now;
		sessionMain << "SELECT C_VALUE from " + table1 + " where C_PARAM='OBSTACLE_HEIGHT'", into(m_height), now;
		sessionMain << "SELECT C_VALUE from " + table1 + " where C_PARAM='EVENT_CLT_INITIAL_PRIZE'", into(m_initAwardListAmount), now;

		sessionMain << "SELECT C_VALUE from " + table1 + " where C_PARAM='DISCIPLINE'", into(m_eventing), now;

		m_eventing = m_eventing == 4 ? 1 : 0;

		
		if (m_eventing) {
			string str = "[SPRINGEN]";
			if (m_discipline == 2) str = "[CROSS]";
			else if (m_discipline == 3) str = "[DRESSUR]";

			m_szEvent = m_szEvent + " " + str;
		}
		

		if (m_eventing && m_manualDiscipline != 0) {
			switch (m_manualDiscipline)
			{
			case 1:
				m_discipline = 0;
				break;
			case 2:
				m_discipline = 2;
				break;
			case 3:
				m_discipline = 3;
				break;
			}
		}

		m_dateStart = Poco::DateTime(1900, 1, 1) + Poco::Timespan(date1 - 2, 0, 0, 0, 0);
		m_dateEnd = Poco::DateTime(1900, 1, 1) + Poco::Timespan(date2 - 2, 0, 0, 0, 0);
		m_dateEvent = Poco::DateTime(1900, 1, 1) + Poco::Timespan(dateEvent - 2, 0, 0, 0, 0);

		// update by luda
		string discipline1 = "";
		if (m_discipline == 2) discipline1 = "CROSS_";
		else if (m_discipline == 3) discipline1 == "DRESSAGE_";

		if (m_discipline == 0) {
			/*
			// load start time
			typedef Tuple<int, int, string, string> ChronoTuple;
			std::vector<ChronoTuple> chronos;
			string chronoTable = ns_base::string_format("TCHRONOMESSAGE_%sEVENT%02d_ROUND01", discipline1.c_str(), m_eventNo);
			sessionMain << "SELECT C_LINE, C_NUM, C_MSG1, C_MSG2 from " + chronoTable, into(chronos), now;
			int iFirstRider = 0;
			for (int i = 0; i < chronos.size(); i++) {
				string msg = chronos[i].get<2>();
				if (strncmp(msg.c_str(), "-", 1) == 0) {
					iFirstRider = i;
					break;
				}
			}
			if (iFirstRider != 0) {
				ChronoTuple chrono = chronos[iFirstRider + 2];
				m_gameBeginTime = chrono.get<2>();
			}
			else {
				m_gameBeginTime = "00:00:00";
			}*/
			m_gameBeginTime = "00:00:00";
		}

		// loading riders
		m_riders.clear();
		typedef Tuple<int, string, string, int, string, string, string, string> RiderTuple;
		std::vector<RiderTuple> riders;
		sessionMain << "SELECT C_IDX, C_LAST_NAME, C_FIRST_NAME, C_BIRTH_DATE, C_NATION, C_CLUB, C_CODE, C_REGION_NAME from TRIDERS", into(riders), now;
		for each (RiderTuple record in riders)
		{
			Rider	rider;
			rider.m_nIdx = record.get<0>();
			rider.m_szFirstName = record.get<2>();
			rider.m_szLastName = record.get<1>();
			rider.m_szNation = record.get<4>();
			rider.m_dateBirthday = Poco::DateTime(1900, 1, 1) + Poco::Timespan(record.get<3>(), 0, 0, 0, 0);
			rider.m_szClub = record.get<5>();
			rider.m_szLicense = record.get<6>();
			rider.m_szCity = record.get<7>();

			m_riders[rider.m_nIdx] = rider;
		}

		// loading horses
		m_horses.clear();
		typedef Tuple<int, string, int, int, string, int, string, string, string, string, string> HorseTuple;
		std::vector<HorseTuple>  horses;
		sessionMain << "SELECT C_IDX, C_NAME, C_AGE, C_BIRTH_DATE, C_OWNER, C_SEX, C_FATHER, C_MOTHER, C_FATHER_OF_MOTHER, C_CODE, C_RACE_CODE from THORSES", into(horses), now;
		for each (HorseTuple record in horses)
		{
			Horse	horse;
			horse.m_nIdx = record.get<0>();
			horse.m_szName = record.get<1>();
			horse.m_nAge = record.get<2>();
			horse.m_dateBirthday = Poco::DateTime(1900, 1, 1) + Poco::Timespan(record.get<3>(), 0, 0, 0, 0);
			horse.m_szOwner = record.get<4>();
			horse.m_szGender = record.get<5>() == 0 ? "Male" : "Female";
			horse.m_szFather = record.get<6>();
			horse.m_szMother = record.get<7>();
			horse.m_szFatherOfMother = record.get<8>();
			horse.m_szFnchPassport = record.get<9>();
			horse.m_szSignalementLabel = record.get<10>();
			m_horses[horse.m_nIdx] = horse;
		}

		m_numberIndice.clear();
		string table3 = ns_base::string_format("TCOMMITTED_EVENT%02d", m_eventNo);

		typedef Tuple<int, int, int> NumberIndiceTuple;
		std::vector<NumberIndiceTuple>  numberIndice;
		sessionMain << "SELECT C_NUM, C_HORSE_IDX, C_RIDER_IDX from " + table3, into(numberIndice), now;
		for each (NumberIndiceTuple record in numberIndice)
		{
			NumberToIndexEntry entry;
			entry.m_nNumber = record.get<0>();
			entry.m_nHorseIdx = record.get<1>();
			entry.m_nRiderIdx = record.get<2>();
			m_numberIndice[entry.m_nNumber] = entry;
		}

		m_startlist.clear();
		// update by luda
		string discipline = "ROUND";
		if (m_discipline == 2) discipline = "CROSS";
		else if (m_discipline == 3) discipline = "DRESSAGE";
		
		string table2 = ns_base::string_format("TSTARTLIST_EVENT%02d_%s01", m_eventNo, discipline.c_str());


		typedef Tuple<int, int, int> StartlistTuple;
		std::vector<StartlistTuple>  startlist;
		sessionMain << "SELECT C_LINE, C_NUM, C_START from " + table2 + " order by C_LINE", into(startlist), now;
		for each (StartlistTuple record in startlist)
		{
			StartListEntry entry;
			entry.m_nPos = record.get<0>();
			entry.m_nNumber = record.get<1>();
			entry.m_nStartTime = record.get<2>();
			map<int, NumberToIndexEntry>::iterator it = m_numberIndice.find(entry.m_nNumber);
			if (it != m_numberIndice.end()) {
				entry.m_nHorseIdx = it->second.m_nHorseIdx;
				entry.m_nRiderIdx = it->second.m_nRiderIdx;
			}
			else {
				entry.m_nHorseIdx = -1;
				entry.m_nRiderIdx = -1;
			}

			m_startlist.push_back(entry);
		}

		
		m_teams.clear();

		if (m_modeTeam || m_modeTeamRelay) {

			string table = ns_base::string_format("TTEAM_EVENT%02d", m_eventNo);
			typedef Tuple<string, int, int, int, int, int, int> TeamTuple;
			std::vector<TeamTuple>  teams;
			sessionMain << "SELECT C_NAME, C_COMMITTED1, C_COMMITTED2, C_COMMITTED3, C_COMMITTED4, C_COMMITTED5, C_NUM from " + table + " order by C_NUM", into(teams), now;
			for each (TeamTuple record in teams)
			{
				Team team;

				string teamName = record.get<0>();

				team.m_szName = teamName;

				team.m_nMembers[0] = record.get<1>();
				team.m_nMembers[1] = record.get<2>();
				team.m_nMembers[2] = record.get<3>();
				team.m_nMembers[3] = record.get<4>();
				team.m_nMembers[4] = record.get<5>();

				team.m_nNum = record.get<6>();

				m_teams[teamName] = team;
			}
		}
		
		LoadJudges();

		m_prevCoursePoints = 0;
	}

	catch (Exception& e) {
		m_bLoaded = false;
		return false;
	}

	m_szDbFolder = folder;
	m_bLoaded = true;

	m_publishUploadFiles = GetPublishFilesUpload();

	return true;
}

bool EquestreDb::LoadCoursePoints()
{
	if (!m_bLoaded) return false;

	// open meeting file
	//Session sessionMain("SQLite", m_mainFile);

	string db = ns_base::string_format("Event%03d.scdb", m_eventNo);
	string subFile = std::regex_replace(m_mainFile, std::regex("\.scdb"), db);
	Session sessionSub("SQLite", subFile);

	string table1 = ns_base::string_format("TOBSTACLEINFOS_EVENT%02d_CROSS01", m_eventNo);
	m_coursePoints.clear();
	typedef Tuple<int, int> CoursePointTuple;
	std::vector<CoursePointTuple> points;
	sessionSub << "SELECT C_NUM, (C_I01 + C_I04 + C_I07 + C_I10 + C_I13 + C_I17 + C_I20 + C_I23 + C_I26 + C_I29 + C_I32 + C_I35 + C_I38 + C_I41 + C_I44 + C_I47) AS C_COURSE_POINTS FROM " + table1, into(points), now;

	static int isFirst = true;

	int sumPoints = 0;
	for each (CoursePointTuple record in points)
	{
		int num = record.get<0>();
		int coursePoint = record.get<1>();

		sumPoints += coursePoint;

		m_coursePoints[num] = coursePoint;
	}

	/*
	if (isFirst) {
		isFirst = false;
		
		m_prevCoursePoints = sumPoints;

		return true;
	}

	if (m_prevCoursePoints != sumPoints)
	*/
		return true;
	
	//return false;
}

void EquestreDb::UpdateTeamRanking()
{
	if (m_bLoaded) {
		Session sessionMain("SQLite", m_mainFile);
		string db = ns_base::string_format("Event%03d.scdb", m_eventNo);
		string subFile = std::regex_replace(m_mainFile, std::regex("\.scdb"), db);
		Session sessionSub("SQLite", subFile);

		if (m_discipline == 0) { // for jumping

			// loading ranking
			for (int i = 0; i < 4; i++) {
				m_roundRanks[i].clear();
				m_jumpoffRanks[i].clear();
			}

			string table1 = ns_base::string_format("TPARAMETERS_EVENT%02d", m_eventNo);
			sessionMain << "SELECT C_VALUE from " + table1 + " where C_PARAM='ROUND_NUMBER'", into(m_roundNumber), now;
			sessionMain << "SELECT C_VALUE from " + table1 + " where C_PARAM='JUMPOFF_NUMBER'", into(m_jumpoffNumber), now;

			sessionMain << "SELECT C_VALUE from " + table1 + " where C_PARAM='TABLE_ROUND1'", into(m_roundTableTypes[0]), now;
			sessionMain << "SELECT C_VALUE from " + table1 + " where C_PARAM='TABLE_ROUND2'", into(m_roundTableTypes[1]), now;
			sessionMain << "SELECT C_VALUE from " + table1 + " where C_PARAM='TABLE_ROUND3'", into(m_roundTableTypes[2]), now;
			sessionMain << "SELECT C_VALUE from " + table1 + " where C_PARAM='TABLE_ROUND4'", into(m_roundTableTypes[3]), now;

			sessionMain << "SELECT C_VALUE from " + table1 + " where C_PARAM='TABLE_JUMPOFF1'", into(m_jumpoffTableTypes[0]), now;
			sessionMain << "SELECT C_VALUE from " + table1 + " where C_PARAM='TABLE_JUMPOFF2'", into(m_jumpoffTableTypes[1]), now;
			sessionMain << "SELECT C_VALUE from " + table1 + " where C_PARAM='TABLE_JUMPOFF3'", into(m_jumpoffTableTypes[2]), now;
			sessionMain << "SELECT C_VALUE from " + table1 + " where C_PARAM='TABLE_JUMPOFF4'", into(m_jumpoffTableTypes[3]), now;

			sessionMain << "SELECT C_VALUE from " + table1 + " where C_PARAM='ALLOWED_TIME_ROUND1'", into(m_allowedTimeRounds[0]), now;
			sessionMain << "SELECT C_VALUE from " + table1 + " where C_PARAM='ALLOWED_TIME_ROUND2'", into(m_allowedTimeRounds[1]), now;
			sessionMain << "SELECT C_VALUE from " + table1 + " where C_PARAM='ALLOWED_TIME_ROUND3'", into(m_allowedTimeRounds[2]), now;
			sessionMain << "SELECT C_VALUE from " + table1 + " where C_PARAM='ALLOWED_TIME_ROUND4'", into(m_allowedTimeRounds[3]), now;

			sessionMain << "SELECT C_VALUE from " + table1 + " where C_PARAM='ALLOWED_TIME_JUMPOFF1'", into(m_allowedTimeJumpoffs[0]), now;
			sessionMain << "SELECT C_VALUE from " + table1 + " where C_PARAM='ALLOWED_TIME_JUMPOFF2'", into(m_allowedTimeJumpoffs[1]), now;
			sessionMain << "SELECT C_VALUE from " + table1 + " where C_PARAM='ALLOWED_TIME_JUMPOFF3'", into(m_allowedTimeJumpoffs[2]), now;
			sessionMain << "SELECT C_VALUE from " + table1 + " where C_PARAM='ALLOWED_TIME_JUMPOFF4'", into(m_allowedTimeJumpoffs[3]), now;

			sessionMain << "SELECT C_VALUE from " + table1 + " where C_PARAM='AGAINST_CLOCK_ROUND1'", into(m_againstTimeClockRounds[0]), now;
			sessionMain << "SELECT C_VALUE from " + table1 + " where C_PARAM='AGAINST_CLOCK_ROUND2'", into(m_againstTimeClockRounds[1]), now;
			sessionMain << "SELECT C_VALUE from " + table1 + " where C_PARAM='AGAINST_CLOCK_ROUND3'", into(m_againstTimeClockRounds[2]), now;
			sessionMain << "SELECT C_VALUE from " + table1 + " where C_PARAM='AGAINST_CLOCK_ROUND4'", into(m_againstTimeClockRounds[3]), now;

			sessionMain << "SELECT C_VALUE from " + table1 + " where C_PARAM='AGAINST_CLOCK_JUMPOFF1'", into(m_againstTimeClockJumpoffs[0]), now;
			sessionMain << "SELECT C_VALUE from " + table1 + " where C_PARAM='AGAINST_CLOCK_JUMPOFF2'", into(m_againstTimeClockJumpoffs[1]), now;
			sessionMain << "SELECT C_VALUE from " + table1 + " where C_PARAM='AGAINST_CLOCK_JUMPOFF3'", into(m_againstTimeClockJumpoffs[2]), now;
			sessionMain << "SELECT C_VALUE from " + table1 + " where C_PARAM='AGAINST_CLOCK_JUMPOFF4'", into(m_againstTimeClockJumpoffs[3]), now;

			try {
				for (int iRound = 1; iRound <= m_roundNumber; iRound++) {
					string table = ns_base::string_format("TTIMEINFOS_EVENT%02d_ROUND%02d", m_eventNo, iRound);
					typedef Tuple<int, int, int, int, int, int, int, int, int> RankTuple;
					std::vector<RankTuple>  ranks;
					sessionSub << "SELECT C_NUM, C_STATUS, C_TIME, C_TIME_PENALTY, (C_TIME + C_TIME_PENALTY + C_TIME_FAULTS) as TIMES, C_POINTS_SURPASSING, C_POINTS_FAULTS, (C_POINTS_SURPASSING + C_POINTS_FAULTS) as POINTS, C_TIME_FAULTS from " + table + " order by C_STATUS, POINTS, TIMES", into(ranks), now;
					for each (RankTuple record in ranks)
					{
						int nNum = record.get<0>();
						Ranking& ranking = m_roundRanks[iRound - 1][nNum];
						ranking.m_nNum = nNum;
						ranking.m_nStatus = record.get<1>();
						ranking.m_nTime = record.get<2>();
						ranking.m_nTimePenalty = record.get<3>();
						ranking.m_nTimes = record.get<4>();
						ranking.m_nPointsSurpassing = record.get<5>();
						ranking.m_nPointsFaults = record.get<6>();
						ranking.m_nPoints = record.get<7>() + record.get<8>();
					}
				}
				for (int iRound = 1; iRound <= m_jumpoffNumber; iRound++) {
					string table = ns_base::string_format("TTIMEINFOS_EVENT%02d_JUMPOFF%02d", m_eventNo, iRound);
					typedef Tuple<int, int, int, int, int, int, int, int, int> RankTuple;
					std::vector<RankTuple>  ranks;
					sessionSub << "SELECT C_NUM, C_STATUS, C_TIME, C_TIME_PENALTY, (C_TIME + C_TIME_PENALTY) as TIMES, C_POINTS_SURPASSING, C_POINTS_FAULTS, (C_POINTS_SURPASSING + C_POINTS_FAULTS) as POINTS, C_TIME_FAULTS from " + table + " order by C_STATUS, POINTS, TIMES", into(ranks), now;
					for each (RankTuple record in ranks)
					{
						int nNum = record.get<0>();
						Ranking& ranking = m_jumpoffRanks[iRound - 1][nNum];
						ranking.m_nNum = nNum;
						ranking.m_nStatus = record.get<1>();
						ranking.m_nTime = record.get<2>();
						ranking.m_nTimePenalty = record.get<3>();
						ranking.m_nTimes = record.get<4>();

						ranking.m_nPointsSurpassing = record.get<5>();
						ranking.m_nPointsFaults = record.get<6>();
						ranking.m_nPoints = record.get<7>() + record.get<8>();
					}
				}
			}
			catch (Exception&) {

			}
		}
		else if (m_discipline == 2) { // for cross

			// loading ranking
			for (int i = 0; i < 4; i++) {
				m_roundRanks[i].clear();
				m_jumpoffRanks[i].clear();
			}

			string table1 = ns_base::string_format("TPARAMETERS_EVENT%02d", m_eventNo);
			sessionMain << "SELECT C_VALUE from " + table1 + " where C_PARAM='ROUND_NUMBER'", into(m_roundNumber), now;
			sessionMain << "SELECT C_VALUE from " + table1 + " where C_PARAM='JUMPOFF_NUMBER'", into(m_jumpoffNumber), now;

			try {

				int iRound = 1;

				string table = ns_base::string_format("TTIMEINFOS_EVENT%02d_CROSS%02d_J", m_eventNo, iRound);
				typedef Tuple<int, int, int, int, int, int, int, int, int> RankTuple;
				std::vector<RankTuple>  ranks;
				sessionSub << "SELECT C_NUM, C_STATUS, C_TIME, C_TIME_PENALTY, (C_TIME + C_TIME_PENALTY + C_TIME_FAULTS) as TIMES, C_POINTS_SURPASSING, C_POINTS_FAULTS, (C_POINTS_SURPASSING + C_POINTS_FAULTS) as POINTS, C_TIME_FAULTS from " + table + " order by C_STATUS, POINTS, TIMES", into(ranks), now;
				for each (RankTuple record in ranks)
				{
					int nNum = record.get<0>();
					Ranking& ranking = m_roundRanks[iRound - 1][nNum];
					ranking.m_nNum = nNum;
					ranking.m_nStatus = record.get<1>();
					ranking.m_nTime = record.get<2>();
					ranking.m_nTimePenalty = record.get<3>();
					ranking.m_nTimes = record.get<4>();
					ranking.m_nPointsSurpassing = record.get<5>();
					ranking.m_nPointsFaults = record.get<6>();
					ranking.m_nPoints = record.get<7>() + record.get<8>();
				}
			}
			catch (Exception&) {

			}

		}

	}
}

void EquestreDb::UpdateRanking()
{
	if (m_bLoaded) {
		Session sessionMain("SQLite", m_mainFile);

		string db = ns_base::string_format("Event%03d.scdb", m_eventNo);
		string subFile = std::regex_replace(m_mainFile, std::regex("\.scdb"), db);
		Session sessionSub("SQLite", subFile);

		m_dressageJudgeScoreTotal = 0;



		if (m_discipline == 0) { // for jumping

			// loading ranking
			for (int i = 0; i < 4; i++) {
				m_roundRanks[i].clear();
				m_jumpoffRanks[i].clear();
			}

			string table1 = ns_base::string_format("TPARAMETERS_EVENT%02d", m_eventNo);
			sessionMain << "SELECT C_VALUE from " + table1 + " where C_PARAM='ROUND_NUMBER'", into(m_roundNumber), now;
			sessionMain << "SELECT C_VALUE from " + table1 + " where C_PARAM='JUMPOFF_NUMBER'", into(m_jumpoffNumber), now;

			sessionMain << "SELECT C_VALUE from " + table1 + " where C_PARAM='TABLE_ROUND1'", into(m_roundTableTypes[0]), now;
			sessionMain << "SELECT C_VALUE from " + table1 + " where C_PARAM='TABLE_ROUND2'", into(m_roundTableTypes[1]), now;
			sessionMain << "SELECT C_VALUE from " + table1 + " where C_PARAM='TABLE_ROUND3'", into(m_roundTableTypes[2]), now;
			sessionMain << "SELECT C_VALUE from " + table1 + " where C_PARAM='TABLE_ROUND4'", into(m_roundTableTypes[3]), now;

			sessionMain << "SELECT C_VALUE from " + table1 + " where C_PARAM='TABLE_JUMPOFF1'", into(m_jumpoffTableTypes[0]), now;
			sessionMain << "SELECT C_VALUE from " + table1 + " where C_PARAM='TABLE_JUMPOFF2'", into(m_jumpoffTableTypes[1]), now;
			sessionMain << "SELECT C_VALUE from " + table1 + " where C_PARAM='TABLE_JUMPOFF3'", into(m_jumpoffTableTypes[2]), now;
			sessionMain << "SELECT C_VALUE from " + table1 + " where C_PARAM='TABLE_JUMPOFF4'", into(m_jumpoffTableTypes[3]), now;

			sessionMain << "SELECT C_VALUE from " + table1 + " where C_PARAM='ALLOWED_TIME_ROUND1'", into(m_allowedTimeRounds[0]), now;
			sessionMain << "SELECT C_VALUE from " + table1 + " where C_PARAM='ALLOWED_TIME_ROUND2'", into(m_allowedTimeRounds[1]), now;
			sessionMain << "SELECT C_VALUE from " + table1 + " where C_PARAM='ALLOWED_TIME_ROUND3'", into(m_allowedTimeRounds[2]), now;
			sessionMain << "SELECT C_VALUE from " + table1 + " where C_PARAM='ALLOWED_TIME_ROUND4'", into(m_allowedTimeRounds[3]), now;

			sessionMain << "SELECT C_VALUE from " + table1 + " where C_PARAM='ALLOWED_TIME_JUMPOFF1'", into(m_allowedTimeJumpoffs[0]), now;
			sessionMain << "SELECT C_VALUE from " + table1 + " where C_PARAM='ALLOWED_TIME_JUMPOFF2'", into(m_allowedTimeJumpoffs[1]), now;
			sessionMain << "SELECT C_VALUE from " + table1 + " where C_PARAM='ALLOWED_TIME_JUMPOFF3'", into(m_allowedTimeJumpoffs[2]), now;
			sessionMain << "SELECT C_VALUE from " + table1 + " where C_PARAM='ALLOWED_TIME_JUMPOFF4'", into(m_allowedTimeJumpoffs[3]), now;

			sessionMain << "SELECT C_VALUE from " + table1 + " where C_PARAM='AGAINST_CLOCK_ROUND1'", into(m_againstTimeClockRounds[0]), now;
			sessionMain << "SELECT C_VALUE from " + table1 + " where C_PARAM='AGAINST_CLOCK_ROUND2'", into(m_againstTimeClockRounds[1]), now;
			sessionMain << "SELECT C_VALUE from " + table1 + " where C_PARAM='AGAINST_CLOCK_ROUND3'", into(m_againstTimeClockRounds[2]), now;
			sessionMain << "SELECT C_VALUE from " + table1 + " where C_PARAM='AGAINST_CLOCK_ROUND4'", into(m_againstTimeClockRounds[3]), now;

			sessionMain << "SELECT C_VALUE from " + table1 + " where C_PARAM='AGAINST_CLOCK_JUMPOFF1'", into(m_againstTimeClockJumpoffs[0]), now;
			sessionMain << "SELECT C_VALUE from " + table1 + " where C_PARAM='AGAINST_CLOCK_JUMPOFF2'", into(m_againstTimeClockJumpoffs[1]), now;
			sessionMain << "SELECT C_VALUE from " + table1 + " where C_PARAM='AGAINST_CLOCK_JUMPOFF3'", into(m_againstTimeClockJumpoffs[2]), now;
			sessionMain << "SELECT C_VALUE from " + table1 + " where C_PARAM='AGAINST_CLOCK_JUMPOFF4'", into(m_againstTimeClockJumpoffs[3]), now;

			try {

				for (int iRound = 1; iRound <= m_roundNumber; iRound++) {
					string table = ns_base::string_format("TTIMEINFOS_EVENT%02d_ROUND%02d", m_eventNo, iRound);
					typedef Tuple<int, int, int, int, int, int, int, int, int> RankTuple;
					std::vector<RankTuple>  ranks;
					sessionSub << "SELECT C_NUM, C_STATUS, C_TIME, C_TIME_PENALTY, (C_TIME + C_TIME_PENALTY + C_TIME_FAULTS) as TIMES, C_POINTS_SURPASSING, C_POINTS_FAULTS, (C_POINTS_SURPASSING + C_POINTS_FAULTS) as POINTS, C_TIME_FAULTS from " + table + " order by C_STATUS, POINTS, TIMES", into(ranks), now;
					for each (RankTuple record in ranks)
					{
						int nNum = record.get<0>();
						Ranking& ranking = m_roundRanks[iRound - 1][nNum];
						ranking.m_nNum = nNum;
						ranking.m_nStatus = record.get<1>();
						ranking.m_nTime = record.get<2>();
						ranking.m_nTimePenalty = record.get<3>();
						ranking.m_nTimes = record.get<4>();
						ranking.m_nPointsSurpassing = record.get<5>();
						ranking.m_nPointsFaults = record.get<6>();
						ranking.m_nPoints = record.get<7>() + record.get<8>();

					}
				}
				for (int iRound = 1; iRound <= m_jumpoffNumber; iRound++) {
					string table = ns_base::string_format("TTIMEINFOS_EVENT%02d_JUMPOFF%02d", m_eventNo, iRound);
					typedef Tuple<int, int, int, int, int, int, int, int, int> RankTuple;
					std::vector<RankTuple>  ranks;
					sessionSub << "SELECT C_NUM, C_STATUS, C_TIME, C_TIME_PENALTY, (C_TIME + C_TIME_PENALTY) as TIMES, C_POINTS_SURPASSING, C_POINTS_FAULTS, (C_POINTS_SURPASSING + C_POINTS_FAULTS) as POINTS, C_TIME_FAULTS from " + table + " order by C_STATUS, POINTS, TIMES", into(ranks), now;
					for each (RankTuple record in ranks)
					{
						int nNum = record.get<0>();
						Ranking& ranking = m_jumpoffRanks[iRound - 1][nNum];
						ranking.m_nNum = nNum;
						ranking.m_nStatus = record.get<1>();
						ranking.m_nTime = record.get<2>();
						ranking.m_nTimePenalty = record.get<3>();
						ranking.m_nTimes = record.get<4>();

						ranking.m_nPointsSurpassing = record.get<5>();
						ranking.m_nPointsFaults = record.get<6>();
						ranking.m_nPoints = record.get<7>() + record.get<8>();
					}
				}
			}
			catch (Exception&) {

			}
		}
		else if (m_discipline == 2) { // for cross
			
			// loading ranking
			for (int i = 0; i < 4; i++) {
				m_roundRanks[i].clear();
				m_jumpoffRanks[i].clear();
			}

			string table1 = ns_base::string_format("TPARAMETERS_EVENT%02d", m_eventNo);
			sessionMain << "SELECT C_VALUE from " + table1 + " where C_PARAM='ROUND_NUMBER'", into(m_roundNumber), now;
			sessionMain << "SELECT C_VALUE from " + table1 + " where C_PARAM='JUMPOFF_NUMBER'", into(m_jumpoffNumber), now;

			try {

				int iRound = 1;

				string table = ns_base::string_format("TTIMEINFOS_EVENT%02d_CROSS%02d_J", m_eventNo, iRound);
				typedef Tuple<int, int, int, int, int, int, int, int, int> RankTuple;
				std::vector<RankTuple>  ranks;
				sessionSub << "SELECT C_NUM, C_STATUS, C_TIME, C_TIME_PENALTY, (C_TIME + C_TIME_PENALTY + C_TIME_FAULTS) as TIMES, C_POINTS_SURPASSING, C_POINTS_FAULTS, (C_POINTS_SURPASSING + C_POINTS_FAULTS) as POINTS, C_TIME_FAULTS from " + table + " order by C_STATUS, POINTS, TIMES", into(ranks), now;
				for each (RankTuple record in ranks)
				{
					int nNum = record.get<0>();
					Ranking& ranking = m_roundRanks[iRound - 1][nNum];
					ranking.m_nNum = nNum;
					ranking.m_nStatus = record.get<1>();
					ranking.m_nTime = record.get<2>();
					ranking.m_nTimePenalty = record.get<3>();
					ranking.m_nTimes = record.get<4>();
					ranking.m_nPointsSurpassing = record.get<5>();
					ranking.m_nPointsFaults = record.get<6>();
					ranking.m_nPoints = record.get<7>() + record.get<8>();
				}
			}
			catch (Exception&) {

			}
		}
		else if (m_discipline == 3) { // for dressage
			// loading ranking
			for (int i = 0; i < 4; i++) {
				m_roundRanks[i].clear();
				m_jumpoffRanks[i].clear();
			}

			string table1 = ns_base::string_format("TPARAMETERS_EVENT%02d", m_eventNo);
			sessionMain << "SELECT C_VALUE from " + table1 + " where C_PARAM='ROUND_NUMBER'", into(m_roundNumber), now;
			sessionMain << "SELECT C_VALUE from " + table1 + " where C_PARAM='JUMPOFF_NUMBER'", into(m_jumpoffNumber), now;

			sessionMain << "SELECT SUM(C_VALUE) from " + table1 + " where C_PARAM like 'SCORE_COEF_%'", into(m_dressageJudgeScoreTotal), now;
			m_dressageJudgeScoreTotal = m_dressageJudgeScoreTotal == 0? 1 : m_dressageJudgeScoreTotal * 10;

			int iRound = 1;

			string table = ns_base::string_format("TTIMEINFOS_EVENT%02d_DRESSAGE%02d_J", m_eventNo, iRound);
			typedef Tuple<int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int> RankTuple;
			std::vector<RankTuple>  ranks;

			try {

				sessionSub << "SELECT C_NUM, C_STATUS, C_TIME, C_TIME_PENALTY, (C_TIME + C_TIME_PENALTY + C_TIME_FAULTS) as TIMES, C_POINTS_SURPASSING, C_POINTS_FAULTS, (C_POINTS_SURPASSING + C_POINTS_FAULTS) as POINTS, C_TIME_FAULTS, C_FIGURE_MARK, C_JUDGE1, C_JUDGE2, C_JUDGE3, C_JUDGE4, C_JUDGE5, C_JUDGE6, C_JUDGE7 from " + table + " order by C_STATUS, POINTS, TIMES", into(ranks), now;

				for each (RankTuple record in ranks)
				{
					int nNum = record.get<0>();
					Ranking& ranking = m_roundRanks[iRound - 1][nNum];
					ranking.m_nNum = nNum;
					ranking.m_nStatus = record.get<1>();
					ranking.m_nTime = record.get<2>();
					ranking.m_nTimePenalty = record.get<3>();
					//ranking.m_nTimes = record.get<4>();
					//ranking.m_nTimes = record.get<7>() + record.get<8>();
					ranking.m_nTimes = record.get<6>();
					//ranking.m_nTimes = record.get<9>();

					ranking.m_nPointsSurpassing = record.get<5>();
					ranking.m_nPointsFaults = record.get<6>();
					//ranking.m_nPoints = record.get<7>() + record.get<8>();
					ranking.m_nPoints = record.get<9>();
					//ranking.m_nPoints = record.get<6>();

					int nJudgeScores[5];
					nJudgeScores[0] = record.get<10>();
					nJudgeScores[1] = record.get<11>();
					nJudgeScores[2] = record.get<12>();
					nJudgeScores[3] = record.get<13>();
					nJudgeScores[4] = record.get<14>();

					char szScore[255];
					/*
					float fFinalScore = 0;
					for (int k = 0; k < 5; k ++) {
						fFinalScore += (float)nJudgeScores[k];
					}
					fFinalScore /= 5;
					sprintf(szScore, "%.02f", fFinalScore);
					ranking.m_szJudgeFinalScore = szScore;
					*/

					for (int k = 0; k < 5; k ++) {
						
						if (nJudgeScores[k]) {
							sprintf(szScore, "%.02f", (int)(10000 * nJudgeScores[k] / m_dressageJudgeScoreTotal) / 100.0f);
							ranking.m_szJudgeScores += m_judges[k + 1].m_szPosition + ": " + szScore + "% ";
						}
					}

					/*
					ranking.m_szJudgeScores =
						m_judges[1].m_szPosition + ": " + std::to_string(100 * record.get<10>() / m_dressageJudgeScoreTotal) + "%, " +
						m_judges[2].m_szPosition + ": " + std::to_string(100 * record.get<11>() / m_dressageJudgeScoreTotal) + "%, " +
						m_judges[3].m_szPosition + ": " + std::to_string(100 * record.get<12>() / m_dressageJudgeScoreTotal) + "%, " +
						m_judges[4].m_szPosition + ": " + std::to_string(100 * record.get<13>() / m_dressageJudgeScoreTotal) + "%, " +
						m_judges[5].m_szPosition + ": " + std::to_string(100 * record.get<14>() / m_dressageJudgeScoreTotal) + "% ";// +
						//m_judges[6].m_szPosition + ": " + std::to_string(record.get<15>()) + ", " +
						//m_judges[7].m_szPosition + ": " + std::to_string(record.get<16>());
						*/
				}
			}
			catch (Exception&) {

			}
		}

	}
}

void EquestreDb::UpdateCCRanking()
{
	if (m_bLoaded) {

		Session sessionMain("SQLite", m_mainFile);
		string db = ns_base::string_format("Event%03d.scdb", m_eventNo);
		string subFile = std::regex_replace(m_mainFile, std::regex("\.scdb"), db);
		Session sessionSub("SQLite", subFile);

		m_ccRanks.clear();

		try {
			string table = ns_base::string_format("TTIMEINFOS_EVENT%02d_ROUND01", m_eventNo);
			typedef Tuple<int, int, int, int, int, int, int, int, int> RankTuple;
			std::vector<RankTuple>  ranks;
			sessionSub << "SELECT C_NUM, C_STATUS, C_TIME, C_TIME_PENALTY, (C_TIME + C_TIME_PENALTY + C_TIME_FAULTS) as TIMES, C_POINTS_SURPASSING, C_POINTS_FAULTS, (C_POINTS_SURPASSING + C_POINTS_FAULTS) as POINTS, C_TIME_FAULTS from " + table + " order by C_STATUS, POINTS, TIMES", into(ranks), now;
			
			for each (RankTuple record in ranks)
			{
				int nNum = record.get<0>();
				CCRanking& ranking = m_ccRanks[nNum];
				ranking.m_nNum = nNum;
				ranking.m_nJumpStatus = record.get<1>();
				ranking.m_nJumpPoints = record.get<7>();
				ranking.m_nJumpTimes = record.get<2>();
				//ranking.m_nTime = record.get<2>();
				//ranking.m_nTimePenalty = record.get<3>();
				//ranking.m_nTimes = record.get<4>();
				//ranking.m_nPointsSurpassing = record.get<5>();
				//ranking.m_nPointsFaults = record.get<6>();
				

				map<int, NumberToIndexEntry>::iterator it = m_numberIndice.find(nNum);
				if (it != m_numberIndice.end()) {
					ranking.m_nHorseIdx = it->second.m_nHorseIdx;
					ranking.m_nRiderIdx = it->second.m_nRiderIdx;
				}
				else {
					ranking.m_nHorseIdx = -1;
					ranking.m_nRiderIdx = -1;
				}
			}
		}
		catch (Exception&) {

		}

		try {
			string table = ns_base::string_format("TTIMEINFOS_EVENT%02d_CROSS01_J", m_eventNo);
			typedef Tuple<int, int, int, int, int, int, int, int, int> RankTuple;
			std::vector<RankTuple>  ranks;
			sessionSub << "SELECT C_NUM, C_STATUS, C_TIME, C_TIME_PENALTY, (C_TIME + C_TIME_PENALTY + C_TIME_FAULTS) as TIMES, C_POINTS_SURPASSING, C_POINTS_FAULTS, (C_POINTS_SURPASSING + C_POINTS_FAULTS) as POINTS, C_TIME_FAULTS from " + table + " order by C_STATUS, POINTS, TIMES", into(ranks), now;

			for each (RankTuple record in ranks)
			{
				int nNum = record.get<0>();
				CCRanking& ranking = m_ccRanks[nNum];
				ranking.m_nNum = nNum;
				ranking.m_nCrossStatus = record.get<1>();
				ranking.m_nCrossPoints = record.get<7>();
				ranking.m_nCrossTimes = record.get<2>();

				map<int, NumberToIndexEntry>::iterator it = m_numberIndice.find(nNum);
				if (it != m_numberIndice.end()) {
					ranking.m_nHorseIdx = it->second.m_nHorseIdx;
					ranking.m_nRiderIdx = it->second.m_nRiderIdx;
				}
				else {
					ranking.m_nHorseIdx = -1;
					ranking.m_nRiderIdx = -1;
				}

			}
		}
		catch (Exception&) {

		}

		string table = ns_base::string_format("TTIMEINFOS_EVENT%02d_DRESSAGE01_J", m_eventNo);
		typedef Tuple<int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int> RankTuple;
		std::vector<RankTuple>  ranks;
	
		try {
			
			sessionSub << "SELECT C_NUM, C_STATUS, C_TIME, C_TIME_PENALTY, (C_TIME + C_TIME_PENALTY + C_TIME_FAULTS) as TIMES, C_POINTS_SURPASSING, C_POINTS_FAULTS, (C_POINTS_SURPASSING + C_POINTS_FAULTS) as POINTS, C_TIME_FAULTS from " + table + " order by C_STATUS, POINTS, TIMES", into(ranks), now;

			for each (RankTuple record in ranks)
			{
				int nNum = record.get<0>();
				CCRanking& ranking = m_ccRanks[nNum];
				ranking.m_nNum = nNum;
				ranking.m_nDressageStatus = record.get<1>();
				ranking.m_nDressagePoints = record.get<7>();


				map<int, NumberToIndexEntry>::iterator it = m_numberIndice.find(nNum);
				if (it != m_numberIndice.end()) {
					ranking.m_nHorseIdx = it->second.m_nHorseIdx;
					ranking.m_nRiderIdx = it->second.m_nRiderIdx;
				}
				else {
					ranking.m_nHorseIdx = -1;
					ranking.m_nRiderIdx = -1;
				}

			}
		}
		catch (Exception&) {

		}

	}
}

bool EquestreDb::LoadJudges()
{
	if (m_discipline != 3/*Dressage*/)
		return false;

	Session sessionMain("SQLite", m_mainFile);

	try {
		m_judges.clear();

		string table1 = ns_base::string_format("TPARAMETERS_EVENT%02d", m_eventNo);
		typedef Tuple<string> ParamTuple;
		std::vector<ParamTuple> params;
		sessionMain << "SELECT C_VALUE from " + table1 + " WHERE C_PARAM like 'JUDGE_%'", into(params), now;

		int nRow = 0;
		string strPos = "";
		Judge judge;
		for each (ParamTuple record in params)
		{
			int nCol = nRow % 6;

			string strValue = record.get<0>();
			
			switch (nCol)
			{
			case 0: // Position
				judge.m_szPosition = strPos = strValue;
				judge.m_nIndex = ((int)nRow / 6) + 1;
				break;
			case 1: // Last Name
				judge.m_szName = strValue;
				break;
			case 2: // First Name
				judge.m_szName += " " + strValue;
				break;
			case 3: // Nation
				judge.m_szNation = strValue;
				break;
			case 4: // code
				if (!strValue.empty() && !strPos.empty()) {
					judge.m_nCode = std::stoi(strValue);
					m_judges[judge.m_nIndex] = judge;
				}
				break;
			default:
				break;
			}

			nRow ++;
		}
	}
	catch (Exception&) {
		return false;
	}

	return true;
}

EquestreDb* EquestreDb::getInstance()
{
	static EquestreDb inst;
	return &inst;
}


bool EquestreDb::Reload()
{
	return Load(m_szDbFolder);
}

int EquestreDb::getPosFromNum(int num)
{
	std::vector<StartListEntry>::iterator it = m_startlist.begin();
	for (; it != m_startlist.end(); it++) {
		if (it->m_nNumber == num)
			return it->m_nPos;
	}
	return -1;
}


void EquestreDb::SetCountry(string country) {
	this->m_country = country;
}