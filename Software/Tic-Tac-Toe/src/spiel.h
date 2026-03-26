#include <stdint.h>
#include <board.h>
#include <radio.h>
/* === macros ============================================================== */
#define COL_PADS    (_BV(PD5) | _BV(PD6) | _BV(PD7))
#define COL_LEDS    (_BV(PB0) | _BV(PB1) | _BV(PB2))
#define ROW_IO_MASK (_BV(PB3) | _BV(PB4) | _BV(PB5))

#define OFF (0)
#define GREEN (1)
#define RED (2)
#define NB_FIELDS (9)
#define DISPLAY_RED (0)
#define DISPLAY_GREEN (3)

#define KEY_NONE (255)

#define PAYLD_SIZE        (116)
#define CRC_SIZE          (sizeof(crc_t))
#define TRX_FRAME_SIZE    (sizeof(xxo_frame_t))
#define PAYLD_START       (sizeof(prot_header_t))
#define CHANNEL 17
#define SHORTADDR 0x4214
#define PANID 0xF00F
#define FRAME_CTRL_FIELD (0x8861)

/*command codes in frames */
#define REQUEST_GAME (16)
#define CONFIRM_GAME (17)
#define CANCEL_GAME  (18)

/* === types =============================================================== */

typedef struct
{
    /* 802.15.4 MAC Header */
    uint16_t fcf;   /**< Frame control field*/
    uint8_t  seq;   /**< Sequence number */
    uint16_t panid; /**< 16 bit PAN ID */
    uint16_t dst;   /**< 16 bit destination address */
    uint16_t src;   /**< 16 bit source address */

    /* Payload */
    uint8_t event;

    /* 802.15.4 MAC Footer */
    uint16_t crc_t;

} xxo_frame_t;

typedef uint16_t crc_t;

typedef enum
{
    NO_ERROR = 0,
    NO_PEER_RESPONSE,
    TIMEOUT,
    GAME_CANCELED
} error_t;

/* === prototypes ========================================================== */
#ifdef __cplusplus
extern "C" {
#endif

    /* captocuh.c / restouch.c */
    uint8_t update_pads(uint8_t row);

    /* leds.c */
    void led_cls(void);
    void led_set(uint8_t idx, uint8_t color);
    uint8_t led_get(uint8_t idx);
    uint8_t led_check_winner(uint8_t mycolor);

    void led_flash(uint8_t n, uint8_t idx, uint8_t color);
    void led_flood_fill(uint8_t color);
    void led_start_game(uint8_t color);

    void display_leds(uint8_t row);

    /* funk.c */
    void xxo_radio_init(node_config_t *ncfg);
    void xxo_send(uint8_t key, node_config_t *ncfg);
    uint8_t xxo_radio_get_event(void);
    void xxo_turn_off_radio(void);

    /* misc.c */
    void xxo_io_init(void);
    void xxo_set_timeout(uint16_t tmo);

    uint8_t xxo_get_key(void);

    void xxo_deep_sleep(void);
    void xxo_sleep(int16_t sleep_time);
    uint8_t xxo_is_power_on_reset(void);

    void xxo_set_error(error_t error);
    error_t xxo_get_error(void);


#ifdef __cplusplus
} /* extern "C" */
#endif
