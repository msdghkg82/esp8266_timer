#ifndef API_HANDLERS_H_
#define API_HANDLERS_H_

void handle_Root();
void handle_GetStatus();
void handle_SetTimer();
void handle_SetSysTimestamp();
void handle_GetDate();
void handle_SetSchedule();
void handle_GetSchedules();
void handle_ResetSchedules();
void handle_RemoveSchedule();
void handle_wifisetting();
void handle_wifitoggle();
void handle_GetLog();
void handle_RemoveLog();
void handle_ChangeLang();

#endif /* API_HANDLERS_H_ */