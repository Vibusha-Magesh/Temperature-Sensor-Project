#define _XTAL_FREQ 20000000  // 20 MHz Crystal

#include <xc.h>
#include <stdio.h>

// CONFIG
#pragma config FOSC = HS
#pragma config WDTE = OFF
#pragma config PWRTE = OFF
#pragma config BOREN = ON
#pragma config LVP = OFF
#pragma config CPD = OFF
#pragma config WRT = OFF
#pragma config CP = OFF

// LCD Pins
#define RS RD0
#define EN RD1
#define D4 RD2
#define D5 RD3
#define D6 RD4
#define D7 RD5

// Function Prototypes
void LCD_Init();
void LCD_Command(unsigned char);
void LCD_Char(unsigned char);
void LCD_String(const char *);
void LCD_Set_Cursor(unsigned char, unsigned char);
void ADC_Init();
int ADC_Read(unsigned char);

void main(void) {
    char temp_str[16];
    int adc_value;
    float temperature;
    unsigned char buzzer_triggered = 0;

    TRISA = 0x01;   // RA0 input for LM35
    TRISD = 0x00;   // RD0–RD5 as output for LCD
    TRISC0 = 0;     // RC0 output for buzzer
    RC0 = 0;        // Buzzer initially OFF

    ADC_Init();
    LCD_Init();

    while (1) {
        adc_value = ADC_Read(0);  // Read ADC channel 0
        temperature = adc_value * 4.88 / 10.0;  // Convert ADC to °C

        sprintf(temp_str, "Temp: %.2f C", temperature);

        // First line: status
        LCD_Set_Cursor(1, 1);
        if (temperature > 80.0) {
            LCD_String("WARNING: HOT!   ");
        } else {
            LCD_String("Temperature:    ");
        }

        // Second line: value
        LCD_Set_Cursor(2, 1);
        LCD_String("                ");  // Clear old text
        LCD_Set_Cursor(2, 1);
        LCD_String(temp_str);

        // Buzzer logic
        if (temperature > 80.0 && buzzer_triggered == 0) {
            RC0 = 1;               // Turn buzzer ON
            __delay_ms(5000);      // Wait 5 seconds
            RC0 = 0;               // Turn buzzer OFF
            buzzer_triggered = 1;  // Prevent retriggering
        }

        if (temperature <= 80.0) {
            buzzer_triggered = 0;  // Reset trigger
        }

        __delay_ms(1000);  // Update every second
    }
}

// === LCD Functions ===

void LCD_Command(unsigned char cmd) {
    RS = 0;
    D4 = (cmd >> 4) & 1;
    D5 = (cmd >> 5) & 1;
    D6 = (cmd >> 6) & 1;
    D7 = (cmd >> 7) & 1;
    EN = 1; __delay_ms(1); EN = 0;

    D4 = cmd & 1;
    D5 = (cmd >> 1) & 1;
    D6 = (cmd >> 2) & 1;
    D7 = (cmd >> 3) & 1;
    EN = 1; __delay_ms(1); EN = 0;
    __delay_ms(2);
}

void LCD_Char(unsigned char data) {
    RS = 1;
    D4 = (data >> 4) & 1;
    D5 = (data >> 5) & 1;
    D6 = (data >> 6) & 1;
    D7 = (data >> 7) & 1;
    EN = 1; __delay_ms(1); EN = 0;

    D4 = data & 1;
    D5 = (data >> 1) & 1;
    D6 = (data >> 2) & 1;
    D7 = (data >> 3) & 1;
    EN = 1; __delay_ms(1); EN = 0;
    __delay_ms(2);
}

void LCD_Init() {
    __delay_ms(20);
    LCD_Command(0x02);   // 4-bit mode
    LCD_Command(0x28);   // 2-line, 5x7 font
    LCD_Command(0x0C);   // Display ON, cursor OFF
    LCD_Command(0x06);   // Auto increment cursor
    LCD_Command(0x01);   // Clear display
    __delay_ms(2);
}

void LCD_String(const char *str) {
    while (*str) {
        LCD_Char(*str++);
    }
}

void LCD_Set_Cursor(unsigned char row, unsigned char column) {
    unsigned char pos = (row == 1) ? 0x80 : 0xC0;
    LCD_Command(pos + column - 1);
}

// === ADC Functions ===

void ADC_Init() {
    ADCON0 = 0x41;  // ADC ON, Channel 0
    ADCON1 = 0x80;  // Right-justified, Vref = Vdd
}

int ADC_Read(unsigned char channel) {
    ADCON0 &= 0xC5;
    ADCON0 |= (channel << 3);
    __delay_us(20);
    GO_nDONE = 1;
    while (GO_nDONE);
    return ((ADRESH << 8) + ADRESL);
}
