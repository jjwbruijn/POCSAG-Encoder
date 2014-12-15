/*
 * Encoder.h
 *
 * Created: 27-11-2014 16:05:58
 *  Author: Jelmer Bruijn
 */ 


#ifndef ENCODER_H_
#define ENCODER_H_

volatile uint16_t seconds;

#define WATCHDOG_MAX 300
uint8_t subseconds;
uint8_t ADCval;
volatile uint8_t ADCThreshold;

volatile uint8_t DeviceState;
#define STATE_NORMAL 0
#define STATE_POWER_LOSS 1
#define STATE_POWER_LOSS_SENT 2
#define STATE_SERVER_FAIL 3
#define STATE_SERVER_FAIL_SENT 4
void SetupSecTimer();
void SetupADC();

#endif /* ENCODER_H_ */