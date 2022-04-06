#ifndef DEFINES_H_   /* Include guard */
#define DEFINES_H_

/* LEDS */
#define LED_PIO				PIOC
#define LED_PIO_ID			ID_PIOC
#define LED_PIO_IDX			8
#define LED_IDX_MASK		(1 << LED_PIO_IDX)

#define LED_PIO1			PIOA
#define LED_PIO_ID1			ID_PIOA
#define LED_IDX1			0
#define LED_IDX_MASK1		(1 << LED_IDX1)

#define LED_PIO2			PIOC
#define LED_PIO_ID2			ID_PIOC
#define LED_IDX2			30
#define LED_IDX_MASK2		(1 << LED_IDX2)

#define LED_PIO3			PIOB
#define LED_PIO_ID3			ID_PIOB
#define LED_IDX3			2
#define LED_IDX_MASK3		(1 << LED_IDX3)

/* BOTAO 1 DO OLED */
#define BUT_PIO1			PIOD
#define BUT_PIO_ID1			ID_PIOD
#define BUT_IDX1			28
#define BUT_IDX_MASK1		(1 << BUT_IDX1)

//ECHO
#define ECHO_PIO PIOD
#define ECHO_PIO_ID	ID_PIOD
#define ECHO_PIO_IDX 30
#define ECHO_IDX_MASK (1 << ECHO_PIO_IDX)

//
#define TRIG_PIO PIOC
#define TRIG_PIO_ID	ID_PIOC
#define TRIG_PIO_IDX 13
#define TRIG_IDX_MASK (1 << TRIG_PIO_IDX)
#endif // DEFINES_H_
