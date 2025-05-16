#include <avr/io.h>
#include <avr/interrupt.h>
#include <timer.h>

/* Allocates the system ticks counter (milliseconds since boot). */
volatile uint32_t systicks = 0;
uint32_t start_time = 0;
uint32_t time_format = 24;

/* Clock correction */
#define CLOCK_SKEW 86400
uint32_t last_correction = 0;

void time_to_string(char* time_string) {
    char twelve_hour;
    uint32_t current_time = systicks / 1000 + start_time; 
    uint8_t hours = current_time / 3600 % 24;
    if (time_format == 12) {
        if (hours < 12) {
            twelve_hour = 'A';
        } else {
            twelve_hour = 'P';
            hours = hours - 12;
        }
    }
    uint8_t minutes = current_time % 3600 / 60;
    uint8_t seconds = current_time % 60;
    time_string[0] = (char) ('0' + hours / 10);
    time_string[1] = (char) ('0' + hours % 10);
    time_string[2] = ':';
    time_string[3] = (char) ('0' + minutes / 10);
    time_string[4] = (char) ('0' + minutes % 10);
    time_string[5] = ':';
    time_string[6] = (char) ('0' + seconds / 10);
    time_string[7] = (char) ('0' + seconds % 10);
    if (time_format == 12) {
        time_string[8] = ' ';
        time_string[9] = twelve_hour;
        time_string[10] = 'M';
        time_string[11] = '\0';
    } else {
        time_string[8] = ' ';
        time_string[9] = ' ';
        time_string[10] = ' ';
        time_string[11] = '\0';
    }
}

void set_start_time(char time_string[9]) {
    uint32_t hour;
    uint32_t minute;
    uint32_t second;

    hour = (time_string[0] - '0') * 10 + (time_string[1] - '0');
    minute = (time_string[3] - '0') * 10 + (time_string[4] - '0');
    second = (time_string[6] - '0') * 10 + (time_string[7] - '0');

    start_time = hour * 3600 + minute * 60 + second;
    systicks = 0;
}

void next_time_digit(char *time_string, int digit_position) {
    int max_digit = 0;
    if (digit_position == 0) {
        max_digit = 2;
    }
    if (digit_position == 1) {
        if (time_string[0] == '2') {
            max_digit = 4;
        } else {
            max_digit = 9;
        }
    }
    if (digit_position == 3 || digit_position == 6) {
        max_digit = 5;   
    }
    if (digit_position == 4 || digit_position == 7) {
        max_digit = 9;   
    }
    int digit_value = time_string[digit_position] - '0';
    digit_value = NEXT_OPTION(digit_value, max_digit);
    time_string[digit_position] = digit_value + '0';
}

void Timer2_init_systicks(void) {
    /* Clear previous settings */
    TCCR2A = 0;
    TCCR2B = 0;

    /* Set CTC mode */
    TCCR2A |= (1 << WGM01);

    /* Set prescaler to 8 */
    TCCR2B |= (1 << CS22);

    /* Activate Compare A interrupt */
    TIMSK2 |= (1 << OCIE2A); 

    /* 16MHz / 8 => 2000 kHz */
    OCR2A = 249;
}

/* interrupt every 1ms */
ISR(TIMER2_COMPA_vect) {
    if (SYSTICKS_PASSED(last_correction, CLOCK_SKEW)) {
        last_correction = systicks;
    } else {   
        systicks++;
    }
}
