#include "conf_board.h"
#include <asf.h>

/************************************************************************/
/* BOARD CONFIG                                                          */
/************************************************************************/

#define BUT_PIO           PIOA
#define BUT_PIO_ID        ID_PIOA
#define BUT_PIO_PIN       11
#define BUT_PIO_PIN_MASK  (1 << BUT_PIO_PIN)

// NOVO BOTAO ADICIONADO
#define BUT_PIO1			      PIOD
#define BUT_PIO_ID1			    ID_PIOD
#define BUT_PIO_IDX1			  28
#define BUT_PIO_IDX_MASK1		(1 << BUT_PIO_IDX1)

#define LED_PIO         PIOC
#define LED_PIO_ID      ID_PIOC
#define LED_PIO_IDX     8
#define LED_IDX_MASK    (1 << LED_PIO_IDX)

#define USART_COM_ID ID_USART1
#define USART_COM USART1

/************************************************************************/
/* RTOS                                                                */
/************************************************************************/

#define TASK_LED_STACK_SIZE (4096 / sizeof(portSTACK_TYPE))
#define TASK_LED_STACK_PRIORITY (tskIDLE_PRIORITY)
#define TASK_BUT_STACK_SIZE (4096 / sizeof(portSTACK_TYPE))
#define TASK_BUT_STACK_PRIORITY (tskIDLE_PRIORITY)
extern void vApplicationStackOverflowHook(xTaskHandle *pxTask, signed char *pcTaskName);
extern void vApplicationIdleHook(void);
extern void vApplicationTickHook(void);
extern void vApplicationMallocFailedHook(void);
extern void xPortSysTickHandler(void);

/************************************************************************/
/* recursos RTOS                                                        */
/************************************************************************/

/* Semaforo a ser usado pela task led */
/* ENDERECO GLOBAL*/
SemaphoreHandle_t xSemaphoreBut;
SemaphoreHandle_t xSemaphoreBut1; // NOVO SEMAFORO usado pelo novo BUT1

/** Queue for msg log send data */
QueueHandle_t xQueueLedFreq;
QueueHandle_t xQueueIncremento;

/************************************************************************/
/* prototypes local                                                     */
/************************************************************************/
static void BUT_init(void);
void LED_init(int estado);

void but_callback(void);
void but_callback1(void);

void pin_toggle(Pio *pio, uint32_t mask);
static void USART1_init(void);

int incremento;
/************************************************************************/
/* RTOS application funcs                                               */
/************************************************************************/

/**
 * \brief Called if stack overflow during execution
 */
extern void vApplicationStackOverflowHook(xTaskHandle *pxTask, signed char *pcTaskName) {
  printf("stack overflow %x %s\r\n", pxTask, (portCHAR *)pcTaskName);
  /* If the parameters have been corrupted then inspect pxCurrentTCB to
   * identify which task has overflowed its stack.
   */
  for (;;) {
  }
}

/**
 * \brief This function is called by FreeRTOS idle task - COLOCA NO SLEEP MODE
 */
extern void vApplicationIdleHook(void) { pmc_sleep(SAM_PM_SMODE_SLEEP_WFI); }

/**
 * \brief This function is called by FreeRTOS each tick
 */
extern void vApplicationTickHook(void) {}

extern void vApplicationMallocFailedHook(void) {
  /* Called if a call to pvPortMalloc() fails because there is insufficient
  free memory available in the FreeRTOS heap.  pvPortMalloc() is called
  internally by FreeRTOS API functions that create tasks, queues, software
  timers, and semaphores.  The size of the FreeRTOS heap is set by the
  configTOTAL_HEAP_SIZE configuration constant in FreeRTOSConfig.h. */

  /* Force an assert. */
  configASSERT((volatile void *)NULL);
}

/************************************************************************/
/* handlers / callbacks                                                 */
/************************************************************************/


/* DENTRO DA INTERRUPCAO/CALLBACK LIBERAMOS O SINAL COM xSemaphoreGiveFromISR */
/* CASO A LIBERACAO NAO SEJA DENTRO DE UMA INTERRUPCAO, USAR xSemaphoreGive */

void but_callback(void) {
  incremento = -100;
  BaseType_t xHigherPriorityTaskWoken = pdFALSE;
  xQueueSendFromISR(xQueueIncremento, &incremento, &xHigherPriorityTaskWoken);
  /*DIMINUI FREQ*/
}

void but_callback1(void) {
  incremento = 100;
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;
  xQueueSendFromISR(xQueueIncremento, &incremento, &xHigherPriorityTaskWoken);
  /*AUMENTA FREQ*/
}

/************************************************************************/
/* TASKS                                                                */
/************************************************************************/

static void task_but(void *pvParameters) {

  BUT_init();
  uint32_t delayTicks = 2000;
  incremento = 0;  // inicialmente nada foi apertado

  for (;;) {
    if (xQueueReceive(xQueueIncremento, &incremento, (TickType_t)0)) {
		  printf("Valor do incremento/decremento: %d \n", incremento);
		  delayTicks += incremento;
		  xQueueSend(xQueueLedFreq, (void *)&delayTicks, 10);
     }
  }
}

static void task_led(void *pvParameters) {

  LED_init(1);

  uint32_t msg = 0;
  uint32_t delayMs = 2000;

  /* tarefas de um RTOS n??o devem retornar */
  for (;;) {
    /* verifica se chegou algum dado na queue, e espera por 0 ticks */
    if (xQueueReceive(xQueueLedFreq, &msg, (TickType_t) 0)) {
      /* chegou novo valor, atualiza delay ! */
      /* aqui eu poderia verificar se msg faz sentido (se esta no range certo)*/
      /* converte ms -> ticks */
      delayMs = msg / portTICK_PERIOD_MS;
      printf("Frequencia atual: %d \n", delayMs);
    }

    /* pisca LED */
    pin_toggle(LED_PIO, LED_IDX_MASK);

    /* suspende por delayMs */
    vTaskDelay(delayMs);
  }
}


/************************************************************************/
/* funcoes                                                              */
/************************************************************************/

/**
 * \brief Configure the console UART.
 */
static void configure_console(void) {
  const usart_serial_options_t uart_serial_options = {
      .baudrate = CONF_UART_BAUDRATE,
      .charlength = CONF_UART_CHAR_LENGTH,
      .paritytype = CONF_UART_PARITY,
      .stopbits = CONF_UART_STOP_BITS,
  };

  /* Configure console UART. */
  stdio_serial_init(CONF_UART, &uart_serial_options);

  /* Specify that stdout should not be buffered. */
  setbuf(stdout, NULL);
}

void pin_toggle(Pio *pio, uint32_t mask) {
  if (pio_get_output_data_status(pio, mask))
    pio_clear(pio, mask);
  else
    pio_set(pio, mask);
}

void LED_init(int estado){
	pmc_enable_periph_clk(LED_PIO_ID);
	pio_set_output(LED_PIO, LED_IDX_MASK, estado, 0, 0);
};

static void BUT_init(void) {
	
  // Configura PIO para lidar com o pino do bot??o como entrada com pull-up
  pio_configure(BUT_PIO, PIO_INPUT, BUT_PIO_PIN_MASK, PIO_PULLUP | PIO_DEBOUNCE);
  pio_set_debounce_filter(BUT_PIO, BUT_PIO_PIN_MASK, 60);

  pio_configure(BUT_PIO1, PIO_INPUT, BUT_PIO_IDX_MASK1, PIO_PULLUP | PIO_DEBOUNCE);
  pio_set_debounce_filter(BUT_PIO1, BUT_PIO_IDX_MASK1, 60);

  // Ativa interrup????o e limpa primeira IRQ gerada na ativacao
  pio_enable_interrupt(BUT_PIO, BUT_PIO_PIN_MASK);
  pio_enable_interrupt(BUT_PIO1, BUT_PIO_IDX_MASK1);
  
 // Configura interrup????o no pino referente ao botao e associa fun????o de callback caso uma interrup????o for gerada
  pio_handler_set(BUT_PIO, BUT_PIO_ID, BUT_PIO_PIN_MASK, PIO_IT_FALL_EDGE, but_callback);
  pio_handler_set(BUT_PIO1,BUT_PIO_ID1, BUT_PIO_IDX_MASK1, PIO_IT_FALL_EDGE, but_callback1);

  pio_get_interrupt_status(BUT_PIO);
  pio_get_interrupt_status(BUT_PIO1);

  // Configura NVIC com prioridades 
  NVIC_EnableIRQ(BUT_PIO_ID);
  NVIC_SetPriority(BUT_PIO_ID, 4); // Prioridade 4
  NVIC_EnableIRQ(BUT_PIO_ID1);
  NVIC_SetPriority(BUT_PIO_ID1, 4); // Prioridade 4
}

/************************************************************************/
/* main                                                                 */
/************************************************************************/

/**
 *  \brief FreeRTOS Real Time Kernel example entry point.
 *
 *  \return Unused (ANSI-C compatibility).
 */
int main(void) {
  /* Initialize the SAM system */
  sysclk_init();
  board_init();
  configure_console();
  printf("Sys init DONE \n");

  /* Attempt to create a semaphore. */
  /* ANTES DE USAR O SEM??FORO DEVEMOS CRIAR/INICIALIZAR */

  xSemaphoreBut = xSemaphoreCreateBinary();
  if (xSemaphoreBut == NULL)
    printf("Falha em criar o semaforo BUT \n");

  xSemaphoreBut1 = xSemaphoreCreateBinary();
	if (xSemaphoreBut1 == NULL)
	printf("Falha em criar o semaforo BUT1 \n");

  /* GERENCIANDO COM A FILA */
  /* Cria queue com 32 espacos, cada espa??o possui o tamanho de um inteiro*/
  xQueueLedFreq = xQueueCreate(32, sizeof(uint32_t));
  if (xQueueLedFreq == NULL)
    printf("Falha em criar a queue do LED \n");

  xQueueIncremento = xQueueCreate(32, sizeof(uint32_t));
  if (xQueueIncremento == NULL)
  printf("Falha em criar a queue da FREQ \n");

  /* Create task to make led blink */
  if (xTaskCreate(task_led, "Led", TASK_LED_STACK_SIZE, NULL, TASK_LED_STACK_PRIORITY, NULL) != pdPASS) {
    printf("Failed to create test led task\r\n");
  } else {
    printf("Task led OK \r\n");  
  }

  /* Create task to monitor processor activity */
  if (xTaskCreate(task_but, "BUT", TASK_BUT_STACK_SIZE, NULL, TASK_BUT_STACK_PRIORITY, NULL) != pdPASS) {
    printf("Failed to create UartTx task\r\n");
  } else {
     printf("Task led but OK \r\n");  
  }

  /* Start the scheduler. */
  vTaskStartScheduler();

  /* RTOS n??o deve chegar aqui !! */
  while (1) {}
  /* Will only get here if there was insufficient memory to create the idle
   * task. */
  return 0;
}
