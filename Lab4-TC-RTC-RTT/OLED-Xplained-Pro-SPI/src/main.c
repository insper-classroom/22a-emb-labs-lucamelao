#include <asf.h>
#include "gfx_mono_ug_2832hsweg04.h"
#include "gfx_mono_text.h"
#include "sysfont.h"

typedef struct  {
	uint32_t year;
	uint32_t month;
	uint32_t day;
	uint32_t week;
	uint32_t hour;
	uint32_t minute;
	uint32_t second;
} calendar;

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

/************************************************************************/
/* PROTOTYPES                                             */
/************************************************************************/
void LED_init(int estado);
void TC_init(Tc * TC, int ID_TC, int TC_CHANNEL, int freq);
void pin_toggle(Pio *pio, uint32_t mask);
void RTC_init(Rtc *rtc, uint32_t id_rtc, calendar t, uint32_t irq_type);
void pisca_led(int n, int t);
void but_callBack(void);
/************************************************************************/
/* VAR globais                                                          */
/************************************************************************/
volatile char flag_rtc_alarm = 0;
volatile char but1_flag = 0;
volatile char count_flag = 0;
/************************************************************************/
/* INITS                                                          */
/************************************************************************/
void LED_init(int estado) {
	pmc_enable_periph_clk(LED_PIO_ID);
	pio_set_output(LED_PIO, LED_IDX_MASK, estado, 0, 0);
	
	pmc_enable_periph_clk(LED_PIO_ID1);
	pio_set_output(LED_PIO1, LED_IDX_MASK1, estado, 0, 0);
	
	pmc_enable_periph_clk(LED_PIO_ID2);
	pio_set_output(LED_PIO2, LED_IDX_MASK2, estado, 0, 0);
	
	pmc_enable_periph_clk(LED_PIO_ID3);
	pio_set_output(LED_PIO3, LED_IDX_MASK3, estado, 0, 0); // inicia apagado
};

void TC_init(Tc * TC, int ID_TC, int TC_CHANNEL, int freq){
	uint32_t ul_div;
	uint32_t ul_tcclks;
	uint32_t ul_sysclk = sysclk_get_cpu_hz();

	/* Configura o PMC */
	pmc_enable_periph_clk(ID_TC);

	/** Configura o TC para operar em  freq hz e interrupçcão no RC compare */
	tc_find_mck_divisor(freq, ul_sysclk, &ul_div, &ul_tcclks, ul_sysclk);
	tc_init(TC, TC_CHANNEL, ul_tcclks | TC_CMR_CPCTRG);
	tc_write_rc(TC, TC_CHANNEL, (ul_sysclk / ul_div) / freq);

	/* Configura NVIC*/
	NVIC_SetPriority(ID_TC, 4);
	NVIC_EnableIRQ((IRQn_Type) ID_TC);
	tc_enable_interrupt(TC, TC_CHANNEL, TC_IER_CPCS);
}

static void RTT_init(float freqPrescale, uint32_t IrqNPulses, uint32_t rttIRQSource) {

	uint16_t pllPreScale = (int) (((float) 32768) / freqPrescale);
	
	rtt_sel_source(RTT, false);
	rtt_init(RTT, pllPreScale);
	
	if (rttIRQSource & RTT_MR_ALMIEN) {
		uint32_t ul_previous_time;
		ul_previous_time = rtt_read_timer_value(RTT);
		while (ul_previous_time == rtt_read_timer_value(RTT));
		rtt_write_alarm_time(RTT, IrqNPulses+ul_previous_time);
	}

	/* config NVIC */
	NVIC_DisableIRQ(RTT_IRQn);
	NVIC_ClearPendingIRQ(RTT_IRQn);
	NVIC_SetPriority(RTT_IRQn, 4);
	NVIC_EnableIRQ(RTT_IRQn);

	/* Enable RTT interrupt */
	if (rttIRQSource & (RTT_MR_RTTINCIEN | RTT_MR_ALMIEN))
	rtt_enable_interrupt(RTT, rttIRQSource);
	else
	rtt_disable_interrupt(RTT, RTT_MR_RTTINCIEN | RTT_MR_ALMIEN);
	
}

void RTC_init(Rtc *rtc, uint32_t id_rtc, calendar t, uint32_t irq_type) {
	/* Configura o PMC */
	pmc_enable_periph_clk(ID_RTC);

	/* Default RTC configuration, 24-hour mode */
	rtc_set_hour_mode(rtc, 0);

	/* Configura data e hora manualmente */
	rtc_set_date(rtc, t.year, t.month, t.day, t.week);
	rtc_set_time(rtc, t.hour, t.minute, t.second);

	/* Configure RTC interrupts */
	NVIC_DisableIRQ(id_rtc);
	NVIC_ClearPendingIRQ(id_rtc);
	NVIC_SetPriority(id_rtc, 4);
	NVIC_EnableIRQ(id_rtc);

	/* Ativa interrupcao via alarme */
	rtc_enable_interrupt(rtc,  irq_type);
}
/************************************************************************/
/* FUNCTIONS                                                          */
/************************************************************************/
void pin_toggle(Pio *pio, uint32_t mask) {
	if(pio_get_output_data_status(pio, mask))
	pio_clear(pio, mask);
	else
	pio_set(pio,mask);
}

static float get_time_rtt(){
	uint ul_previous_time = rtt_read_timer_value(RTT);
}

void pisca_led (int n, int t) {
	for (int i=0;i<n;i++){
		pio_clear(LED_PIO3, LED_IDX_MASK3);
		delay_ms(t);
		pio_set(LED_PIO3, LED_IDX_MASK3);
		delay_ms(t);
	}
}

void set_alarm_but1() {
	uint32_t current_hour, current_min, current_sec;
	uint32_t current_year, current_month, current_day, current_week;
	
	rtc_get_time(RTC, &current_hour, &current_min, &current_sec);
	rtc_get_date(RTC, &current_year, &current_month, &current_day, &current_week);
	
	rtc_set_date_alarm(RTC, 1, current_month, 1, current_day);
	rtc_set_time_alarm(RTC, 1, current_hour, 1, current_min, 1, current_sec + 20);
}

void set_time() {
	gfx_mono_draw_string("        ", 5, 16, &sysfont);
	char str[15];
	uint32_t current_hour, current_min, current_sec;
	rtc_get_time(RTC, &current_hour, &current_min, &current_sec);
	sprintf(str, "%d:%d:%d", current_hour,current_min,current_sec);
	gfx_mono_draw_string(str, 5, 16, &sysfont);
}

/************************************************************************/
/* HANDLERS                                                          */
/************************************************************************/
void TC1_Handler(void) {
	/**
	* Devemos indicar ao TC que a interrupção foi satisfeita.
	* Isso é realizado pela leitura do status do periférico
	**/
	volatile uint32_t status = tc_get_status(TC0, 1);

	/** Muda o estado do LED (pisca) **/
	pin_toggle(LED_PIO, LED_IDX_MASK);
}

void TC2_Handler(void) {
	/**
	* Devemos indicar ao TC que a interrupção foi satisfeita.
	* Isso é realizado pela leitura do status do periférico
	**/
	volatile uint32_t status = tc_get_status(TC0, 2);

	/** Muda o estado do LED (pisca) **/
	pin_toggle(LED_PIO1, LED_IDX_MASK1);
}

void RTT_Handler(void) {
	uint32_t ul_status;

	/* Get RTT status - ACK */
	ul_status = rtt_get_status(RTT);

	/* IRQ due to Alarm */
	if ((ul_status & RTT_SR_ALMS) == RTT_SR_ALMS) {
		RTT_init(4, 16, RTT_MR_ALMIEN);
	}
	
	/* IRQ due to Time has changed */
	if ((ul_status & RTT_SR_RTTINC) == RTT_SR_RTTINC) {
		pin_toggle(LED_PIO2, LED_IDX_MASK2);    // BLINK Led
	}

}

void RTC_Handler(void) {
	uint32_t ul_status = rtc_get_status(RTC);
	
	/* Time or date alarm */
	if ((ul_status & RTC_SR_ALARM) == RTC_SR_ALARM) {
		// o código para irq de alame vem aqui
		flag_rtc_alarm = 1;
	}
	
	/* seccond tick */
	if ((ul_status & RTC_SR_SEC) == RTC_SR_SEC) {
		// o código para irq de segundo vem aqui
		count_flag = 1;
	}
	rtc_clear_status(RTC, RTC_SCCR_SECCLR);
	rtc_clear_status(RTC, RTC_SCCR_ALRCLR);
	rtc_clear_status(RTC, RTC_SCCR_ACKCLR);
	rtc_clear_status(RTC, RTC_SCCR_TIMCLR);
	rtc_clear_status(RTC, RTC_SCCR_CALCLR);
	rtc_clear_status(RTC, RTC_SCCR_TDERRCLR);
}

void but_callBack(void){
	but1_flag = 1;
}

void io_init(void)
{
	// Init OLED
	gfx_mono_ssd1306_init();
	// Configura led
	pio_configure(LED_PIO3, PIO_OUTPUT_0, LED_IDX_MASK, PIO_DEFAULT);

	// Inicializa clock do periférico PIO responsavel pelo botao
	pmc_enable_periph_clk(BUT_PIO_ID1);

	// Configura PIO para lidar com o pino do botão como entrada com pull-up
	pio_configure(BUT_PIO1, PIO_INPUT, BUT_IDX_MASK1, PIO_PULLUP);

	// Configura interrupção no pino referente ao botao e associa função de callback caso uma interrupção for gerada a função de callback é a: but_callback()
	pio_handler_set(BUT_PIO1, BUT_PIO_ID1, BUT_IDX_MASK1, PIO_IT_EDGE, but_callBack);

	// Ativa interrupção e limpa primeira IRQ gerada na ativacao
	pio_enable_interrupt(BUT_PIO1, BUT_IDX_MASK1);
	pio_get_interrupt_status(BUT_PIO1);

	// Configura NVIC para receber interrupcoes do PIO do botao com prioridade 4 (quanto mais próximo de 0 maior)
	NVIC_EnableIRQ(BUT_PIO_ID1);
	NVIC_SetPriority(BUT_PIO_ID1, 4);
}

/* MAIN */
int main (void)
{
	io_init();
	board_init();
	sysclk_init();
	LED_init(1);
	    
	TC_init(TC0, ID_TC1, 1, 10);
	tc_start(TC0, 1);

	TC_init(TC0, ID_TC2, 2, 8);
	tc_start(TC0, 2);
	
	RTT_init(4, 16, RTT_MR_ALMIEN); 
	
	/** Configura RTC */
	calendar rtc_initial = {2018, 3, 19, 12, 15, 45 ,1};
	RTC_init(RTC, ID_RTC, rtc_initial, RTC_IER_ALREN | RTC_IER_SECEN);

	while(1) {
		if(flag_rtc_alarm){
			pisca_led(5, 200);
			flag_rtc_alarm = 0;
		}
		if(but1_flag){
			set_alarm_but1();
			but1_flag = 0;
		}
		if (count_flag) {
			set_time();
			count_flag = 0;
		}
	pmc_sleep(SAM_PM_SMODE_SLEEP_WFI);
	}
}
