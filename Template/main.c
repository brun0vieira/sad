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


int main(void)
{	
	int state = 0;
	configPorts();
	//usart_init();
	//char *str;

	while(1)
	{
		delay(20);
		if(!PORTDbits.RD6)
        {
            state = changeMode(state);
        }
		if(state)
		{
			if(!PORTDbits.RD7)
				moveLeft();
			if(!PORTAbits.RA7)
				moveRight();
			/*str = usart_read_string();
			usart_write_string(str);*/
		}
					
	}
}
