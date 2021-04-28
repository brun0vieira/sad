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
void stop();

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

int main(void)
{	
	int state = 0;
	configPorts();
	
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
		}			
	}
}
