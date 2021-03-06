/*
 * system_software.c
 *
 * Created: 2019/03/10
 * Copyright ? 2019 Kiyotaka Akasaka. All rights reserved.
 */ 
#include <stdint.h>
#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <stdarg.h>

#include <samd51_error.h>
#include <samd51_gpio.h>
#include <samd51_sercom.h>
#include <samd51_uart.h>
#include <samd51_clock.h>
#include <samd51_timer.h>
#include <samd51_i2c.h>
#include <samd51_sdhc.h>
#include <samd51_adc.h>

#include "include/ai_mini4wd.h"
#include "include/internal/gpio.h"
#include "include/internal/sensor.h"
#include "include/internal/timer.h"
#include "include/internal/clock.h"
#include "include/internal/pwm.h"
#include "include/internal/mmc.h"
#include "include/internal/hids.h"
#include "include/internal/fs.h"

#include "include/ai_mini4wd_sensor.h"
#include "include/ai_mini4wd_motor_driver.h"
#include "include/ai_mini4wd_hid.h"
#include "include/ai_mini4wd_timer.h"


typedef struct GlobalParameters_t
{
	uint8_t enabledPrintf;
} GlobalParameters;

static volatile GlobalParameters sGlobalParams;


static uint8_t sUartTxBuf[512];
static uint8_t sUartRxBuf[512];

static SAMD51_UART_FIFO sUartTxFifo;
static SAMD51_UART_FIFO sUartRxFifo;


// From prohect file
void SystemInit(void);

int aiMini4wdInitialize(uint32_t flags)
{
    /* Initialize the SAM system */
    SystemInit();

	memset ((void *)&sGlobalParams, 0x00, sizeof(sGlobalParams));

	//J GPIO itialization
	(void)aiMini4wdInitializeGpio();

	//J 1MHzのクロックソースを作る
	samd51_gclk_configure_generator(LIB_MINI_4WD_CLK_GEN_NUMBER_1MHZ, SAMD51_GCLK_SRC_DFLL, 48, 0, SAMD51_GCLK_DIV_NORMAL);

	//J 32kHz backup clock
	samd51_gclk_configure_generator(LIB_MINI_4WD_CLK_GEN_NUMBER_32kHZ, SAMD51_GCLK_SRC_OSCULP32K, 0, 0, SAMD51_GCLK_DIV_NORMAL);

	//J 48MHzのクロック源をMAINとは切り離す
	samd51_gclk_configure_generator(LIB_MINI_4WD_CLK_GEN_NUMBER_48MHZ, SAMD51_GCLK_SRC_DFLL, 0, 0, SAMD51_GCLK_DIV_NORMAL);
	//J 48MHz 基準クロックをクロジェネ0番から移動させる
	samd51_gclk_configure_peripheral_channel(SAMD51_GCLK_OSCCTRL_DFLL48, LIB_MINI_4WD_CLK_GEN_NUMBER_48MHZ);

	//J DFLL用のクロックを準備
	samd51_gclk_configure_peripheral_channel(SAMD51_GCLK_OSCCTRL_FDPLL_32K, LIB_MINI_4WD_CLK_GEN_NUMBER_32kHZ);
	samd51_gclk_configure_peripheral_channel(SAMD51_GCLK_OSCCTRL_FDPLL0, LIB_MINI_4WD_CLK_GEN_NUMBER_1MHZ);

	//J DFLLを120MHzで起動
	samd51_mclk_enable(SAMD51_APBA_OSCCTRL, 1);
	SAMD51_OSC_OPT osc_opt;
	{
		osc_opt.dco_filter = 0;
		osc_opt.filter = 0;
		osc_opt.filter_en = 1;		
	}
	samd51_oscillator_dpll_enable(0, SAMD51_OSC_REF_GCLK, 1000000, 120000000, &osc_opt);
	
	//J 120MHzになったFDPLLをGCLK0のソースに設定する（死にそう）
	samd51_gclk_configure_generator(LIB_MINI_4WD_CLK_GEN_NUMBER_MAIN, SAMD51_GCLK_SRC_DPLL0, 0, 0, SAMD51_GCLK_DIV_NORMAL);

	//J Debug UART
	if (flags & AI_MINI_4WD_INIT_FLAG_USE_DEBUG_PRINT) {
		sGlobalParams.enabledPrintf = 1;

		samd51_uart_fifo_setup(&sUartTxFifo, sUartTxBuf, sizeof(sUartTxBuf));
		samd51_uart_fifo_setup(&sUartRxFifo, sUartRxBuf, sizeof(sUartRxBuf));
		samd51_mclk_enable(SAMD51_APBB_SERCOM2, 1);
		samd51_gclk_configure_peripheral_channel(SAMD51_GCLK_SERCOM2_CORE, LIB_MINI_4WD_CLK_GEN_NUMBER_48MHZ);
		samd51_uart_initialize(SAMD51_SERCOM2, 115200, SAMD51_SERCOM_PAD1, SAMD51_SERCOM_PAD0, &sUartTxFifo, &sUartRxFifo);
	}

	//J Timers
	aiMini4WdInitializeTimer();
	
	//J FSの初期化. 内部的に2000ms 待つので旧来のWaitを消す
	aiMini4wdFsInitialize();
	
//	volatile uint32_t tick = aiMini4WdTimerGetSystemtick();
//	while ((tick + 2000) > aiMini4WdTimerGetSystemtick());
	
	//J Sensor 関係処理の初期化
	//J 加速度センサ、タコメータ
	aiMini4wdSensorsInitialize();
	
	//J Motor Driver
	aiMini4WdInitializePwm();
	
/*
	FIL fp;
	res = f_open(&fp, "TEST.TXT", FA_READ);
	aiMini4wdDebugPrintf ("f_open(). res = %d\r\n", res);

	char str[10] = "";
	f_read(&fp, str, 5, NULL);
	
	aiMini4wdDebugPrintf("[TEST] %s\r\n", str);
	
	f_close(&fp);

	res = f_open(&fp, "TEST2.TXT", FA_CREATE_ALWAYS | FA_WRITE);
	aiMini4wdDebugPrintf ("f_open(). res = %d\r\n", res);

	char test[2048];
	memset (test, 'A', sizeof(test));

	f_write(&fp, test, sizeof(test), NULL);
	
	f_close(&fp);
*/

	//J ADC for Battery Voltage and Motor Current
	samd51_mclk_enable(SAMD51_APBD_ADCn0, 1);
	samd51_mclk_enable(SAMD51_APBD_ADCn1, 1);
	samd51_gclk_configure_peripheral_channel(SAMD51_GCLK_ADC0, LIB_MINI_4WD_CLK_GEN_NUMBER_48MHZ);
	samd51_gclk_configure_peripheral_channel(SAMD51_GCLK_ADC1, LIB_MINI_4WD_CLK_GEN_NUMBER_48MHZ);

	//J refer to Table 45-3. Averaging.	
	SAMD51_ADC_POST_PROCESS_OPT adc0_opt;
	{
		adc0_opt.average_cnt = SAMD51_ADC_AVERAGE_16_SAMPLES;
		adc0_opt.average_div_power = 4;
	}
	int ret = samd51_adc_setup(0, SAMD51_ADC_SINGLE_SHOT, SAMD51_ADC_BIT_RES_12, SAMD51_ADC_REF_EXTERNAL_REFA, &adc0_opt);
	if (ret != AI_OK) {
		aiMini4wdDebugPrintf("Failed to samd51_adc_setup(). ret = 0x%08x\r\n", ret);
		return ret;
	}

#if 0 //J ADC1が動かないのでADC0 をチャンネル切り替えて使うしかない
	//J 変換を開始（連続変換なのでこの後ずっと変換されるはず
	ret = samd51_adc_convert(0, SAMD51_ADC_SINGLE_END, SAMD51_ADC_POS_AIN14, SAMD51_ADC_NEG_GND, NULL);
	if (ret != AI_OK) {
		aiMini4wdDebugPrintf("Failed to samd51_adc_convert(). ret = 0x%08x\r\n", ret);
		return ret;
	}
#endif

#if 0 // ADC1がAIN14/15を接続すると動かない疑惑
	//J モータードライバの電流計測用 こちらも基本変換しっぱなしにしておいて、必用に応じて都度読む
	ret = samd51_adc_setup(1, SAMD51_ADC_FREE_RUN, SAMD51_ADC_BIT_RES_12, SAMD51_ADC_REF_EXTERNAL_REFA, &adc0_opt);
	if (ret != AI_OK) {
		aiMini4wdDebugPrintf("Failed to samd51_adc_setup(). ret = 0x%08x\r\n", ret);
		return ret;
	}

	ret = samd51_adc_convert(1, SAMD51_ADC_SINGLE_END, SAMD51_ADC_POS_SCALEDIOVCC, SAMD51_ADC_NEG_GND, NULL);
	if (ret != AI_OK) {
		aiMini4wdDebugPrintf("Failed to samd51_adc_convert(). ret = 0x%08x\r\n", ret);
		return ret;
	}
#endif

	//J Status OK
	aiMini4wdSetStatusLed(1);
	
	return 0;
}

static int sErrorStatus = 0;
static int sStatusLed = 0;
void aiMini4wdUpdateErrorStatusIndication(void)
{
	if (sErrorStatus == 0) {
		aiMini4wdSetStatusLed(1);
	}
	else {
		aiMini4wdSetStatusLed(sStatusLed);
		sStatusLed = 1-sStatusLed;
	}
}

void aiMini4wdSetErrorStatus(int status)
{
	sErrorStatus = status;
	aiMini4wdDebugPrintf("[FATAL] : Error Status = 0x%08x(%d)\r\n", status, status);
	
	return;
}


char gCommonLineBuf[512];
int aiMini4wdDebugPrintf(const char *format, ...)
{
	if (!sGlobalParams.enabledPrintf) {
		return 0;
	}
    va_list ap;

    va_start( ap, format );
	int len = vsnprintf(gCommonLineBuf, sizeof(gCommonLineBuf), format, ap );
    va_end( ap );

	samd51_uart_puts(SAMD51_SERCOM2, gCommonLineBuf);

	return len;
}

int aiMini4wdDebugPuts(const char *str, size_t len)
{
	if (!sGlobalParams.enabledPrintf) {
		return 0;
	}

	return samd51_uart_tx(SAMD51_SERCOM2, (uint8_t *)str, len);
}

int aiMini4wdDebugTryGetc(void)
{
	if (!sGlobalParams.enabledPrintf) {
		return 0;
	}

	char c = 0;
	int ret = samd51_uart_try_rx(SAMD51_SERCOM2, (uint8_t *)&c);
	if (ret == 0) {
		ret = ((int)c & 0xff);
	}

	return ret;
}

int aiMini4wdDebugGetc(void)
{
	if (!sGlobalParams.enabledPrintf) {
		return 0;
	}

	char c = 0;
	int ret = samd51_uart_rx(SAMD51_SERCOM2, (uint8_t *)&c, 1);
	if (ret == 0) {
		ret = ((int)c & 0xff);
	}

	return ret;
}