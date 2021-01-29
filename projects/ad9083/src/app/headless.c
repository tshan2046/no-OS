/**
 * \file adrv9009/src/app/headless.c
 *
 * \brief Contains example code for user integration with their application
 *
 * Copyright 2015-2017 Analog Devices Inc.
 * Released under the AD9378-AD9379 API license, for more information see the "LICENSE.txt" file in this zip file.
 *
 */

/****< Insert User Includes Here >***/

#include <stdio.h>
#include "ad9083.h"
//#include "adi_hal.h"
#include "inttypes.h"
#include "spi.h"
#include "spi_extra.h"
#include "gpio_extra.h"
#include "error.h"
#include "delay.h"
#include "parameters.h"
#include "util.h"
#include "axi_jesd204_tx.h"
#include "axi_adc_core.h"
#include "axi_dmac.h"
#ifndef ALTERA_PLATFORM
#include "xil_cache.h"
#endif
//#include "talise.h"
//#include "talise_config.h"
#include "app_config.h"
#include "app_clocking.h"
#include "app_jesd.h"
#include "app_ad9083.h"
#include "ad9528.h"

#ifdef IIO_SUPPORT

#include "iio.h"
#include "iio_axi_adc.h"
#include "iio_axi_dac.h"
#include "irq.h"
#include "irq_extra.h"
#include "uart.h"
#include "uart_extra.h"

static struct uart_desc *uart_desc;

/**
 * iio_uart_write() - Write data to UART device wrapper.
 * @buf - Pointer to buffer containing data.
 * @len - Number of bytes to write.
 * @Return: SUCCESS in case of success, FAILURE otherwise.
 */
static ssize_t iio_uart_write(const char *buf, size_t len)
{
	return uart_write(uart_desc, (const uint8_t *)buf, len);
}

/**
 * iio_uart_read() - Read data from UART device wrapper.
 * @buf - Pointer to buffer containing data.
 * @len - Number of bytes to read.
 * @Return: SUCCESS in case of success, FAILURE otherwise.
 */
static ssize_t iio_uart_read(char *buf, size_t len)
{
	return uart_read(uart_desc, (uint8_t *)buf, len);
}

#endif // IIO_SUPPORT

/**********************************************************/
/**********************************************************/
/********** Talise Data Structure Initializations ********/
/**********************************************************/
/**********************************************************/
extern struct axi_jesd204_rx *rx_jesd;
int main(void)
{
	uint8_t uc = 7;
	int32_t status;

	printf("Hello\n");

	status = app_clocking_init(uc);
	if (status != SUCCESS) {
		printf("app_clock_init() error: %" PRId32 "\n", status);

		return FAILURE;
	}

	status = app_jesd_init(uc);
	if (status != SUCCESS) {
		printf("app_jesd_init() error: %" PRId32 "\n", status);

		return FAILURE;
	}

	status = app_ad9083_init(uc);
	if (status != SUCCESS) {
		printf("app_clock_init() error: %" PRId32 "\n", status);

		return FAILURE;
	}

	status = axi_jesd204_rx_status_read(rx_jesd);
	if (status != SUCCESS) {
		printf("axi_jesd204_tx_status_read() error: %"PRIi32"\n", status);
	}

	printf("Bye\n");
	return SUCCESS;
}
