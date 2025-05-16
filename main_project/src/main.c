#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <avr/wdt.h>
#include <avr/io.h>
#include <stdio.h>

#include "usart.h"
#include "timer.h"
#include "ssd1306.h"
#include "buttons.h"

/* No. of seconds before going into standby mode */
#define STANDBY 60

void copy_string_values(volatile char *string, char *copy_string) {
    uint8_t i = 0;
    while (string[i]) {
        copy_string[i] = string[i];
        i++;
    }
    string[i] = '\0';
}

void clear_lcd_line(int line) {
    char *clear = "                  ";
    SSD1306_SetPosition(0, line);
    SSD1306_DrawString(clear);
    SSD1306_UpdateScreen(SSD1306_ADDR);
}

void Set_time() {
    uint8_t digit_position = 0;

    char time_string[12];
    char position_string[9] = "^       ";
    time_to_string(time_string);
    SSD1306_SetPosition(35, 1);
    SSD1306_DrawString(time_string);
    SSD1306_SetPosition(35, 2);
    SSD1306_DrawString(position_string);
    SSD1306_UpdateScreen(SSD1306_ADDR);

    while (digit_position < 8) {
        if (blue_button) {
            blue_button = 0;
            position_string[digit_position] = ' ';
            digit_position++;
            if (time_string[digit_position] == ':') {
                digit_position++;
            }
            position_string[digit_position] = '^';
            SSD1306_SetPosition(35, 2);
            SSD1306_DrawString(position_string);
            SSD1306_UpdateScreen(SSD1306_ADDR);
        }
        if (red_button) {
            red_button = 0;
            next_time_digit(time_string, digit_position);
            SSD1306_SetPosition(35, 1);
            SSD1306_DrawString(time_string);
            SSD1306_UpdateScreen(SSD1306_ADDR);
        }
    }
    SSD1306_ClearScreen();
    SSD1306_UpdateScreen(SSD1306_ADDR);
    set_start_time(time_string);
}

/***
 * Turns off watchdog for less power consumption.
 */
void WDT_off(void) {
    cli();
    wdt_reset();

    /* Clear WDRF in MCUSR */
    MCUSR &= ~(1 << WDRF);

    /* Write logical one to WDCE and WDE */
    /* Keep old prescaler setting to prevent unintentional time-out */
    WDTCSR |= (1 << WDCE) | (1 << WDE);

    /* Turn off WDT */
    WDTCSR = 0x00;
    sei();
}

/***
 * 0 - turn off standby (turn on device)
 * 1 - turn on standby (turn off device)
 * 
 */
void standby_function(int command) {
    if (command == 0) {
        PRR &= ~(1 << PRTWI);
        TWI_ENABLE();
        clear_lcd_line(1);
    }
    if (command == 1) {
        SSD1306_ClearScreen();
        SSD1306_SetPosition(0, 1);
        SSD1306_DrawString("  Standby mode...");
        SSD1306_UpdateScreen(SSD1306_ADDR);
        PRR |= (1 << PRTWI);
    }
}

/***
 * Disables all pull-ups for pins for power-saving.
 * 
 */
void unused_pins() {
    DDRB = 0;
    DDRC = 0;
    DDRD = 0;
    PORTB = 0;
    PORTC = 0;
    PORTD = 0;
}

void init_all() {
    sei();
    unused_pins();
    USART0_init();
    USART0_use_stdio();
    Buttons_init();
    SSD1306_Init(SSD1306_ADDR);
    SSD1306_ClearScreen();
    SSD1306_UpdateScreen(SSD1306_ADDR);
    Timer2_init_systicks();
    Set_time();
}

void print_time_to_lcd() {
    char time_string[12];
    time_to_string(time_string);
    SSD1306_SetPosition(35, 0);
    SSD1306_DrawString(time_string);
    SSD1306_UpdateScreen(SSD1306_ADDR);
}

void print_options_to_lcd(char options[4][18], int option, int max_option) {
    char print_options[50];
    int position = 0;
    for (int i = 0; i <= max_option; i++) {
        if (i == option) {
            print_options[position++] = '>';
        } else {
            print_options[position++] = ' ';
        }
        int option_pos = 0;
        while(options[i][option_pos]) {
            print_options[position++] = options[i][option_pos++];
        }
        print_options[position++] = ' ';
    }
    print_options[position] = '\0';
    SSD1306_SetPosition(0, 2);
    SSD1306_DrawString(print_options);
    SSD1306_UpdateScreen(SSD1306_ADDR);
}

void value_to_string(char *string, int value, char append) {
    int power_of_ten = 1;
    while (value / power_of_ten >= 10) {
        power_of_ten *= 10;
    }
    int position = 0;
    while(power_of_ten > 0) {
        string[position++] = value % (power_of_ten * 10) / power_of_ten + '0';
        power_of_ten /= 10;
    }
    string[position++] = append;
    string[position] = '\0'; 
    // copy_string_values(append, string + position);
}

void runtime_to_string(char *string) {
    uint32_t runtime = systicks / 1000;
    uint8_t seconds = runtime % 60;
    runtime /= 60;
    uint32_t minutes = runtime % 60;
    runtime /= 60;
    uint8_t hours = runtime % 24;
    runtime /= 24;
    uint8_t days = runtime;
    uint8_t already_printed = 0;
    uint8_t position = 0;

    if (days > 0) {
        if (days > 9) {
            string[position++] = days / 10 + '0';
        }
        string[position++] = days % 10 + '0';
        string[position++] = 'd';
        string[position++] = ' ';
        already_printed = 1;
    }
    if (hours > 0 || already_printed) {
        if (hours > 9) {
            string[position++] = hours / 10 + '0';
        }
        string[position++] = hours % 10 + '0';
        string[position++] = 'h';
        string[position++] = ' ';
        already_printed = 1;
    }
    if (minutes > 0 || already_printed) {
        if (minutes > 9) {
            string[position++] = minutes / 10 + '0';
        }
        string[position++] = minutes % 10 + '0';
        string[position++] = 'm';
        string[position++] = ' ';
    }
    if (seconds > 9) {
        string[position++] = seconds / 10 + '0';
    }
    string[position++] = seconds % 10 + '0';
    string[position++] = 's';
    string[position++] = ' ';
    string[position++] = '\0';
}

/***
 * p = print runtime
 * u = update
 * c = clear runtime
 * 
 */
void print_runtime_to_lcd(char command) {
    if (command == 'p') {
        clear_lcd_line(2);
        clear_lcd_line(3);
    }
    if (command == 'p' || command == 'u') {
        char print_runtime[18] = "Current runtime:";
        SSD1306_SetPosition(0, 1);
        SSD1306_DrawString(print_runtime);
        runtime_to_string(print_runtime);
        SSD1306_SetPosition(0, 2);
        SSD1306_DrawString(print_runtime);
        SSD1306_UpdateScreen(SSD1306_ADDR);
    }
    if (command == 'c') {
        clear_lcd_line(1);
        clear_lcd_line(2);
        clear_lcd_line(3);
    }
}

int main() { 
    init_all();

    DDRB |= (1 << PB5);
    PORTB &= ~(1 << PB5);

    uint32_t standby_ping = 0;
    uint8_t standby_mode = 0;
    uint32_t runtime_ping = -2001;
    uint32_t time_ping = -1001;
    uint8_t show_runtime = 0;

    char options[4][18] = {"time   ", "runtime", "temp      ", "rain"};
    uint8_t option = 0;
    uint8_t max_option = 3;
    print_options_to_lcd(options, option, max_option);

    while(1) {
        if (standby_mode) {
            if (blue_button || red_button) {
                standby_mode = 0;
                standby_function(0);
                blue_button = 0;
                red_button = 0;
                print_time_to_lcd();
                print_options_to_lcd(options, option, max_option);
                standby_ping = systicks;
            }
            continue;
        }
        if (blue_button && !show_runtime) {
            blue_button = 0;
            option = NEXT_OPTION(option, max_option);
            print_options_to_lcd(options, option, max_option);
        }
        if (red_button && !show_runtime) {
            red_button = 0;

            // change time format
            if (option == 0) {
                if (time_format == 24) {
                    time_format = 12;
                } else {
                    time_format = 24;
                }  
                print_time_to_lcd();
            }

            // show runtime
            if (option == 1) {
                print_runtime_to_lcd('p');
                runtime_ping = systicks;
                show_runtime = 1;
            }
        }
        if (SYSTICKS_PASSED(standby_ping, (uint16_t) STANDBY * 1000)) {
            standby_mode = 1;
            PORTB &= ~(1 << PB5);
            standby_function(1);
            continue;
        }
        if (SYSTICKS_PASSED(runtime_ping, 2000) && show_runtime) {
            print_runtime_to_lcd('c');
            show_runtime = 0;
            print_options_to_lcd(options, option, max_option);
        }
        if (SYSTICKS_PASSED(time_ping, 1000)) {
            if (!SYSTICKS_PASSED(runtime_ping, 2000)) {
                print_runtime_to_lcd('u');
            }
            print_time_to_lcd();
            time_ping = systicks;
            if (PORTB & (1 << PB5)) {
                PORTB &= ~(1 << PB5);
            } else {
                PORTB |= (1 << PB5);
            }
        }
    }
    return 0;
}