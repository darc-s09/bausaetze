/* === includes ============================================================ */
#include <avr/wdt.h>
#include "spiel.h"

/* === macros ============================================================== */

/* after 30s inactivity we fell into sleep */
#define TIME_OUT (15 * 1000)
/* === types =============================================================== */
typedef enum
{
    DEEP_SLEEP,
    IDLE,
    WAIT_MY_KEY,
    WAIT_PEER_KEY,
    ERROR,
    PREPARE_SLEEP
} game_state_t;

/* === globals ============================================================= */

/* === functions =========================================================== */
int main(void)
{
    node_config_t ncfg;
    game_state_t state;
    uint8_t mycolor, peercolor, local_key, radio_event, mylastkey, winner_color;
    error_t err;

    state = IDLE;
    local_key = KEY_NONE;
    radio_event = KEY_NONE;

    mycolor = OFF;
    peercolor = OFF;
    mylastkey = NB_FIELDS;

    xxo_io_init();
    xxo_radio_init(&ncfg);
    led_cls();

    if (xxo_is_power_on_reset() != 0)
    {
        /*
         * Geraet wurde eingeschaltet: sofort schlafen legen.
         */
        state = PREPARE_SLEEP;
    }
    else
    {
        /* lösche Tasteneingaben */
        xxo_radio_get_event();
        xxo_get_key();
        led_flash(1, 2, GREEN);
        led_flash(1, 6, RED);
        xxo_set_timeout(TIME_OUT);
    }

    while(1)
    {
        local_key = xxo_get_key();
        radio_event = xxo_radio_get_event();
        err = xxo_get_error();        

        if(err != NO_ERROR && state != ERROR)
        {
            state = ERROR;
        }

        switch (state)
        {
            case IDLE:
                if(local_key == 4)
                {
                   xxo_send(REQUEST_GAME, &ncfg);
                }
                if((radio_event == REQUEST_GAME) && (mycolor == OFF))
                {
                    mycolor = GREEN;
                    peercolor = RED;
                    xxo_send(CONFIRM_GAME, &ncfg);
                    led_start_game(mycolor);
                    state = WAIT_MY_KEY;
                }
                else if (radio_event == CONFIRM_GAME)
                {
                    xxo_set_timeout(TIME_OUT);
                    mycolor = RED;
                    peercolor = GREEN;
                    led_start_game(mycolor);
                    state = WAIT_PEER_KEY;
                }
                break;


            case WAIT_MY_KEY:
                if (local_key != KEY_NONE)
                {
                    xxo_set_timeout(TIME_OUT);
                    if (OFF == led_get(local_key))
                    {
                        mylastkey = local_key;
                        led_set(local_key, mycolor);
                        xxo_send(local_key, &ncfg);
                        state = WAIT_PEER_KEY;
                    }
                }
                break;

            case WAIT_PEER_KEY:
                if (radio_event < NB_FIELDS)
                {
                    xxo_set_timeout(TIME_OUT);
                    led_set(radio_event, peercolor);
                    state = WAIT_MY_KEY;
                }
                break;

            case ERROR:  
                xxo_send(CANCEL_GAME, &ncfg);
                xxo_set_timeout(TIME_OUT);
                led_cls();
                led_flash(3, err, RED);
                mycolor = OFF;
                peercolor = OFF;
                mylastkey = NB_FIELDS;
                state = PREPARE_SLEEP;
                break;

            case DEEP_SLEEP:
            case PREPARE_SLEEP:
                /* pass */
                break;

        }

        switch (state)
        {
            case WAIT_MY_KEY:
            case WAIT_PEER_KEY:
                winner_color = led_check_winner(mycolor);
                if (OFF != winner_color)
                {
                    xxo_sleep(1500);
                    state = PREPARE_SLEEP;
                }
                break;

            case PREPARE_SLEEP:
                    led_cls();
                    mycolor = OFF;
                    peercolor = OFF;
                    mylastkey = NB_FIELDS;
                    xxo_turn_off_radio();
                    state = DEEP_SLEEP;
                    xxo_deep_sleep();
                break;

            case IDLE:
                if (mylastkey < NB_FIELDS && state == WAIT_MY_KEY)
                {
                    led_flash(4, mylastkey, mycolor);
                    led_set(mylastkey, mycolor);
                }
                break;

            case ERROR:
            case DEEP_SLEEP:
                /* pass */
                break;
        }
        sleep_mode();
    }
    return 0;
}
