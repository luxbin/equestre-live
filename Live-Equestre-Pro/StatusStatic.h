#pragma once
class CStatusStatic : public CStatic
{
	int m_nStatus;
public:
	CStatusStatic();
	virtual ~CStatusStatic();


	void SetStatus(int status);

};

