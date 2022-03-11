#include <asf.h>

#include "gfx_mono_ug_2832hsweg04.h"
#include "gfx_mono_text.h"
#include "sysfont.h"

/* flag */

volatile char but_flag; 

/* prototypes */

void pisca_led(int n, int t);
void but_callBack(void);
void io_init(void);

/* PERIFÉRICOS */

// LED
#define LED_PIO      PIOA
#define LED_PIO_ID   ID_PIOA
#define LED_IDX      0
#define LED_IDX_MASK (1 << LED_IDX)

// Botão
#define BUT_PIO      PIOD
#define BUT_PIO_ID   ID_PIOD
#define BUT_IDX		 28
#define BUT_IDX_MASK (1 << BUT_IDX)


/* FUNÇÕES */

// pisca led N vez no odo T
void pisca_led(int n, int t){
	for (int i=0;i<n;i++){
		pio_clear(LED_PIO, LED_IDX_MASK);
		delay_ms(t);
		pio_set(LED_PIO, LED_IDX_MASK);
		delay_ms(t);
	}
}

void but_callBack(void){
	if (!pio_get(BUT_PIO, PIO_INPUT, BUT_IDX_MASK)){  // na hora que pressiona
			but_flag = 1;
	}else{
		but_flag = 0; // quando tira
	}
}

// Inicializa botao SW0 do kit com interrupcao
void io_init(void)
{
	// Init OLED
	gfx_mono_ssd1306_init();
	// Configura led
	pmc_enable_periph_clk(LED_PIO_ID);
	pio_configure(LED_PIO, PIO_OUTPUT_0, LED_IDX_MASK, PIO_DEFAULT);

	// Inicializa clock do periférico PIO responsavel pelo botao
	pmc_enable_periph_clk(BUT_PIO_ID);

	// Configura PIO para lidar com o pino do botão como entrada
	// com pull-up
	pio_configure(BUT_PIO, PIO_INPUT, BUT_IDX_MASK, PIO_PULLUP);

	// Configura interrupção no pino referente ao botao e associa
	// função de callback caso uma interrupção for gerada
	// a função de callback é a: but_callback()
	pio_handler_set(BUT_PIO,
	BUT_PIO_ID,
	BUT_IDX_MASK,
	PIO_IT_EDGE,
	but_callBack);

	// Ativa interrupção e limpa primeira IRQ gerada na ativacao
	pio_enable_interrupt(BUT_PIO, BUT_IDX_MASK);
	pio_get_interrupt_status(BUT_PIO);
	
	// Configura NVIC para receber interrupcoes do PIO do botao
	// com prioridade 4 (quanto mais próximo de 0 maior)
	NVIC_EnableIRQ(BUT_PIO_ID);
	NVIC_SetPriority(BUT_PIO_ID, 4); // Prioridade 4
}

int main (void)
{
	io_init();
	board_init();
	sysclk_init();
	delay_init();
	
	int delay = 500;

	/* Insert application code here, after the board has been initialized. */
	while(1) {
		if (but_flag) {
			delay_ms(400);
			if(but_flag){
				delay -= 100; // APERTO LONGO
				}
			else{
				delay += 100;  // APERTO CURTO
				}
			but_flag = 0;  
			}
		
		char str[128]; 
		sprintf(str, "%d ms", delay); 
		gfx_mono_draw_string(str, 50, 16, &sysfont);
	}
}