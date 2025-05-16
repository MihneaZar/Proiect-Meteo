#ifndef TIMERS_H_
#define TIMERS_H_

#include <inttypes.h>

/* 
 * Declares the system ticks counter (milliseconds since boot)
 * for use within other .c modules.
 */
extern volatile uint32_t systicks;
uint32_t time_format;

#define NEXT_OPTION(option, max_option) (option == max_option ? 0 : option + 1)

/* 
 * Useful macro to check whether a specific amount of time
 * passed since some given last event.
 */
#define SYSTICKS_PASSED(last_event, diff_amount) \
	((systicks - (last_event)) >= (diff_amount))

void time_to_string(char* time_string);
void set_start_time(char time_string[9]);
void Set_time();
void next_time_digit(char *time_string, int digit_position);
void Timer2_init_systicks(void);

#endif // TIMERS_H_
