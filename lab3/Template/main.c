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

#define MAX_TEMPERATURE 245

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
int solar_tracker(int ldr_1, int ldr_2, int state, int temperature);
void check_temperature(int temperature);
void debounce();
void print_aqc2_status(int ldr_1, int ldr_2, int temperature, int state);
void config_I2C();
int read_I2C();
int write_I2C();

/*
	<AQC2>
		<ldr1> value </ldr1>
		<ldr2> value </ldr2>
		<temperature> value </temperature>
		<state> value </state>
	</AQC2>
*/

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
	int success, msg;
	char msg_string[50];

	PORTAbits.RA6 = 1;
	usart_write_string("\n<Aviso>\n	<Mensagem>Modo normal ativado.</Mensagem>\n</Aviso>");
	success = write_I2C();
	
	if(success){
		msg = read_I2C();
		sprintf(msg_string, "LDR diff=%d, ", msg); 
		usart_write_string(msg_string);
	}
}

void setStandby() 
{
	stopMotor();
	PORTAbits.RA6 = 0;
	usart_write_string("\n<Aviso>\n	<Mensagem>Modo standby ativado.</Mensagem>\n</Aviso>");
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
	// binary: 1111111111100011 -> hex: ffe3
	AD1PCFG = 0xFFE3; // configure a/d port 
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

int solar_tracker(int ldr_1, int ldr_2, int state, int temperature)
{
	if(ldr_1 < 20 && ldr_2 < 20) // noite - atribuir o 20 a uma variavel
	{
		stopMotor();
		usart_write_string("\n<Aviso>\n	<Mensagem>Motor a parar.</Mensagem>\n</Aviso>");
	}
	else if(ldr_1 - ldr_2 > 100) // if there's a noticable difference then it rotates 
	{
		moveLeft();
		usart_write_string("\n<Aviso>\n	<Mensagem>Motor a rodar para a esquerda.</Mensagem>\n</Aviso>");
		delay(2000);
		
		while(ldr_1 - ldr_2 > 100 && state==1)
		{	
			ldr_1 = adc_read(2); // left
			ldr_2 = adc_read(3); // right
			print_aqc2_status(ldr_1,ldr_2,temperature,state);
		
			if(!PORTDbits.RD6)
			{
				debounce();
				
				state = changeMode(state);
				stopMotor();
				break;
			}
		}
		usart_write_string("\n<Aviso>\n	<Mensagem>Seguidor solar bem posicionado. Motor a parar.</Mensagem>\n</Aviso>");
	}
	else if(ldr_2 - ldr_1 > 100)
	{
		moveRight();
		usart_write_string("\n<Aviso>\n	<Mensagem>Motor a rodar para a direita.</Mensagem>\n</Aviso>");
		delay(2000);
		while(ldr_2 - ldr_1 > 100 && state==1)
		{
			ldr_1 = adc_read(2); // left
			ldr_2 = adc_read(3); // right
			print_aqc2_status(ldr_1,ldr_2,temperature,state);

			if(!PORTDbits.RD6)
			{
				debounce();
				
				state = changeMode(state);
				stopMotor();
				break;
			}
		}
		usart_write_string("\n<Aviso>\n	<Mensagem>Seguidor solar bem posicionado. Motor a parar.</Mensagem>\n</Aviso>");
	}
	else
	{
		stopMotor();
		delay(2000);
	}
	return state;	
}

void check_temperature(int temperature)
{
	if(temperature>=MAX_TEMPERATURE)
	{
		usart_write_string("\n<Aviso>\n	<Mensagem>Temperatura acima do recomendado. Motor a parar.</Mensagem>\n</Aviso>");
		stopMotor();
		
	}
}

void debounce()
{
	while(!PORTDbits.RD6)
		delay(1000);
}

void print_aqc2_status(int ldr_1, int ldr_2, int temperature, int state)
{
	char str[150];
	sprintf(str,"\n<AQC2>\n	<ldr1> %d </ldr1>\n	<ldr2> %d </ldr2>\n	<temperature> %d </temperature>\n	<state> %d </state>\n</AQC2>",ldr_1,ldr_2,temperature,state);
	usart_write_string(str);
}

void config_I2C()
{
	int baudRate = 39;
	I2C2BRG = baudRate; // sets the baud rate pretended
	I2C2CONbits.I2CEN = 1; // enables I2C
}

int read_I2C()
{
	int message;

    I2C2CONbits.SEN = 1; // 
    
	// First we need to w8 until the start condiition is completed
	while(I2C2CONbits.SEN) ;	

    I2C2TRN = ((0x0A<<1) | 1); // Shifts the address 1 bit to the left and adds a 1
    
	// transmit buffer full bit
    while(I2C2STATbits.TBF) ; // w8s until the transmission ends

    // transmit status bit
    while(I2C2STATbits.TRSTAT) ; // w8s until the master transmission ends
  
    if(I2C2STATbits.ACKSTAT) // checks if the master received an acknowledge
        return 0;
    
    I2C2CONbits.RCEN = 1; // master reception
	
	while(!I2C2STATbits.RBF) ; // w8s until the end of the reception

	message = (I2C2RCV); // MSB
	
	while(I2C2CONbits.RCEN) ; // w8s for the start condition to be complete

	I2C2CONbits.ACKDT = 1; // sends a nack
	
	I2C2CONbits.ACKEN = 1;
	
	while(I2C2CONbits.ACKEN) ; 
	
	I2C2CONbits.PEN = 1;
	
	while(I2C2CONbits.PEN) ; 
    
	return message;
}

int write_I2C()
{
	I2C2CONbits.SEN = 1;

	// First we need to w8 until the start condiition is completed
	while(I2C2CONbits.SEN) ;
	
	// transmit register
	I2C2TRN = (0x0A<<1 | 0); // Shifts the address 1 bit to the left and adds a 0
	
	// transmit buffer full status
	while(I2C2STATbits.TBF) ; // w8s until the transmission ends
	
	// transmit status bit
	while(I2C2STATbits.TRSTAT) ; // w8s until the master transmission ends

	// acknowledge status bit
	if(I2C2STATbits.ACKSTAT) // checks if the master received an acknowledge
		return 0;
	// transmit register
	I2C2TRN = 0x07;
	
	while(I2C2STATbits.TBF) ; // w8s until the transmission ends
	
	while(I2C2STATbits.TRSTAT) ; // w8s until the master transmission ends
	
	if(I2C2STATbits.ACKSTAT) // checks if the master received an acknowledge
		return 0;
	
	I2C2CONbits.PEN = 1; // Stop event
	
	while(I2C2CONbits.PEN) ; // w8s until the end of the event
	
	return 1;
}

int main(void)
{	
	int state = 0;
	configPorts();
	usart_init();
	adc_init();
	config_I2C();
	int ldr_1, ldr_2, temperature;
	ldr_1 = ldr_2 = 0;

	while(1)
	{
		delay(20);
		if(!PORTDbits.RD6)
		{
			debounce();
			state = changeMode(state);
		}
		if(state)
		{
			// we no longer need to read the adc values as we receive the ldr diff from the arduino
			//ldr_1 = adc_read(2); // left
			//ldr_2 = adc_read(3); // right
			//temperature = adc_read(4);
			//print_aqc2_status(ldr_1,ldr_2,temperature,state);
			//state = solar_tracker(ldr_1, ldr_2, state, temperature);
		}			
	}
}
