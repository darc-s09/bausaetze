/* === includes ============================================================ */
//#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <util/delay.h>

/* uracoli libraries */
#include <board.h>
#include <transceiver.h>
#include <radio.h>

#include "spiel.h"

/* === macros ============================================================== */
#define KEY_NONE (255)

/* === globals ============================================================= */

static uint8_t RxFrame[MAX_FRAME_SIZE];
static volatile bool TxInProgress;
static volatile uint8_t RxEvent;
static uint16_t DestinationAddress;

/* === functions =========================================================== */

/* === radio init =========== */
void xxo_radio_init(node_config_t *ncfg)
{

    if (0 != get_node_config_eeprom(ncfg, 0))
    {
        /* Standardwerte setzen, fall ungültige Daten im EEPROM */
        ncfg->short_addr = SHORTADDR;
        ncfg->pan_id = PANID;
        ncfg->channel = CHANNEL;
        DestinationAddress = SHORTADDR;
    }
    else
    {
        DestinationAddress = ncfg->short_addr;
        DestinationAddress += (ncfg->short_addr & 1) ? +1 : -1;
    }

    radio_init(RxFrame, sizeof(RxFrame));
    radio_set_param(RP_CHANNEL(ncfg->channel));
    radio_set_param(RP_SHORTADDR(ncfg->short_addr));
    radio_set_param(RP_PANID(ncfg->pan_id));
    radio_set_param(RP_IDLESTATE(STATE_RXAUTO));
    radio_set_param(RP_TXPWR(-4));

    radio_set_state(STATE_RXAUTO);
    RxEvent = KEY_NONE;
}

/* === transmit functions === */
void xxo_send(uint8_t event, node_config_t *ncfg)
{
    static uint8_t seqno = 0;
    xxo_frame_t txbuf;
    /* fill frame information */
    txbuf.fcf = 0x8861;
    txbuf.seq = seqno++;
    txbuf.panid = ncfg->pan_id;
    txbuf.dst = DestinationAddress;
    txbuf.src = ncfg->short_addr;
    radio_set_state(STATE_TXAUTO);

    /* fill payload */
    txbuf.event = event;

    /* send frame */
    TxInProgress = true;

    radio_send_frame(sizeof(xxo_frame_t), (uint8_t*)&txbuf, 0);

    set_sleep_mode(SLEEP_MODE_IDLE);
    while (TxInProgress)
    {
        sleep_mode();
    }
}

void usr_radio_tx_done(radio_tx_done_t status)
{

    TxInProgress = false;
    if (status == TX_NO_ACK)
    {
        xxo_set_error(NO_PEER_RESPONSE);
    }
}

/* === receive functions === */
uint8_t * usr_radio_receive_frame(uint8_t len, uint8_t *frm, uint8_t lqi, int8_t ed, uint8_t crc)
{
    xxo_frame_t *pframe;
    pframe = (xxo_frame_t *)frm;
    if (pframe->event == CANCEL_GAME)
    {
        xxo_set_error(GAME_CANCELED);
    }
    else if (RxEvent == KEY_NONE)
    {
        RxEvent = pframe->event;
    }
    return frm;
}

uint8_t xxo_radio_get_event(void)
{
uint8_t ret;

    cli();
    ret = RxEvent;
    RxEvent = KEY_NONE;
    sei();
    return ret;
}

void xxo_turn_off_radio(void)
{
    radio_set_state(STATE_SLEEP);
}
