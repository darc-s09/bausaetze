#include <avr/io.h>
#include <avr/interrupt.h>
#include <string.h>
#include "spiel.h"

static uint8_t LedState[9];

static uint8_t WinningPattern[8][3] = {
        {0, 1, 2},
        {3, 4, 5},
        {6, 7, 8},
        {0, 3, 6},
        {1, 4, 7},
        {2, 5, 8},
        {0, 4, 8},
        {2, 4, 6}
};

static void show_winner(uint8_t winner_color, uint8_t mycolor);


void led_cls(void)
{
    memset(LedState, 0, 9);
}

uint8_t led_get(uint8_t idx)
{
    return LedState[idx];
}

void led_set(uint8_t idx, uint8_t color)
{

    if ((idx >= 0) && (idx < 9))
    {
        LedState[idx] = color;
    }
}


void led_flash(uint8_t n, uint8_t idx, uint8_t color)
{

    for(uint8_t i = 0; i < n; i++)
    {
        if (idx < 9)
        {
            LedState[idx] = color;
            xxo_sleep(250);
            LedState[idx] = OFF;
            xxo_sleep(250);
        }
        else
        {
            memset((void*)LedState, color, NB_FIELDS);
            xxo_sleep(250);
            memset((void*)LedState, OFF, NB_FIELDS);
            xxo_sleep(250);
        }
    }
}

void led_flash_pattern(uint8_t n, uint8_t a, uint8_t b, uint8_t c, uint8_t col)
{
    do
    {
        LedState[a] = col;
        LedState[b] = col;
        LedState[c] = col;
        xxo_sleep(250);
        LedState[a] = OFF;
        LedState[b] = OFF;
        LedState[c] = OFF;
        xxo_sleep(250);
    }
    while(n--);
}

void led_flood_fill(uint8_t color)
{

    memset((void*)LedState, 0, NB_FIELDS);
    for(uint8_t i = 0; i < 9; i++)
    {
        LedState[i] = color;
        xxo_sleep(100);
    }
}


uint8_t led_check_winner(uint8_t mycolor)
{
    uint8_t i;
    uint8_t *pwin;
    uint8_t winner_color = OFF;
    for (i = 0; i < 8; i++)
    {
        pwin = WinningPattern[i];
        if ((LedState[pwin[0]] == GREEN) &&
            (LedState[pwin[1]] == GREEN) &&
            (LedState[pwin[2]] == GREEN))
        {
            winner_color = GREEN;


            break;
        }
        if ((LedState[pwin[0]] == RED) &&
            (LedState[pwin[1]] == RED) &&
            (LedState[pwin[2]] == RED))
        {
            winner_color = RED;
            break;
        }
    }

    if (winner_color != OFF)
    {
        led_flash_pattern(5, pwin[0], pwin[1], pwin[2], winner_color);
    }
    else
    {
        for (i = 0; i< 9; i++)
        {
            if (LedState[i] == OFF)
            {
                break;
            }
        }
        if (i == 9)
        {
            winner_color = 4;
        }
    }
    
    if (winner_color != OFF)
    {
        show_winner(winner_color, mycolor);
    }
    return winner_color;
}

void led_start_game(uint8_t color)
{
    uint8_t i, leds[9] = {0,1,2,5,8,7,6,3,4};
    
    for (i=0; i<9; i++)
    {
        if (i > 0)
        {
            led_set(leds[i - 1], OFF);
        }        
        led_set(leds[i], color);
        xxo_sleep(100);        
    }
    xxo_sleep(250);        
    led_set(leds[8], OFF);
}

static void show_winner(uint8_t winner_color, uint8_t mycolor)
{
    uint8_t i;
    if (winner_color != 4)
    {
        led_flood_fill(winner_color);
        led_set(4, mycolor);
    }
    else
    {
        for (i=0; i< 4;i++)
        {
            led_set(0, GREEN);
            led_set(2, GREEN);
            led_set(4, GREEN);
            led_set(6, GREEN);
            led_set(8, GREEN);
            led_set(1, RED);
            led_set(3, RED);
            led_set(5, RED);
            led_set(7, RED);
            xxo_sleep(250);
            led_set(0, RED);
            led_set(2, RED);
            led_set(4, RED);
            led_set(6, RED);
            led_set(8, RED);
            led_set(1, GREEN);
            led_set(3, GREEN);
            led_set(5, GREEN);
            led_set(7, GREEN);
            xxo_sleep(250);
        }
    }
}

void display_leds(uint8_t row)
{
    static uint8_t display_state = DISPLAY_GREEN;
    static uint8_t pat_row[6] = {
                                 _BV(3), /* rot */
                                 _BV(4),
                                 _BV(5),
                                 _BV(4)+_BV(5), /* gruen */
                                 _BV(3)+_BV(5),
                                 _BV(3)+_BV(4)
                                 };

    uint8_t i, rowval, portb, *pled;

    rowval = pat_row[display_state + row];
    portb = rowval;
    pled = &LedState[3*row];

    for(i = 0; i < 3; i++)
    {
        if (display_state == DISPLAY_RED)
        {
            portb |= (*pled == RED) ? 0 : _BV(i);
        }

        else
        {
            portb |= (*pled == GREEN) ? _BV(i) : 0;
        }
        pled ++;
    }

    DDRB = 0;
    PORTB = portb;
    DDRB = (DDRB & ~ROW_IO_MASK) | _BV(row+3) | 7;

    if (row == 2)
    {
        display_state = display_state ? DISPLAY_RED : DISPLAY_GREEN;
    }
}

