/*
 * samd51_adc.h
 *
 * Created: 2019/03/10
 * Copyright ? 2019 Kiyotaka Akasaka. All rights reserved.
 */ 


#ifndef SAMD51_ADC_H_
#define SAMD51_ADC_H_

typedef enum SAMD51_ADC_REFERENCE_t
{
	SAMD51_ADC_REF_INTERNAL = 0,
	SAMD51_ADC_REF_INTVCC0 = 2,
	SAMD51_ADC_REF_INTVCC1 = 3,
	SAMD51_ADC_REF_EXTERNAL_REFA = 4,
	SAMD51_ADC_REF_EXTERNAL_REFB = 5,
	SAMD51_ADC_REF_EXTERNAL_REFC = 6
} SAMD51_ADC_REFERENCE;


typedef enum SAMD51_ADC_BIT_RESOLUTION_t
{
	SAMD51_ADC_BIT_RES_8 = 3,
	SAMD51_ADC_BIT_RES_10 = 2,
	SAMD51_ADC_BIT_RES_12 = 0
} SAMD51_ADC_BIT_RESOLUTION;


typedef enum SAMD51_ADC_MODE_t
{
	SAMD51_ADC_SINGLE_SHOT = 0,
	SAMD51_ADC_FREE_RUN = 1
} SAMD51_ADC_MODE;

typedef enum SAMD51_ADC_INPUT_TYPE_t
{
	SAMD51_ADC_SINGLE_END = 0,
	SAMD51_ADC_DIFFERNTIAL = 1	
} SAMD51_ADC_INPUT_TYPE;

typedef enum SAMD51_ADC_POS_INPUT_t
{
	SAMD51_ADC_POS_AIN0 = 0,
	SAMD51_ADC_POS_AIN1,
	SAMD51_ADC_POS_AIN2,
	SAMD51_ADC_POS_AIN3,
	SAMD51_ADC_POS_AIN4,
	SAMD51_ADC_POS_AIN5,
	SAMD51_ADC_POS_AIN6,
	SAMD51_ADC_POS_AIN7,
	SAMD51_ADC_POS_AIN8,
	SAMD51_ADC_POS_AIN9,
	SAMD51_ADC_POS_AIN10,
	SAMD51_ADC_POS_AIN11,
	SAMD51_ADC_POS_AIN12,
	SAMD51_ADC_POS_AIN13,
	SAMD51_ADC_POS_AIN14,
	SAMD51_ADC_POS_AIN15,
	SAMD51_ADC_POS_AIN16,
	SAMD51_ADC_POS_AIN17,
	SAMD51_ADC_POS_AIN18,
	SAMD51_ADC_POS_AIN19,
	SAMD51_ADC_POS_AIN20,
	SAMD51_ADC_POS_AIN21,
	SAMD51_ADC_POS_AIN22,
	SAMD51_ADC_POS_AIN23,
	SAMD51_ADC_POS_SCALEDCOREVCC,
	SAMD51_ADC_POS_SCALEDVBAT,
	SAMD51_ADC_POS_SCALEDIOVCC,
	SAMD51_ADC_POS_BANDGAP,
	SAMD51_ADC_POS_PTAT,
	SAMD51_ADC_POS_CTAT,
	SAMD51_ADC_DAC
} SAMD51_ADC_POS_INPUT;

typedef enum SAMD51_ADC_NEG_INPUT_t
{
	SAMD51_ADC_NEG_AIN0 = 0,
	SAMD51_ADC_NEG_AIN1,
	SAMD51_ADC_NEG_AIN2,
	SAMD51_ADC_NEG_AIN3,
	SAMD51_ADC_NEG_AIN4,
	SAMD51_ADC_NEG_AIN5,
	SAMD51_ADC_NEG_AIN6,
	SAMD51_ADC_NEG_AIN7,
	SAMD51_ADC_NEG_GND  = 0x18
} SAMD51_ADC_NEG_INPUT;


typedef enum SAMD51_ADC_AVERAGE_COUNT_t{
	SAMD51_ADC_AVERAGE_1_SAMPLES = 0,
	SAMD51_ADC_AVERAGE_2_SAMPLES = 1,
	SAMD51_ADC_AVERAGE_4_SAMPLES = 2,
	SAMD51_ADC_AVERAGE_8_SAMPLES = 3,
	SAMD51_ADC_AVERAGE_16_SAMPLES = 4,
	SAMD51_ADC_AVERAGE_32_SAMPLES = 5,
	SAMD51_ADC_AVERAGE_64_SAMPLES = 6,
	SAMD51_ADC_AVERAGE_128_SAMPLES = 7,
	SAMD51_ADC_AVERAGE_256_SAMPLES = 8,
	SAMD51_ADC_AVERAGE_512_SAMPLES = 9,
	SAMD51_ADC_AVERAGE_1024_SAMPLES = 10,
} SAMD51_ADC_AVERAGE_COUNT;

typedef struct SAMD51_ADC_POST_PROCESS_OPT_t
{
	SAMD51_ADC_AVERAGE_COUNT average_cnt;	
	uint8_t average_div_power;
} SAMD51_ADC_POST_PROCESS_OPT;

typedef void (*SAMD51_ADC_CONVERSION_DONE_CB)(int status, int16_t result);


int samd51_adc_setup(uint32_t ch, SAMD51_ADC_MODE mode, SAMD51_ADC_BIT_RESOLUTION res, SAMD51_ADC_REFERENCE ref, SAMD51_ADC_POST_PROCESS_OPT *opt);
int samd51_adc_convert(uint32_t ch, SAMD51_ADC_INPUT_TYPE input, SAMD51_ADC_POS_INPUT pos, SAMD51_ADC_NEG_INPUT neg, SAMD51_ADC_CONVERSION_DONE_CB cb);
int samd51_adc_get_result(uint32_t ch, int16_t *result);
int samd51_adc_abort(uint32_t ch);



#endif /* SAMD51_ADC_H_ */