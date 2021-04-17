// Authors: Bruno Vieira, Francisco Róis and Nikita Dyskin

#include <xc.h>
#include <stdio.h>

#pragma config FOSC = HS // Oscillator Selection bits (HS oscillator)
#pragma config WDTE = OFF // Watchdog Timer Enable bit
#pragma config PWRTE = OFF // Power-up Timer Enable bit
#pragma config BOREN = OFF // Brown-out Reset Enable bit
#pragma config LVP = OFF // Low-Voltage (Single-Supply) In-Circuit Serial Programming Enable bit
#pragma config CPD = OFF // Data EEPROM Memory Code Protection bit
#pragma config WRT = OFF // Flash Program Memory Write Enable bits
#pragma config CP = OFF // Flash Program Memory Code Protection bit

void configPorts();
void moveLeft();
void moveRight();
void stopMotor();
void setNormal();
void setStandby();
int changeMode(int state);
void delay(int t);
void adc_init();
void usart_init();
void usart_read_char();
void usart_read_string();

void configPorts() 
{
    // 0 stands for output and 1 for input
    TRISDbits.TRISD0 = 0; 
    TRISDbits.TRISD1 = 0; // If its 1 - Motor moving left
    TRISDbits.TRISD2 = 0;
    TRISDbits.TRISD3 = 0;
    TRISDbits.TRISD4 = 0;
    TRISDbits.TRISD5 = 0;
    TRISDbits.TRISD6 = 0;
    TRISDbits.TRISD7 = 0;
    
    TRISBbits.TRISB0 = 0;
    TRISBbits.TRISB1 = 0; // If its 1 - Motor moving right
    TRISBbits.TRISB2 = 0;
    TRISBbits.TRISB3 = 1; // Button to change mode (Standby and normal)
    TRISBbits.TRISB4 = 0;
    TRISBbits.TRISB5 = 0;
    TRISBbits.TRISB6 = 0;
    TRISBbits.TRISB7 = 0;
    
    TRISAbits.TRISA0 = 1;
    TRISAbits.TRISA1 = 1;
    
    // 0 stands for off
    PORTDbits.RD0 = 0;
    PORTDbits.RD1 = 0; 
    PORTDbits.RD2 = 0; 
    PORTDbits.RD3 = 0; 
    PORTDbits.RD4 = 0; 
    PORTDbits.RD5 = 0; 
    PORTDbits.RD6 = 0;
    PORTDbits.RD7 = 0;
    
    PORTBbits.RB0 = 0;
    PORTBbits.RB1 = 0; 
    PORTBbits.RB2 = 0; 
    PORTBbits.RB3 = 1; 
    PORTBbits.RB4 = 0; 
    PORTBbits.RB5 = 0; 
    PORTBbits.RB6 = 0;
    PORTBbits.RB7 = 0; 
}

void moveLeft()
{
    PORTBbits.RB1 = 0; // moving right led off
    PORTDbits.RD1 = 1; // moving left led on
}

void moveRight()
{
    PORTDbits.RD1 = 0; // moving left led off
    PORTBbits.RB1 = 1; // moving right led on
}

void stopMotor()
{
    PORTDbits.RD1 = 0; // moving left led off
    PORTBbits.RB1 = 0; // moving right led off
}

void setNormal() 
{
    PORTDbits.RD3 = 1; // d3 led on
}

void setStandby() 
{
    stopMotor();
    PORTDbits.RD3 = 0; // d3 led off
}

int changeMode(int state) 
{
    state = !state;
    
    if(state==1)
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

void adc_init()
{
    ADCON0=0;
    ADCON1=0;
    
}

void usart_init()
{
    BRGH = 1;   // high baud rate select bit: 1 - high speed
    SPBRG = 25; // page 114 datasheet - baud rate = 9.6k and f=4Mhz
    SYNC = 0; // asynchronous mode
    SPEN = 1;  // serial port enable bit enabled
    RX9 = 0; // 8 bits reception
    TX9 = 0; // 8 bits transmission
    TXEN = 1; // transmit enable bit
    CREN = 1; // enables continuous receive
}

char usart_read_char()
{
    if(RCIF) // flag bit RCIF will be set when reception is complete
    {
        for( ; ; )
            if(RCIF)
                break;
        return RCREG;
    }
    else
        return 0;
}

void usart_read_string()
{
    char* text;
    int i=0;
    
    do
    {
        text[i] = usart_read_char();
        i++;
    }
    while(text[i]!='\0' && text[i]!='\r' && text[i]!='\r');
    
    text[i]='\0';
}



int main(void)
{
    int state = 0; // 0 stands for standby mode and 1 for normal 
    
    configPorts();
    
    while(1)
    {
        delay(20);
        if(!PORTBbits.RB3)
        {
            state = changeMode(state);
        }
        
    }
}

