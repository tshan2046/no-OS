/***************************************************************************//**
 *   @file   headless.c
 *   @brief  AD9083 application example.
 *   @author Cristian Pop (cristian.pop@analog.com)
********************************************************************************
 * Copyright 2021(c) Analog Devices, Inc.
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *  - Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  - Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *  - Neither the name of Analog Devices, Inc. nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *  - The use of this software may or may not infringe the patent rights
 *    of one or more patent holders.  This license does not release you
 *    from the requirement that you obtain separate licenses from these
 *    patent holders to use this software.
 *  - Use of the software either in source or binary form, must be run
 *    on or directly connected to an Analog Devices Inc. component.
 *
 * THIS SOFTWARE IS PROVIDED BY ANALOG DEVICES "AS IS" AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, NON-INFRINGEMENT,
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL ANALOG DEVICES BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, INTELLECTUAL PROPERTY RIGHTS, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*******************************************************************************/

/******************************************************************************/
/***************************** Include Files **********************************/
/******************************************************************************/

#include <stdio.h>
#include "ad9083.h"
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
#include "app_clocking.h"
#include "app_jesd.h"
#include "app_ad9083.h"
#include "ad9528.h"

#ifdef IIO_SUPPORT
#include "app_iio.h"
#endif

extern struct axi_jesd204_rx *rx_jesd;
int main(void)
{
	uint8_t uc = 7;
	int32_t status;
	struct axi_adc_init rx_adc_init = {
		.name = "rx_adc",
		.base = RX_CORE_BASEADDR,
		.num_channels = 16,
	};
	struct axi_adc *rx_adc;

	struct axi_dmac_init rx_dmac_init = {
		"rx_dmac",
		RX_DMA_BASEADDR,
		DMA_DEV_TO_MEM,
		0
	};
	struct axi_dmac *rx_dmac;


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

	axi_adc_init(&rx_adc, &rx_adc_init);

	axi_dmac_init(&rx_dmac, &rx_dmac_init);
#ifdef IIO_SUPPORT
	printf("The board accepts libiio clients connections through the serial backend.\n");

	struct iio_axi_adc_init_param iio_axi_adc_init_par;
	iio_axi_adc_init_par = (struct iio_axi_adc_init_param) {
		.rx_adc = rx_adc,
		.rx_dmac = rx_dmac,
	};

	return iio_server_init(&iio_axi_adc_init_par);
#else
	printf("Bye\n");

	return SUCCESS;
#endif
}
