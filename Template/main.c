#include <p24fxxxx.h>
#include <stdio.h>
#include <string.h>

// Configuration Bits
#ifdef __PIC24FJ64GA004__ //Defined by MPLAB when using 24FJ64GA004 device
_CONFIG1( JTAGEN_OFF & GCP_OFF & GWRP_OFF & COE_OFF & FWDTEN_OFF & ICS_PGx1 & IOL1WAY_ON) 
_CONFIG2( FCKSM_CSDCMD & OSCIOFNC_OFF & POSCMOD_HS & FNOSC_PRI & I2C1SEL_SEC)
#else
_CONFIG1( JTAGEN_OFF & GCP_OFF & GWRP_OFF & COE_OFF & FWDTEN_OFF & ICS_PGx2) 
_CONFIG2( FCKSM_CSDCMD & OSCIOFNC_OFF & POSCMOD_HS & FNOSC_PRI)
#endif

void configPorts();
void setNormal();
void setStandby();
int changeMode(int state);
void delay(int t);
void moveRight();
void moveLeft();
void stopMotor();
void usart_init();
char usart_read_char();
char* usart_read_string();
void usart_write_char(char c);
void usart_write_string();
void adc_init();
int adc_read(int channel);
int solar_tracker(int ldr_1, int ldr_2, int state);

void configPorts() {
	TRISDbits.TRISD6 = 1; // Button to change mode (Standby and normal)
	TRISAbits.TRISA7 = 1; // Button to move right
	TRISAbits.TRISA6 = 0; // LED to indicate if the motor is ON or OFF
	TRISAbits.TRISA0 = 0; // Motor is moving right
	TRISAbits.TRISA1 = 0; // Motor is moving left
	TRISDbits.TRISD7 = 1; // Button to move left
	TRISAbits.TRISA7 = 1; // Button to move right
}

void setNormal() 
{
	PORTAbits.RA6 = 1;
}

void setStandby() 
{
	stopMotor();
	PORTAbits.RA6 = 0;
}

int changeMode(int state) // check debounce 
{
	state = !state;
	
	if(state)
		setNormal();
	else
		setStandby();
	
	usart_write_string("Mudou de modo");
	return state;
}

void delay(int t)
{
    int i;
    for(i=0;i<t;i++);
}

void moveRight()
{
	LATAbits.LATA1 = 0;
	LATAbits.LATA0 = 1;
}

void moveLeft()
{
	LATAbits.LATA0 = 0;
	LATAbits.LATA1 = 1;
}

void stopMotor() 
{
	LATAbits.LATA0 = 0;
	LATAbits.LATA1 = 0;
}

void usart_init()
{
	U2BRG = 25; // BaudRate = 9600 bits/s 4MHz (Page 11 Datasheet)
	U2MODE = 0x8000; // 8-bit data - no parity, 1 STOP bit
	U2STA = 0;  
	U2STAbits.UTXEN = 1; // Enable transmit
	//IFSxbits.UxRXIF ??
	//UxTXIE ??
	//UxRXIE ??
}

char usart_read_char()
{
	// waits until the buffer is empty
	while(!U2STAbits.URXDA) // URXDA = 0 when receive buffer is empt
		;
	// then we can read the char
	return U2RXREG;
}

char* usart_read_string()
{
	int i=0;
	char *text;

	do
	{
		text[i] = usart_read_char();
		i++;	
	}
	while(text[i]!='\0' && text[i]!='\r' && text[i]!='\n');

	text[i]='\0';
	return text;
}

void usart_write_char(char c)
{
	while(U2STAbits.UTXBF)
		;
	U2TXREG=c;
}

void usart_write_string(char *text)
{
	int i=0;
	
	while(text[i] != '\0')
	{
		usart_write_char(text[i]);
		i++;
	}
}

void adc_init()
{
	// 15 bits to configure: AN2 and AN3 analog input 
	// 1111111111110011 -> fff3
	AD1PCFG = 0xFFF3; // configure a/d port 
	AD1CON1 = 0; // a/d control register 1  		
	AD1CON2 = 0; // a/d control register 2
	AD1CON3 = 0; // a/d control register 3
	AD1CSSL = 0; // Use the channel selected by the CH0SA bits (no scan)
	AD1CON1bits.ADON = 1; // turns ADC ON
}

int adc_read(int channel)
{
	AD1CHS = channel; // A/D input channel select register
	AD1CON1bits.SAMP = 1; // Sampling begins
	// delay
	delay(2000);
	AD1CON1bits.SAMP = 0; // Sampling ends
	
	for(;;)
	{
		if(AD1CON1bits.DONE) // w8s until the conversion is completed
			break;
	}
	return ADC1BUF0;
}

int solar_tracker(int ldr_1, int ldr_2, int state)
{
	if(ldr_1 < 20 && ldr_2 < 20) // noite - atribuir o 20 a uma variavel
		stopMotor();
	else if(ldr_1 - ldr_2 > 100) // if there's a noticable difference then it rotates 
	{
		moveLeft();
		delay(2000);
		
		while(ldr_1 - ldr_2 > 100 && state==1)
		{	
			ldr_1 = adc_read(2); // left
			ldr_2 = adc_read(3); // right
			if(!PORTDbits.RD6) {
            	state = changeMode(state);
				stopMotor();	
				break;
			}
		}
	}
	else if(ldr_2 - ldr_1 > 100)
	{
		delay(2000);
		moveRight();
		
		while(ldr_2 - ldr_1 > 100 && state==1)
		{
			ldr_1 = adc_read(2); // left
			ldr_2 = adc_read(3); // right
			if(!PORTDbits.RD6) {
            	state = changeMode(state);
				stopMotor();	
				break;
			}
		}
	}
	else
	{
		stopMotor();
		delay(2000);
	}
	return state;	
}

int main(void)
{	
	int state = 0;
	configPorts();
	usart_init();
	adc_init();
	int ldr_1, ldr_2;
	ldr_1 = ldr_2 = 0;
	char str[100];

	while(1)
	{
		delay(20);
		if(!PORTDbits.RD6)
        {
            state = changeMode(state);
        }
		if(state)
		{
			/*
			if(!PORTDbits.RD7)
				moveLeft();
			if(!PORTAbits.RA7)
				moveRight();
			//str = usart_read_string();
			ldr_2 = adc_read(2);
			ldr_3 = adc_read(3);
			sprintf(str, "AN2: %d\n",ldr_2);
			usart_write_string(str);
			sprintf(str, "AN3: %d\n\n",ldr_3);
			usart_write_string(str);
			delay(2000);
			*/
 
			ldr_1 = adc_read(2); // left
			ldr_2 = adc_read(3); // right
			//temp = adc_read(4);
			sprintf(str, "AN2: %d\n",ldr_1);
			usart_write_string(str);
			sprintf(str, "AN3: %d\n\n",ldr_2);
			usart_write_string(str);
			state = solar_tracker(ldr_1, ldr_2, state);/*temp*/
	
		}
					
	}
}
