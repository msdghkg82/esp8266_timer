#include <scheduler.h>
#include <Arduino.h>

#include <savefile.h>
#include <DateConverter.h>
#include <RTC.h>
#include <algorithm>
#include <log.h>

Schedule_t ScheduleArray[50] = {};
uint8_t ScheduleCount = 0;

void sortSchedules()
{
	std::sort(ScheduleArray, ScheduleArray + ScheduleCount,
			[](const Schedule_t& a, const Schedule_t& b)
		{
			if(a.flag != b.flag)
			{
				return a.flag > b.flag;
			}
			return a.ScheduleTimeStamp < b.ScheduleTimeStamp;
		}
	);
}

bool RemoveSchedule(uint16_t id)
{
	for(uint8_t i = 0; i < ScheduleCount; i++)
	{
		if(ScheduleArray[i].id == id)
		{
			ScheduleArray[i] = Schedule_t{};
			sortSchedules();
			ScheduleCount--;
			Log("Schedule removed, id: " + String(id));
			saveSchedulesFile();
			return true;
		}
	}

	return false;
}

bool AddSchedule(time_t timestamp, uint8_t state, Repeat_t interval, uint16_t id)
{
	if(ScheduleCount < 50)
	{
		ScheduleArray[ScheduleCount].ScheduleTimeStamp = timestamp;

		if(state == 0 || state == 1)
		{
			ScheduleArray[ScheduleCount].state = state;
		}
		else return false;

		
		if(once <= interval && interval <= monthly)
		{
			ScheduleArray[ScheduleCount].interval = interval;
		}
		else return false;
		
		for(uint8_t i = 0; i < ScheduleCount; i++)
		{
			if(ScheduleArray[i].id == id)
			{
				return false;
			}
		}
		
		ScheduleArray[ScheduleCount].id = id;

		ScheduleArray[ScheduleCount].flag = true;
		ScheduleCount++;
		sortSchedules();
		Log("Schedule added, timestamp: " + String(timestamp) + 
			", state: " + String(state) + 
			", interval: " + String(interval) + 
			", id: " + String(id));
		saveSchedulesFile();
		return true;
	}
	else return false;
}

bool ResetSchedules()
{
    for(uint8_t i = 0; i < 50; i++)
	{
		ScheduleArray[i] = Schedule_t{};
	}

	ScheduleCount = 0;
	Log("Schedules reset");
	if(!saveSchedulesFile()) return false;
    
    return true;
}

uint32_t handleMonthlySchedules(Schedule_t& Schedule)
{
	struct tm miladi{};
	time_t timestamp = Schedule.ScheduleTimeStamp;
	localtime_r(&timestamp, &miladi);
	Date_t shamsi = gregorianToPersian_Claude(miladi.tm_year + 1900, miladi.tm_mon + 1, miladi.tm_mday);

	if(shamsi.day <= 29)
	{
		if(shamsi.month <= 6)
		{
			return (31 * 86400);
		}
		else if(shamsi.month <= 11)
		{
			return (30 * 86400);
		}
		else if(shamsi.month == 12)
		{
			if(isPersianLeapYear_Claude(shamsi.year))
			{
				return (30 * 86400);
			}
			else
			{
				return (29 * 86400);
			}
		}
	}

	if(shamsi.day == 30)
	{
		if(shamsi.month == 11 && !isPersianLeapYear_Claude(shamsi.year))
		{
			return ((30 + 29) * 86400);
		}
		else
		{
			return (30 * 86400);
		}
	}

	if(shamsi.day == 31)
	{
		if(1 <= shamsi.month && shamsi.month <= 5)
		{
			return (31 * 86400);
		}
		else if(shamsi.month == 6)
		{
			return ((31 + (5 * 30) + (isPersianLeapYear_Claude(shamsi.year) ? 30 : 29)) * 86400);
		}
	}

	//for warning purposes, should never reach here
	return 0;
}

void ProcessSchedules()
{
	if(ScheduleCount == 0) return;
	if(ScheduleArray[0].flag == false) return;
	if(ScheduleArray[0].ScheduleTimeStamp > systemtimestamp) return;

	digitalWrite(RELAY_PIN, ScheduleArray[0].state);

	switch(ScheduleArray[0].interval)
	{
		case once:
			ScheduleArray[0] = Schedule_t{};
			ScheduleCount--;
			break;
		case daily:
			ScheduleArray[0].ScheduleTimeStamp += 86400;
			break;
		case weekly:
			ScheduleArray[0].ScheduleTimeStamp += (7 * 86400);
			break;
		case monthly:
			ScheduleArray[0].ScheduleTimeStamp += handleMonthlySchedules(ScheduleArray[0]);
			break;
	}

	sortSchedules();
	saveSchedulesFile();
}

void Scheduler_Init()
{
	pinMode(RELAY_PIN, OUTPUT);
	digitalWrite(RELAY_PIN, LOW);
}