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

/************************************************************************/
/* interrupcoes                                                         */
/************************************************************************/

/************************************************************************/
/* funcoes                                                              */
/************************************************************************/

// Função de inicialização do uC
void init(void){
	
	// Initialize the board clock
	sysclk_init();
	
	// Inicializa PIO do botao
	pmc_enable_periph_clk(BUT_PIO_ID1);
	pmc_enable_periph_clk(BUT_PIO_ID2);
	pmc_enable_periph_clk(BUT_PIO_ID3);

	
	// configura pino ligado ao botão como entrada com um pull-up.
	pio_set_input(PIOA, BUT_PIO_IDX_MASK1, PIO_DEFAULT);
	pio_pull_up(PIOA, BUT_PIO_IDX_MASK1, PIO_PULLUP);
	
	pio_set_input(PIOC, BUT_PIO_IDX_MASK2, PIO_DEFAULT);
	pio_pull_up(PIOC, BUT_PIO_IDX_MASK2, PIO_PULLUP);
	
	pio_set_input(PIOB, BUT_PIO_IDX_MASK3, PIO_DEFAULT);
	pio_pull_up(PIOB, BUT_PIO_IDX_MASK3, PIO_PULLUP);

	// Desativa WatchDog Timer
	WDT->WDT_MR = WDT_MR_WDDIS;
	
	// Ativa o PIO na qual o LED foi conectado
	// para que possamos controlar o LED.
	pmc_enable_periph_clk(LED_PIO_ID1);
	pmc_enable_periph_clk(LED_PIO_ID2);
	pmc_enable_periph_clk(LED_PIO_ID3);

	
	//Inicializa PC8 como saída
	pio_set_output(LED_PIO1, LED_PIO_IDX_MASK1, 0, 0, 0);
	pio_set_output(LED_PIO2, LED_PIO_IDX_MASK2, 0, 0, 0);
	pio_set_output(LED_PIO3, LED_PIO_IDX_MASK3, 0, 0, 0);

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
	  if(!pio_get(BUT_PIO1, PIO_INPUT, BUT_PIO_IDX_MASK1)){
		  for (int i = 0; i<10; i++){
			  pio_clear(PIOA, LED_PIO_IDX_MASK1);
			  delay_ms(100);  
			  pio_set(PIOA, LED_PIO_IDX_MASK1);
			  delay_ms(100);
			}
	  } else {
		  pio_set(LED_PIO1, LED_PIO_IDX_MASK1);
	  }
	  
	  
	   if(!pio_get(BUT_PIO2, PIO_INPUT, BUT_PIO_IDX_MASK2)){
		   for (int i = 0; i<10; i++){
			   pio_clear(PIOC, LED_PIO_IDX_MASK2);
			   delay_ms(100);
			   pio_set(PIOC, LED_PIO_IDX_MASK2);
			   delay_ms(100);
		   }
		   } else {
		   pio_set(LED_PIO2, LED_PIO_IDX_MASK2);
	   }
	   
	   
	    if(!pio_get(BUT_PIO3, PIO_INPUT, BUT_PIO_IDX_MASK3)){
		    for (int i = 0; i<10; i++){
			    pio_clear(PIOB, LED_PIO_IDX_MASK3);
			    delay_ms(100);
			    pio_set(PIOB, LED_PIO_IDX_MASK3);
			    delay_ms(100);
		    }
		    } else {
		    pio_set(LED_PIO3, LED_PIO_IDX_MASK3);
	    }
	 }
  return 0;
}
