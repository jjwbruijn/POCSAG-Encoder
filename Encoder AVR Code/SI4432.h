/*
 * SI4432.h
 *
 * Created: 24-9-2014 15:16:55
 *  Author: Jelmer Bruijn
 */ 


#ifndef SI4432_H_
#define SI4432_H_

// GPIO0 = PORTA7
// GPIO1 = PORTA6
// GPIO2 = PORTA5
// NSEL = PORTB0

#define SPIPORT PORTB
#define SPIPORT_DDR DDRB
#define SPI_SS PORTB2
#define SPI_CLK PORTB5
#define SPI_MOSI PORTB3

#define GPIOPORT PORTB
#define GPIODDR DDRB
#define GPIO_DATA_CLK PORTB0 //gpio1
#define GPIO_DATA_IN PORTB1 //gpio0

uint32_t xmitfreq;
int16_t offset;

void SIXmit();
void SIXmitStop();
void SetupSPI(void);
void WaitForSignal(void);
void SIRX(void);
void SetupFreq();


#endif /* SI4432_H_ */