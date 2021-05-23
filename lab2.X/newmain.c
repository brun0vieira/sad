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

#define MAX_TEMPERATURE 40

void configPorts();
void moveLeft();
void moveRight();
void stopMotor();
void setNormal();
void setStandby();
int changeMode(int state);
void delay(int t);
void adc_init();
int adc_read(int ch);
void usart_init();
char usart_read_char();
char* usart_read_string();
void usart_write_char(char c);
void usart_write_string(char *text);
void debounce(int port);
int solar_tracker(int ldr_1, int ldr_2, int temperature, int state);
void print_aqc1_status(int ldr_1, int ldr_2, int temperature, int state);
int check_temperature(int temperature);
void change_heater_state();
void change_cooler_state();

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
    TRISBbits.TRISB4 = 1; // Button to invert heater mode (on or off)
    TRISBbits.TRISB5 = 1; // Button to invert cooler mode (on or off)
    TRISBbits.TRISB6 = 0;
    TRISBbits.TRISB7 = 0;
    
    TRISAbits.TRISA0 = 1; // ldr_1
    TRISAbits.TRISA1 = 1; // ldr_2
    TRISAbits.TRISA2 = 1; // temp
    TRISCbits.TRISC5 = 0; // heater
    TRISCbits.TRISC2 = 0; // cooler
    
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
    ADCON0=0; // a/d control register 0 - controls the operation of the a/d module
    ADCON1=0; // a/d control register 1 - configures the functions of the port pins
    ADCON0bits.ADON = 1; // turns on AD module
}

int adc_read(int ch)
{
    ADCON1bits.ADFM = 1;
    ADCON0bits.CHS = ch;
    
    ADCON0bits.GO_nDONE = 1; // Starts analog to digital conversion
    for(;;)
        if(!ADCON0bits.GO_nDONE) // until the conversion is done
            break;
    
    // adresh - a/d result high register; adresl: a/d result low register
    return ((ADRESH<<8) + ADRESL); // return right justified 10bit result
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

char* usart_read_string()
{
    char* text;
    int i=0;
    
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
    for(;;)
        if(TXIF) // checks if TXREG is the buffer is empty (1) or full (0)
            break;

    TXREG=c;
}

void usart_write_string(char *text)
{
    int i;
    
    for(i=0; text[i]!='\0'; i++)
        usart_write_char(text[i]);   
}

void debounce(int port)
{
    if(port==3)
    {
        while(!PORTBbits.RB3)
            delay(1000);
    }
    else if(port==4)
    {
        while(!PORTBbits.RB4)
            delay(1000);
    }
    else if(port==5)
    {
        while(!PORTBbits.RB5)
            delay(1000);
    }
    
}

int solar_tracker(int ldr_1, int ldr_2, int temperature, int state)
{
    int max_temp_reached = 0;
    max_temp_reached = check_temperature(temperature);
    
    if(ldr_1 < 20 && ldr_2 < 20)
    {
        stopMotor();
        usart_write_string("\n<Aviso>\n	<Mensagem>Motor a parar.</Mensagem>\n</Aviso>");
    }
    else if(max_temp_reached) 
    {
        // doesnt do anything - as supposed
    }
    else if(ldr_1 - ldr_2 > 100)
    {
        moveLeft();
        usart_write_string("\n<Aviso>\n	<Mensagem>Motor a rodar para a esquerda.</Mensagem>\n</Aviso>");
        delay(2000);
        
        while(ldr_1 - ldr_2 > 100 && state==1)
        {
            ldr_1 = adc_read(0);
            ldr_2 = adc_read(1);
            temperature = adc_read(2)/2;
            max_temp_reached = check_temperature(temperature);
            print_aqc1_status(ldr_1,ldr_2,temperature,state);
            
            if(!PORTBbits.RB3)
            {
                debounce(3);
                
                state = changeMode(state);
                stopMotor();
                break;
            }
            else if(!PORTBbits.RB4)
            {
                debounce(4);
                change_heater_state();
            }
            else if(!PORTBbits.RB5)
            {
                debounce(5);
                change_cooler_state();
            }
            else if(max_temp_reached)
            {
                usart_write_string("\n<Aviso>\n	<Mensagem>Temperatura acima do recomendado. Motor a parar.</Mensagem>\n</Aviso>");
                stopMotor();
                break;
            }
        }
        if(!max_temp_reached)
            usart_write_string("\n<Aviso>\n	<Mensagem>Seguidor solar bem posicionado. Motor a parar.</Mensagem>\n</Aviso>");
    }
    else if(ldr_2 - ldr_1 > 100)
    {
        moveRight();
        usart_write_string("\n<Aviso>\n	<Mensagem>Motor a rodar para a direita.</Mensagem>\n</Aviso>");
        delay(2000);
        
        while(ldr_2 - ldr_1 > 100 && state==1)
        {
            ldr_1 = adc_read(0);
            ldr_2 = adc_read(1);
            temperature = adc_read(2)/2;
            max_temp_reached = check_temperature(temperature);
            print_aqc1_status(ldr_1,ldr_2,temperature,state);
            
            if(!PORTBbits.RB3)
            {
                debounce(3);
                
                state = changeMode(state);
                stopMotor();
                break;
            }
            else if(!PORTBbits.RB4)
            {
                debounce(4);
                change_heater_state();
            }
            else if(!PORTBbits.RB5)
            {
                debounce(5);
                change_cooler_state();
            }
            else if(max_temp_reached)
            {
                usart_write_string("\n<Aviso>\n	<Mensagem>Temperatura acima do recomendado. Motor a parar.</Mensagem>\n</Aviso>");
                stopMotor();
                break;
            }
        }
        if(!max_temp_reached)
            usart_write_string("\n<Aviso>\n	<Mensagem>Seguidor solar bem posicionado. Motor a parar.</Mensagem>\n</Aviso>");
    }
    else
    {
        stopMotor();
        delay(2000);
    }
    return state;
}

void print_aqc1_status(int ldr_1, int ldr_2, int temperature, int state)
{
    char str[50];
	sprintf(str,"\n<AQC1>\n	<ldr1> %d </ldr1>\n	<ldr2> %d </ldr2>",ldr_1,ldr_2);
    usart_write_string(str);
    sprintf(str,"\n	<temperature> %d </temperature>",temperature);
    usart_write_string(str);
    sprintf(str,"\n	<state> %d </state>\n</AQC1>",state);
    usart_write_string(str);
}

int check_temperature(int temperature)
{
    if(temperature>MAX_TEMPERATURE)
        return 1;
    return 0;
}

void change_heater_state()
{
    PORTCbits.RC5 = !PORTCbits.RC5; // invert the heater state 
}

void change_cooler_state()
{
    PORTCbits.RC2 = !PORTCbits.RC2; 
}

int main(void)
{
    int state = 0; // 0 stands for standby mode and 1 for normal 
    int ldr_1, ldr_2, temperature;
    ldr_1 = ldr_2 = temperature = 0;
    configPorts();
    usart_init();
    adc_init();
    
    while(1)
    {
        delay(20);
        if(!PORTBbits.RB3)
        {
            debounce(3);
            state = changeMode(state);
        }
        else if(!PORTBbits.RB4)
        {
            debounce(4);
            change_heater_state();
        }
        else if(!PORTBbits.RB5)
        {
            debounce(5);
            change_cooler_state();
        }
        else if(state)
        {
            ldr_1 = adc_read(0);
            ldr_2 = adc_read(1);
            temperature = adc_read(2)/2;
            print_aqc1_status(ldr_1,ldr_2,temperature,state);
            state = solar_tracker(ldr_1,ldr_2,temperature,state);
        }
    }
}

