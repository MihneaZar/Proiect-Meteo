#include <avr/interrupt.h>
#include <avr/io.h>
#include <stdio.h>

#include "usart.h"
#include "timer.h"
#include "ssd1306.h"
#include "buttons.h"
#include "pff.h"
#include "bme280.h"
#include "print_to_lcd.h"

FATFS fs;  // sistemul de fisiere

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

void SD_Init() {
    uint32_t last_attempt = 0;
    while (1) {
        printf("about to mount\n");
        uint8_t mount = pf_mount(&fs);
        printf("done mounting\n");
        printf("%d\n", mount);
        if (mount == FR_OK) {
            printf("mounting is successful\n");
            break;
        }
        // Waits for a second before retrying
        while(!SYSTICKS_PASSED(last_attempt, 1000));
        last_attempt = systicks;
    }
    pf_open("log.csv");
    pf_lseek(0);
}

void SD_log_data(uint32_t temp_c) {
    /* TODO4 Scrieti temp_c in log.csv */
    WORD w;
    pf_write(&temp_c, 4, &w);
    pf_write(NULL, 0, &w);
}

uint32_t SD_read_data() {
    uint32_t temperature_c;
    WORD w;
    pf_read(&temperature_c, 4, &w);
    return temperature_c;
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
    // SD_Init();
    bme280_init(1);
    Set_time();
}

int main() { 
    init_all();

    DDRB |= (1 << PB5);
    PORTB &= ~(1 << PB5);

    uint32_t runtime_ping = -2001;
    uint32_t rainchance_ping = -2001;
    uint32_t time_ping = -1001;
    uint32_t last_sensor_read = -5001;
    uint8_t show_runtime = 0;
    uint8_t show_rainchance = 0;

    char options[4][18] = {"time   ", "runtime", "temp      ", "rain"};
    uint8_t option = 0;
    uint8_t max_option = 3;
    print_options_to_lcd(options, option, max_option);

    char temp_format = 'C';

    while(1) {
        if (blue_button && !show_runtime && !show_rainchance) {
            blue_button = 0;
            option = NEXT_OPTION(option, max_option);
            print_options_to_lcd(options, option, max_option);
        }
        if (red_button && !show_runtime && !show_rainchance) {
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
            // change temperature format
            if (option == 2) {
                if (temp_format == 'C') {
                    temp_format = 'F';
                } else {
                    temp_format = 'C';
                }
                last_sensor_read = -5001;
            }
            // show rain chance
            if (option == 3) {
                print_rainchance();
                show_rainchance = 1;
                rainchance_ping = systicks;
            }
        }
        if (SYSTICKS_PASSED(last_sensor_read, 5000) && !show_runtime && !show_rainchance) {
            print_weather(temp_format);
            last_sensor_read = systicks;
        }
        if (SYSTICKS_PASSED(runtime_ping, 2000) && show_runtime) {
            print_runtime_to_lcd('c');
            show_runtime = 0;
            print_weather(temp_format);
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
        if (SYSTICKS_PASSED(rainchance_ping, 2000) && show_rainchance) {
            show_rainchance = 0;
            print_weather(temp_format);
            print_options_to_lcd(options, option, max_option);
        }
    }
    return 0;
}