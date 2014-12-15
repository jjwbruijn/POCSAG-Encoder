/*
 * Settings.h
 *
 * Created: 27-11-2014 12:12:11
 *  Author: Jelmer Bruijn
 */ 


#ifndef SETTINGS_H_
#define SETTINGS_H_

void SaveSettings(void);
void LoadSettings(void);
void SaveThreshold();

void SavePowerLossMsg(void);
void SaveServerFailMsg(void);
void SaveServerResumeMsg(void);
void LoadServerResumeMsg(void);
void LoadPowerLossMsg(void);
void LoadServerFailMsg(void);
uint8_t ServerResumeMsgSet(void);
uint8_t ServerFailMsgSet(void);
uint8_t PowerLossMsgSet(void);
#endif /* SETTINGS_H_ */