#include <asf.h>
#include "gfx_mono_ug_2832hsweg04.h"
#include "gfx_mono_text.h"
#include "sysfont.h"
#include "defines.h"
#include "utils.h"

/************************************************************************/
/* PROTOTYPES															*/
/************************************************************************/
void io_init(void);
void limpa_oled();
void but_callback(void);
void echo_callback(void);
void set_alarm_but1();
static uint32_t get_time_rtt();

/************************************************************************/
/* VAR globais                                                          */
/************************************************************************/

// flags
volatile char flag_rtc_alarm = 0;
volatile char but_flag = 0;
volatile char count_flag = 0;
volatile char echo_flag;
volatile char rtt_start;

int frequencia_RTT = 1/(2*0.000058);
volatile uint32_t rtt_time = 0;

#define VELOCIDADE_SOM 340.0
#define X_MAXIMO 128
#define Y_MAXIMO 20.0
/************************************************************************/
/* callbacks                                                          */
/************************************************************************/

void echo_callback(void)
{
	if (pio_get(ECHO_PIO, PIO_INPUT, ECHO_IDX_MASK)) {
		// RTT funciona apenas como um rel�gio
		RTT_init(frequencia_RTT, 0, 0);
		} else {
		echo_flag = 1;
		rtt_time = get_time_rtt();
	}
}

/************************************************************************/
/* FUNCTIONS                                                          */
/************************************************************************/
void limpa_oled() {
	gfx_mono_draw_string("             ", 0, 5, &sysfont);
	gfx_mono_draw_string("             ", 0, 16, &sysfont);
}

void onda() {
	// Gera o pulso no pino de Trig com delay_us
	pio_set(TRIG_PIO, TRIG_IDX_MASK);
	delay_us(10);
	pio_clear(TRIG_PIO, TRIG_IDX_MASK);
}

void pin_toggle(Pio *pio, uint32_t mask) {
	if(pio_get_output_data_status(pio, mask))
	pio_clear(pio, mask);
	else
	pio_set(pio,mask);
}

static uint32_t get_time_rtt() {
	// Consulta o valor atual do RTT
	return rtt_read_timer_value(RTT);
}

/************************************************************************/
/* HANDLERS                                                          */
/************************************************************************/

void RTT_Handler(void) {
	uint32_t ul_status;
	ul_status = rtt_get_status(RTT);
	if ((ul_status & RTT_SR_RTTINC) == RTT_SR_RTTINC) {
	}
}

void but_callBack(void){
	but_flag = 1;
}

void io_init(void)
{	
	config_button(BUT_PIO1, BUT_IDX_MASK1, BUT_PIO_ID1, but_callBack, 1, 1);

	// Configura Trig como output
	pmc_enable_periph_clk(TRIG_PIO_ID);
	pio_configure(TRIG_PIO, PIO_OUTPUT_1, TRIG_IDX_MASK, PIO_PULLUP);
	
	// Configura Echo como input
	config_button(ECHO_PIO, ECHO_IDX_MASK, ECHO_PIO_ID, echo_callback, 0, 0);
}

/* MAIN */
int main (void)
{
	board_init();
	sysclk_init();
	gfx_mono_ssd1306_init();
	io_init();
	
	int scan = 0;	
	char str[10];
	float distancia = 0.0;
	
	while(1) {
		if (but_flag) {
			onda(); // gera o pulso
			but_flag = 0;
		}
		   
		if (echo_flag) {
			// C�lculo da dist�ncia usando o valor do RTT
			// Dist�ncia = [Tempo ECHO em n�vel alto * Velocidade do Som]/2
			distancia = rtt_time * VELOCIDADE_SOM * 100 / (frequencia_RTT*2);
			
			// Apaga o que tiver no OLED
			limpa_oled();
			
			// Confere se está no range de leitura
			if (distancia > 2.0 && distancia < 400.0) {
				sprintf(str, "%0.1f cm", distancia);
				gfx_mono_draw_string(str, 0, 0, &sysfont);
				// Gráfico
				if (scan == 0) {
					for (int i = 0; i < X_MAXIMO; i++){
					gfx_mono_draw_rect(0, 16, i, 16, GFX_PIXEL_CLR);
					}
				}
				gfx_mono_draw_rect(scan*8, 16*(2 - distancia/400), 8, 16*(distancia/400), GFX_PIXEL_SET);
				scan = (scan + 1)%16;
			} 
			else {
				gfx_mono_draw_string("Range Error", 0, 0, &sysfont);
			}
		echo_flag = 0;
		}
		pmc_sleep(SAM_PM_SMODE_SLEEP_WFI);
	}
}
