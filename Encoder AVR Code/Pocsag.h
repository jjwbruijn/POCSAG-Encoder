/*
 * Pocsag.h
 *
 * Created: 24-9-2014 15:02:21
 *  Author: Jelmer Bruijn
 */ 


#ifndef POCSAG_H_
#define POCSAG_H_


#define SYNCWORD 0x7CD215D8
#define IDLEWORD 0x7A89C197
#define SENDMARK GPIOPORT|=(1<<GPIO_DATA_IN)
#define SENDSPACE GPIOPORT&=~(1<<GPIO_DATA_IN)



#define F_CPU 16000000UL
#define CODEWORDSIZE 32
#define DATABITS_PER_FRAME 20
#define MAX_FRAME_COUNT 15
#define BAUD 1200
#define TIMERLEN (F_CPU/BAUD)
#define PREAMBLELENGTH 576
#define STATE_OFF 0
#define STATE_PREAMBLE_MARK 1
#define STATE_PREAMBLE_SPACE 2
#define STATE_SYNCWORD 3
#define STATE_FRAME 4
#define MSG_BUFFER_LEN 500

volatile uint8_t xmitState;
volatile uint16_t preambleLeft;
volatile uint8_t codeWordBit;
volatile uint8_t frameNo;

uint32_t frame[16];
uint8_t timercorrection;

volatile uint32_t pagerric;
volatile uint16_t baudrate;
volatile uint8_t messagetype;
volatile uint8_t functiontype;
char message[MSG_BUFFER_LEN];
volatile char* messagep;
volatile uint8_t bitalign;

char bitswitch(char b);
uint32_t createcrc(uint32_t in);
uint32_t MakeMessage(uint32_t message);
uint8_t DecodeNumber(char c);
void SetupTimer(void);
void ClearBuffer();
void StartXmit();
void StopXmit();
void ContinueXmit();
uint32_t MakeAddressFrame(uint32_t ric, uint8_t type);
uint8_t GetStartFrame(uint32_t ric);
void Prepare(void);
#endif /* POCSAG_H_ */