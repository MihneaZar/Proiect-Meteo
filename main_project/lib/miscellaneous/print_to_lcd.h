#ifndef PRINT_TO_LCD_H
#define PRINT_TO_LCD_H

#include <stdio.h>
#include "ssd1306.h"

void clear_lcd_line(int line);
void value_to_string(char *string, int value, char append);
void runtime_to_string(char *string);
void print_time_to_lcd();
void print_options_to_lcd(char options[4][18], int option, int max_option);
void print_runtime_to_lcd(char command);
void print_rainchance();
void print_weather(char temp_format);

#endif // PRINT_TO_LCD_H

