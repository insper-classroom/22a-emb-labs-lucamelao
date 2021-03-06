/**
 * 5 semestre - Eng. da Computação - Insper
 * Rafael Corsi - rafael.corsi@insper.edu.br
 *
 * Projeto 0 para a placa SAME70-XPLD
 *
 * Objetivo :
 *  - Introduzir ASF e HAL
 *  - Configuracao de clock
 *  - Configuracao pino In/Out
 *
 * Material :
 *  - Kit: ATMEL SAME70-XPLD - ARM CORTEX M7
 */

/************************************************************************/
/* includes                                                             */
/************************************************************************/
#include "asf.h"
/************************************************************************/
/* defines                                                              */
/************************************************************************/

// LED DA PLACA
#define LED_PIO           PIOC                 // periferico que controla o LED
#define LED_PIO_ID        ID_PIOC                  // ID do periférico PIOC (controla LED)
#define LED_PIO_IDX       8                    // ID do LED no PIO
#define LED_PIO_IDX_MASK  (1 << LED_PIO_IDX)   // Mascara para CONTROLARMOS o LED

// Configuracoes do botao DA PLACA
#define BUT_PIO			PIOA
#define BUT_PIO_ID		ID_PIOA
#define BUT_PIO_IDX		11
#define BUT_PIO_IDX_MASK (1u << BUT_PIO_IDX) // esse já está pronto.

// Configuracoes dos LEDS
#define LED_PIO1           PIOA                 // periferico que controla o LED
#define LED_PIO_ID1        ID_PIOA                  // ID do periférico PIOC (controla LED)
#define LED_PIO_IDX1       0                    // ID do LED no PIO
#define LED_PIO_IDX_MASK1  (1 << LED_PIO_IDX1)   // Mascara para CONTROLARMOS o 

#define LED_PIO2           PIOC                
#define LED_PIO_ID2        ID_PIOC                  
#define LED_PIO_IDX2       30                    
#define LED_PIO_IDX_MASK2  (1 << LED_PIO_IDX2)   

#define LED_PIO3           PIOB                 
#define LED_PIO_ID3        ID_PIOB                 
#define LED_PIO_IDX3       2                    
#define LED_PIO_IDX_MASK3  (1 << LED_PIO_IDX3)   

// Configuracoes dos buttoms

#define BUT_PIO1			PIOD
#define BUT_PIO_ID1			ID_PIOD
#define BUT_PIO_IDX1		28
#define BUT_PIO_IDX_MASK1	(1u << BUT_PIO_IDX1)

#define BUT_PIO2			PIOC
#define BUT_PIO_ID2			ID_PIOC
#define BUT_PIO_IDX2		31
#define BUT_PIO_IDX_MASK2	(1u << BUT_PIO_IDX2) 

#define BUT_PIO3			PIOA
#define BUT_PIO_ID3			ID_PIOA
#define BUT_PIO_IDX3		19
#define BUT_PIO_IDX_MASK3	(1u << BUT_PIO_IDX3) 

// LAB2
/*  Default pin configuration (no attribute). */
#define _PIO_DEFAULT             (0u << 0)
/*  The internal pin pull-up is active. */
#define _PIO_PULLUP              (1u << 0)
/*  The internal glitch filter is active. */
#define _PIO_DEGLITCH            (1u << 1)
/*  The internal debouncing filter is active. */
#define _PIO_DEBOUNCE            (1u << 3)

/************************************************************************/
/* constants                                                            */
/************************************************************************/

/************************************************************************/
/* variaveis globais                                                    */
/************************************************************************/

/************************************************************************/
/* prototypes                                                           */
/************************************************************************/
void init(void);
void _pio_set(Pio *p_pio, const uint32_t ul_mask);
void _pio_clear(Pio *p_pio, const uint32_t ul_mask);
void _pio_pull_up(Pio *p_pio, const uint32_t ul_mask, const uint32_t ul_pull_up_enable);
void _pio_set_input(Pio *p_pio, const uint32_t ul_mask, const uint32_t ul_attribute);
void _pio_set_output(Pio *p_pio, const uint32_t ul_mask, const uint32_t ul_default_level, const uint32_t ul_multidrive_enable, const uint32_t ul_pull_up_enable);
uint32_t _pio_get(Pio *p_pio, const pio_type_t ul_type, const uint32_t ul_mask);
void _delay_ms(uint32_t time);
/************************************************************************/
/* interrupcoes                                                         */
/************************************************************************/

/************************************************************************/
/* Functions LAB 2 */
void _pio_set(Pio *p_pio, const uint32_t ul_mask)
{
	p_pio->PIO_SODR = ul_mask;
}

void _pio_clear(Pio *p_pio, const uint32_t ul_mask)
{
	p_pio->PIO_CODR = ul_mask;
}
void _pio_pull_up(Pio *p_pio, const uint32_t ul_mask, const uint32_t ul_pull_up_enable)
{
	if (ul_pull_up_enable){
		p_pio->PIO_PUER = ul_mask;
	}else{
	p_pio->PIO_PUDR = ul_mask;
	}
}                                    

void _pio_set_input(Pio *p_pio, const uint32_t ul_mask, const uint32_t ul_attribute)
{
	_pio_pull_up(p_pio, ul_mask, (ul_attribute & _PIO_PULLUP));
	
	if (ul_attribute & (_PIO_DEGLITCH | _PIO_DEBOUNCE))
	{
		p_pio ->PIO_IFER = ul_mask;
	}else{
		p_pio ->PIO_IFDR = ul_mask;
	}
}

void _pio_set_output(Pio *p_pio, const uint32_t ul_mask, const uint32_t ul_default_level, const uint32_t ul_multidrive_enable, const uint32_t ul_pull_up_enable)
{
	p_pio -> PIO_PER = ul_mask; //write-only
	p_pio -> PIO_OER = ul_mask; //write-only
	
	_pio_clear(p_pio, ul_default_level); //clear
	p_pio -> PIO_MDER = ul_multidrive_enable; // ativação do multidrive
	_pio_pull_up(p_pio, ul_mask, ul_pull_up_enable); // ativação do pull-up
}

uint32_t _pio_get(Pio *p_pio, const pio_type_t ul_type, const uint32_t ul_mask)
{
	{
		uint32_t estado;

		if (ul_type == PIO_OUTPUT_0) {
			estado = p_pio->PIO_ODSR;
			} else {
			estado = p_pio->PIO_PDSR;
		}

		if ((estado & ul_mask) == 0) {
			return 0;
			} else {
			return 1;
		}
	}	
}

void _delay_ms(uint32_t time)
{
	uint32_t timer = 0;
	while(timer < time*150000){
		asm("nop");
		timer++;
	}	
}

/************************************************************************/

// Função de inicialização do uC
void init(void){
	
	// Initialize the board clock
	sysclk_init();
	
	// Inicializa PIO do botao
	pmc_enable_periph_clk(BUT_PIO_ID);
	pmc_enable_periph_clk(BUT_PIO_ID1);
	pmc_enable_periph_clk(BUT_PIO_ID2);
	pmc_enable_periph_clk(BUT_PIO_ID3);

	// configura pino ligado ao botão como entrada com um pull-up.
	_pio_set_input(PIOA, BUT_PIO_IDX_MASK, _PIO_PULLUP | _PIO_DEBOUNCE);	
	_pio_set_input(PIOA, BUT_PIO_IDX_MASK1, _PIO_PULLUP | _PIO_DEBOUNCE);
	_pio_set_input(PIOC, BUT_PIO_IDX_MASK2, _PIO_PULLUP | _PIO_DEBOUNCE);
	_pio_set_input(PIOB, BUT_PIO_IDX_MASK3, _PIO_PULLUP | _PIO_DEBOUNCE);

	// Desativa WatchDog Timer
	WDT->WDT_MR = WDT_MR_WDDIS;
	
	// Ativa o PIO na qual o LED foi conectado
	// para que possamos controlar o LED.
	pmc_enable_periph_clk(LED_PIO_ID);
	pmc_enable_periph_clk(LED_PIO_ID1);
	pmc_enable_periph_clk(LED_PIO_ID2);
	pmc_enable_periph_clk(LED_PIO_ID3);

	//Inicializa PC8 como saída
	_pio_set_output(LED_PIO, LED_PIO_IDX_MASK, 0, 0, 0);
	_pio_set_output(LED_PIO1, LED_PIO_IDX_MASK1, 0, 0, 0);
	_pio_set_output(LED_PIO2, LED_PIO_IDX_MASK2, 0, 0, 0);
	_pio_set_output(LED_PIO3, LED_PIO_IDX_MASK3, 0, 0, 0);

}

/************************************************************************/
/* Main                                                                 */
/************************************************************************/

// Funcao principal chamada na inicalizacao do uC.
int main(void)
{
  init();
  // super loop
  // aplicacoes embarcadas não devem sair do while(1).
  while (1)
  {  
	 if(!_pio_get(BUT_PIO, PIO_INPUT, BUT_PIO_IDX_MASK)){
		  for (int i = 0; i<10; i++){
			  _pio_clear(PIOC, LED_PIO_IDX_MASK);
			  _delay_ms(100);
			  _pio_set(PIOC, LED_PIO_IDX_MASK);
			  _delay_ms(100);
		  }
		  } else {
		  _pio_set(LED_PIO, LED_PIO_IDX_MASK);
	  }  
	  if(!_pio_get(BUT_PIO1, PIO_INPUT, BUT_PIO_IDX_MASK1)){
		  for (int i = 0; i<10; i++){
			  _pio_clear(PIOA, LED_PIO_IDX_MASK1);
			  _delay_ms(100);  
			  _pio_set(PIOA, LED_PIO_IDX_MASK1);
			  _delay_ms(100);
			}
	  } else {
		  _pio_set(LED_PIO1, LED_PIO_IDX_MASK1);
	  }
	  
	  
	   if(!_pio_get(BUT_PIO2, PIO_INPUT, BUT_PIO_IDX_MASK2)){
		   for (int i = 0; i<10; i++){
			   _pio_clear(PIOC, LED_PIO_IDX_MASK2);
			   _delay_ms(100);
			   _pio_set(PIOC, LED_PIO_IDX_MASK2);
			   _delay_ms(100);
		   }
		   } else {
		   _pio_set(LED_PIO2, LED_PIO_IDX_MASK2);
	   }
	   
	   
	    if(!_pio_get(BUT_PIO3, PIO_INPUT, BUT_PIO_IDX_MASK3)){
		    for (int i = 0; i<10; i++){
			    _pio_clear(PIOB, LED_PIO_IDX_MASK3);
			    _delay_ms(100);
			    _pio_set(PIOB, LED_PIO_IDX_MASK3);
			    _delay_ms(100);
		    }
		    } else {
		    _pio_set(LED_PIO3, LED_PIO_IDX_MASK3);
	    }
	 }
  return 0;
}
