/*
 * Serial.h
 *
 * Created: 25-11-2014 22:29:24
 *  Author: Jelmer Bruijn
 */ 


#ifndef SERIAL_H_
#define SERIAL_H_


volatile char* bufferp;
void InitSerial(void);
void SerialRXDisable();
void SerialRXEnable();
void ProcessSerialData();
void SerialXmitStart();
void SerialXmitStartWait();
void PocsagTXComplete(void);

#endif /* SERIAL_H_ */