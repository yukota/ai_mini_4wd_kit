/*
 * samd51_gpio.h
 *
 * Created: 2019/03/10
 * Copyright ? 2019 Kiyotaka Akasaka. All rights reserved.
 */ 


#ifndef SAMD51_GPIO_H_
#define SAMD51_GPIO_H_

typedef enum SAMD51_GPIO_PORT_t
{
	SAMD51_GPIO_A00 = 0x000a0000,
	SAMD51_GPIO_A01,
	SAMD51_GPIO_A02,
	SAMD51_GPIO_A03,
	SAMD51_GPIO_A04,
	SAMD51_GPIO_A05,
	SAMD51_GPIO_A06,
	SAMD51_GPIO_A07,
	SAMD51_GPIO_A08,
	SAMD51_GPIO_A09,
	SAMD51_GPIO_A10,
	SAMD51_GPIO_A11,
	SAMD51_GPIO_A12,
	SAMD51_GPIO_A13,
	SAMD51_GPIO_A14,
	SAMD51_GPIO_A15,
	SAMD51_GPIO_A16,
	SAMD51_GPIO_A17,
	SAMD51_GPIO_A18,
	SAMD51_GPIO_A19,
	SAMD51_GPIO_A20,
	SAMD51_GPIO_A21,
	SAMD51_GPIO_A22,
	SAMD51_GPIO_A23,
	SAMD51_GPIO_A24,
	SAMD51_GPIO_A25,
	SAMD51_GPIO_A26,
	SAMD51_GPIO_A27,
	SAMD51_GPIO_A28,
	SAMD51_GPIO_A29,
	SAMD51_GPIO_A30,
	SAMD51_GPIO_A31,

	SAMD51_GPIO_B00 = 0x000b0000,
	SAMD51_GPIO_B01,
	SAMD51_GPIO_B02,
	SAMD51_GPIO_B03,
	SAMD51_GPIO_B04,
	SAMD51_GPIO_B05,
	SAMD51_GPIO_B06,
	SAMD51_GPIO_B07,
	SAMD51_GPIO_B08,
	SAMD51_GPIO_B09,
	SAMD51_GPIO_B10,
	SAMD51_GPIO_B11,
	SAMD51_GPIO_B12,
	SAMD51_GPIO_B13,
	SAMD51_GPIO_B14,
	SAMD51_GPIO_B15,
	SAMD51_GPIO_B16,
	SAMD51_GPIO_B17,
	SAMD51_GPIO_B18,
	SAMD51_GPIO_B19,
	SAMD51_GPIO_B20,
	SAMD51_GPIO_B21,
	SAMD51_GPIO_B22,
	SAMD51_GPIO_B23,
	SAMD51_GPIO_B24,
	SAMD51_GPIO_B25,
	SAMD51_GPIO_B26,
	SAMD51_GPIO_B27,
	SAMD51_GPIO_B28,
	SAMD51_GPIO_B29,
	SAMD51_GPIO_B30,
	SAMD51_GPIO_B31,
} SAMD51_GPIO_PORT;

typedef enum SAMD51_GPIO_DIR_t
{
	SAMD51_GPIO_OUT,
	SAMD51_GPIO_IN,
	SAMD51_GPIO_INOUT,
	SAMD51_GPIO_DISCONNECT,
} SAMD51_GPIO_DIR;

typedef enum SAMD51_GPIO_PULL_t
{
	SAMD51_GPIO_NO_PULL = 0,
	SAMD51_GPIO_PULLUP_DOWN = (1 << 18)
} SAMD51_GPIO_PULL;

typedef enum SAMD51_GPIO_PORTMUX_t
{
	SAMD51_GPIO_MUX_FUNC_A = 0x0,
	SAMD51_GPIO_MUX_FUNC_B = 0x01,
	SAMD51_GPIO_MUX_FUNC_C = 0x02,
	SAMD51_GPIO_MUX_FUNC_D = 0x03,
	SAMD51_GPIO_MUX_FUNC_E = 0x04,
	SAMD51_GPIO_MUX_FUNC_F = 0x05,
	SAMD51_GPIO_MUX_FUNC_G = 0x06,
	SAMD51_GPIO_MUX_FUNC_H = 0x07,
	SAMD51_GPIO_MUX_FUNC_I = 0x08,
	SAMD51_GPIO_MUX_FUNC_J = 0x09,
	SAMD51_GPIO_MUX_FUNC_K = 0x0a,
	SAMD51_GPIO_MUX_FUNC_L = 0x0b,
	SAMD51_GPIO_MUX_FUNC_M = 0x0c,
	SAMD51_GPIO_MUX_FUNC_N = 0x0d,
	SAMD51_GPIO_MUX_DEFAULT = 0xffffffff
} SAMD51_GPIO_PORTMUX;


typedef struct REG_PORT_PMUX_t
{
	uint8_t pmux[16];
} REG_PORT_PMUX_PACK;

#define REG_PORT_PMUX0_BASE				0x41008030UL
#define REG_PORT_PMUX0_					(*((volatile REG_PORT_PMUX_PACK *)REG_PORT_PMUX0_BASE));

#define REG_PORT_PMUX1_BASE				(REG_PORT_PMUX0_BASE + 0x80UL)
#define REG_PORT_PMUX1_					(*((volatile REG_PORT_PMUX_PACK *)REG_PORT_PMUX1_BASE));



int samd51_gpio_configure(SAMD51_GPIO_PORT port, SAMD51_GPIO_DIR dir, SAMD51_GPIO_PULL pull, SAMD51_GPIO_PORTMUX pmux);

int samd51_gpio_input(SAMD51_GPIO_PORT port);
int samd51_gpio_output(SAMD51_GPIO_PORT port, uint8_t out);
int samd51_gpio_output_toggle(SAMD51_GPIO_PORT port);

#endif /* SAMD51_GPIO_H_ */