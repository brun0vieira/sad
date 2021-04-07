// Authors: Bruno Vieira, Francisco Róis and Nikita Dyskin 49541


#include <xc.h>
#pragma config FOSC = HS // Oscillator Selection bits (HS oscillator)
#pragma config WDTE = OFF // Watchdog Timer Enable bit
#pragma config PWRTE = OFF // Power-up Timer Enable bit
#pragma config BOREN = OFF // Brown-out Reset Enable bit
#pragma config LVP = OFF // Low-Voltage (Single-Supply) In-Circuit Serial Programming Enable bit
#pragma config CPD = OFF // Data EEPROM Memory Code Protection bit
#pragma config WRT = OFF // Flash Program Memory Write Enable bits
#pragma config CP = OFF // Flash Program Memory Code Protection bit

int main(void)
{
    TRISBbits.TRISB3 = 1;
    TRISDbits.TRISD0 = 0;
    PORTDbits.RD0 = 0;
    int i = 0;
    while (1){
        if (!PORTBbits.RB3){
            PORTDbits.RD0 = 1;
            for(i = 0 ; i < 20000 ; i++){};
            PORTDbits.RD0 = 0;
            for(i = 0 ; i < 20000 ; i++){};
        }
    }
}