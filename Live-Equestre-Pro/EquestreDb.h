#pragma once
#include <string>
#include  <vector>
#include <map>
#include <Poco/DateTime.h>

using namespace std;

class Rider {
public:
	string		m_szFirstName;
	string		m_szLastName;
	int			m_nIdx;
	string		m_szNation;
	string      m_szClub;
	string      m_szCity;
	string      m_szLicense;
	Poco::DateTime	m_dateBirthday;
};

class Horse {
public:
	string		m_szName;
	int			m_nIdx;
	int			m_nAge;
	string		m_szOwner;
	Poco::DateTime	m_dateBirthday;
	string      m_szGender;
	string      m_szFnchPassport;
	string      m_szFather;
	string      m_szMother;
	string      m_szFatherOfMother;
	string      m_szSignalementLabel;
};

class CCRanking {
public:
	CCRanking() {
		m_nNum = m_nHorseIdx = m_nRiderIdx = m_nJumpPoints = m_nJumpStatus = m_nDressagePoints = m_nDressageStatus = m_nCrossPoints = m_nCrossStatus = m_nJumpTimes  = m_nCrossTimes = 0;
	};

	int			m_nNum;
	int			m_nHorseIdx;
	int			m_nRiderIdx;
	
	int			m_nJumpPoints;
	int			m_nJumpTimes;
	int			m_nJumpStatus;

	int			m_nDressagePoints;
	int			m_nDressageStatus;
	
	int			m_nCrossPoints;
	int			m_nCrossTimes;
	int			m_nCrossStatus;
};

class Ranking {
public:
	Ranking() {
		m_nNum = m_nStatus = m_nTime = m_nTimePenalty = m_nTimes = m_nPointsSurpassing = m_nPointsFaults = m_nPoints = m_nFigureMark = 0;
	};

	int			m_nNum;
	int			m_nStatus;
	int			m_nTime;
	int			m_nTimePenalty;
	int			m_nTimes;

	int			m_nPointsSurpassing;
	int			m_nPointsFaults;
	int			m_nPoints;

	int			m_nFigureMark;			// Point for dressage
	string		m_szJudgeScores;	

};

class Team {
public:
	Team() {
		for (int i = 0; i < 5; i++) {
			m_nMembers[i] = 0;
		}
		m_szName = "";
		m_nNum = 0;
	}

	int			m_nNum;
	string		m_szName;
	int			m_nMembers[5];
};

class Judge {
public:
	Judge() {
		m_nCode = 0;
		m_nIndex = 0;
	}
	int		m_nIndex;
	string	m_szPosition;
	string	m_szName;
	string	m_szNation;
	int		m_nCode;
};

class Rank {
public:
	Rank() {

	}
	int		pos;
	int		num;
	int		time;
	int		timePlus;
	int		figureMark;
	int		point;
	int		pointPlus;
	int		status;

};

class StartListEntry {
public:
	StartListEntry() {
		m_nPos = m_nNumber = 0;
		m_nHorseIdx = m_nRiderIdx = 0;
		m_nStartTime = 0;
	}

	int			m_nPos;
	int			m_nNumber;
	int			m_nHorseIdx;
	int			m_nRiderIdx;
	int			m_nStartTime;
};

class NumberToIndexEntry {
public:
	NumberToIndexEntry() {
		m_nNumber = m_nRiderIdx = m_nHorseIdx = 0;
	};

	int		m_nNumber;
	int		m_nRiderIdx;
	int		m_nHorseIdx;
};


class EquestreDb
{
protected:
	EquestreDb();
	~EquestreDb();

public:
	bool Load(std::string folder);
	bool LoadCoursePoints();
	int	 GetPublishFilesUpload();
	bool GetUploadedPdfInfo(int& printNum, std::string& title, std::string& filename);
	bool LoadJudges();
	void UpdateRanking();
	void UpdateTeamRanking();
	void UpdateCCRanking();
//	void SetAtStart(std::vector<string> listAtStart);

	void SetCountry(std::string country);

	bool		m_bLoaded;

	string		m_szDbFolder;
	string		m_mainFile;
	int			m_eventNo;
	int			m_showNo;

	static EquestreDb* getInstance();

	// properties
	string		m_szTitle;
	string		m_szEvent;
	string      m_gameBeginTime;
	Poco::DateTime		m_dateStart;
	Poco::DateTime		m_dateEnd;
	Poco::DateTime		m_dateEvent;
	int					m_timeEvent;
	int					m_roundNumber;
	int					m_jumpoffNumber;
	int                 m_twoPhaseIntegrated;
	int					m_twoPhaseDiffered;//TWO_PHASES_DIFFERED
	int					m_roundTableTypes[4];
	int                 m_jumpoffTableTypes[4];
	int                 m_round;
	int                 m_jumpoff;
	int					m_discipline;
	int                 m_allowedTimeRounds[4];
	int                 m_allowedTimeJumpoffs[4];
	int                 m_againstTimeClockRounds[4];
	int                 m_againstTimeClockJumpoffs[4];
	int					m_modeTeam;
	int					m_modeTeamRelay;

	string				m_schedulerNumber;
	string				m_category;
	string				m_notes;
	string				m_height;
	string				m_initAwardListAmount;

	int					m_crossOptimumTime;
	int					m_crossMinimumTime;
	int					m_crossLimitTime;

	int					m_crossSurpassingBaseTime;
	int					m_crossSurpassingPoints;
	int					m_crossSurpassingMinBaseTime;
	int					m_crossSurpassingMinPoints;

	int					m_dressageJudgeScoreTotal;

	map<int, Rider>		m_riders;
	map<int, Horse>		m_horses;
	map<int, Judge>		m_judges;
	map<int, Ranking>   m_roundRanks[4];
	map<int, Ranking>   m_jumpoffRanks[4];
	map<string, Team>	m_teams;
	map<int, int>		m_coursePoints;

	//map<int, Ranking>	m_jumpRanks;
	//map<int, Ranking>	m_crossRanks;
	//map<int, Ranking>	m_dressageRanks;

	map<int, CCRanking>	m_ccRanks;

	int					m_prevCoursePoints;

	std::string         m_country;

	int					m_eventing;

	int					m_manualDiscipline;

	int					m_publishUploadFiles;




	std::vector<StartListEntry>	m_startlist;
	map<int, NumberToIndexEntry>	m_numberIndice;

//	map<int, int>		m_atStart;

	bool Reload();
	int getPosFromNum(int num);

protected:
};

#define MyEquestreDb	EquestreDb::getInstance()


