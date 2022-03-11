#include <asf.h>

#include "gfx_mono_ug_2832hsweg04.h"
#include "gfx_mono_text.h"
#include "sysfont.h"

/* flag */

volatile char but_flag; 
volatile char but_flag2;
volatile char but_flag3;

/* prototypes */

void pisca_led(int n, int t);
void but_callBack(void);
void but_callBack2(void);
void but_callBack3(void);
void io_init(void);

/* PERIFÉRICOS */

// LEDs
#define LED_PIO				PIOA
#define LED_PIO_ID			ID_PIOA
#define LED_IDX				0
#define LED_IDX_MASK		(1 << LED_IDX)

#define LED_PIO2			PIOC
#define LED_PIO_ID2			ID_PIOC
#define LED_IDX2			30
#define LED_IDX_MASK2		(1 << LED_IDX2)

#define LED_PIO3			PIOB
#define LED_PIO_ID3			ID_PIOB
#define LED_IDX3			2
#define LED_IDX_MASK3		(1 << LED_IDX3)

// Botões
#define BUT_PIO				PIOD
#define BUT_PIO_ID			ID_PIOD
#define BUT_IDX				28
#define BUT_IDX_MASK		(1 << BUT_IDX)

#define BUT_PIO2			PIOC
#define BUT_PIO_ID2			ID_PIOC
#define BUT_IDX2			31
#define BUT_IDX_MASK2		(1u << BUT_IDX2)

#define BUT_PIO3			PIOA
#define BUT_PIO_ID3			ID_PIOA
#define BUT_IDX3			19
#define BUT_IDX_MASK3		(1u << BUT_IDX3)

/* FUNÇÕES */

void pisca_led(int tempo, int freq){
	if(freq>0){
		long T = 500/freq;
		int n = tempo*freq;
		for (int i=0;i<n;i++){
			pio_clear(LED_PIO2, LED_IDX_MASK2);
			delay_ms(T);
			pio_set(LED_PIO2, LED_IDX_MASK2);
			delay_ms(T);
		}
	}
}
void but_callBack(void){
	if (!pio_get(BUT_PIO, PIO_INPUT, BUT_IDX_MASK)){  // na hora que pressiona
			but_flag = 1;
	}else{
		but_flag = 0; // quando tira
	}
}

void but_callBack2(void){
	but_flag2 = 1;
}

void but_callBack3(void){
	but_flag3 = 1;
}

// Inicializa botao SW0 do kit com interrupcao
void io_init(void)
{
	// Init OLED
	gfx_mono_ssd1306_init();
	// Configura led
	pmc_enable_periph_clk(LED_PIO_ID);
	pmc_enable_periph_clk(LED_PIO_ID2);
	pmc_enable_periph_clk(LED_PIO_ID3);
	pio_configure(LED_PIO, PIO_OUTPUT_0, LED_IDX_MASK, PIO_DEFAULT);
	pio_configure(LED_PIO2, PIO_OUTPUT_0, LED_IDX_MASK2, PIO_DEFAULT);
	pio_configure(LED_PIO3, PIO_OUTPUT_0, LED_IDX_MASK3, PIO_DEFAULT);


	// Inicializa clock do periférico PIO responsavel pelo botao
	pmc_enable_periph_clk(BUT_PIO_ID);
	pmc_enable_periph_clk(BUT_PIO_ID2);
	pmc_enable_periph_clk(BUT_PIO_ID3);

	// Configura PIO para lidar com o pino do botão como entrada com pull-up
	pio_configure(BUT_PIO, PIO_INPUT, BUT_IDX_MASK, PIO_PULLUP);
	pio_configure(BUT_PIO2, PIO_INPUT, BUT_IDX_MASK2, PIO_PULLUP);
	pio_configure(BUT_PIO3, PIO_INPUT, BUT_IDX_MASK3, PIO_PULLUP);

	// Configura interrupção no pino referente ao botao e associa função de callback caso uma interrupção for gerada a função de callback é a: but_callback()
	pio_handler_set(BUT_PIO, BUT_PIO_ID, BUT_IDX_MASK, PIO_IT_EDGE, but_callBack);
	pio_handler_set(BUT_PIO2, BUT_PIO_ID2, BUT_IDX_MASK2, PIO_IT_RISE_EDGE, but_callBack2);
	pio_handler_set(BUT_PIO3, BUT_PIO_ID3, BUT_IDX_MASK3, PIO_IT_RISE_EDGE, but_callBack3);


	// Ativa interrupção e limpa primeira IRQ gerada na ativacao
	pio_enable_interrupt(BUT_PIO, BUT_IDX_MASK);
	pio_get_interrupt_status(BUT_PIO);
	
	pio_enable_interrupt(BUT_PIO2, BUT_IDX_MASK2);
	pio_get_interrupt_status(BUT_PIO2);
	
	pio_enable_interrupt(BUT_PIO3, BUT_IDX_MASK3);
	pio_get_interrupt_status(BUT_PIO3);
	
	// Configura NVIC para receber interrupcoes do PIO do botao com prioridade 4 (quanto mais próximo de 0 maior)
	NVIC_EnableIRQ(BUT_PIO_ID);
	NVIC_SetPriority(BUT_PIO_ID, 4);
	
	NVIC_EnableIRQ(BUT_PIO_ID2);
	NVIC_SetPriority(BUT_PIO_ID2, 4); 
	
	NVIC_EnableIRQ(BUT_PIO_ID3);
	NVIC_SetPriority(BUT_PIO_ID3, 4); 
}

int main (void)
{
	io_init();
	board_init();
	sysclk_init();
	delay_init();
	
	int delay = 1;
	
	char str[128];
	sprintf(str, "%d hz", delay);
	gfx_mono_draw_string(str, 50, 16, &sysfont);
	
	/* Insert application code here, after the board has been initialized. */
	while(1) {
		if (but_flag) {
			delay_ms(400);
			if(but_flag){
				delay -= 1; // APERTO LONGO
				}
			else{
				delay += 1;  // APERTO CURTO
				}
			sprintf(str, "%d hz", delay);
			gfx_mono_draw_string(str, 50, 16, &sysfont);
			but_flag = 0;  
			}
		if (but_flag2){
			pisca_led(5, delay);
			but_flag2 = 0;
		}
		if(but_flag3){
			delay -= 1; // APERTO PARA DIMINUIR
			sprintf(str, "%d hz", delay);
			gfx_mono_draw_string(str, 50, 16, &sysfont);
			but_flag3 = 0;
		}
	pmc_sleep(SAM_PM_SMODE_SLEEP_WFI);
	}
}