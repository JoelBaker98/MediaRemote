class CTimer
{
public:
	CTimer();
	CTimer(unsigned int);
	void SetStartTime();
	void SetStartTime(__int64);
	void SetEndTime();
	void SetSeconds(unsigned int);
	void SetMaxSeconds();
	__int64 GetStartTime();
	__int64 GetEndTime();
	__int64 GetDifference();
	unsigned int GetSeconds();
	__int64 CalculateNumberSeconds();

private:
	unsigned int Seconds;
	__int64 StartTime;
	__int64 EndTime;
	__int64 Frequency;
};