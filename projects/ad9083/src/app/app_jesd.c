/***************************************************************************//**
 *   @file   app_jesd.c
 *   @brief  Application JESD setup.
 *   @author Cristian Pop (cristian.pop@analog.com)
********************************************************************************
 * Copyright 2020(c) Analog Devices, Inc.
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

#include "axi_jesd204_rx.h"
#include "axi_adxcvr.h"
#include "jesd204_clk.h"
#include "error.h"
#include "parameters.h"
#include "app_jesd.h"
#include "adi_cms_api_common.h"
/******************************************************************************/
/************************ Variables Definitions *******************************/
/******************************************************************************/
struct axi_jesd204_rx *rx_jesd;
struct adxcvr *rx_adxcvr;

extern adi_cms_jesd_param_t jtx_param[];
extern uint64_t clk_hz[][3];
/******************************************************************************/
/************************** Functions Implementation **************************/
/******************************************************************************/

/**
 * @brief Application JESD setup.
 * @return SUCCESS in case of success, FAILURE otherwise.
 */
int32_t app_jesd_init(uint8_t uc)
{
	int32_t status;

	struct jesd204_rx_init rx_jesd_init = {
		.name = "rx_jesd",
		.base = RX_JESD_BASEADDR,
		.octets_per_frame = jtx_param[uc].jesd_f,
		.frames_per_multiframe = jtx_param[uc].jesd_n,
		.subclass = jtx_param[uc].jesd_subclass,
		.device_clk_khz = 500000,	/* (lane_clk_khz / 40) */
		.lane_clk_khz = 10000000,	/* LaneRate = ( M/L)*NP*(10/8)*DataRate */
	};

	struct adxcvr_init rx_adxcvr_init = {
		.name = "rx_adxcvr",
		.base = RX_XCVR_BASEADDR,
		.sys_clk_sel = 3,
		.out_clk_sel = 4,
		.cpll_enable = 0,
		.lpm_enable = 1,
		.lane_rate_khz = 10000000,	/* LaneRate = ( M/L)*NP*(10/8)*DataRate */
		.ref_rate_khz = 500000,		/* FPGA_CLK ref */
	};

	status = axi_jesd204_rx_init(&rx_jesd, &rx_jesd_init);
	if (status != SUCCESS) {
		printf("error: %s: axi_jesd204_rx_init() failed\n", rx_jesd_init.name);
		return FAILURE;
	}

	status = adxcvr_init(&rx_adxcvr, &rx_adxcvr_init);
	if (status != SUCCESS) {
		printf("error: %s: adxcvr_init() failed\n", rx_adxcvr_init.name);
		return FAILURE;
	}

	status = adxcvr_clk_enable(rx_adxcvr);
	if (status != SUCCESS) {
		printf("error: %s: adxcvr_clk_enable() failed\n", rx_adxcvr->name);
		return FAILURE;
	}

	status = axi_jesd204_rx_lane_clk_enable(rx_jesd);
	if (status != SUCCESS) {
		printf("error: %s: axi_jesd204_rx_lane_clk_enable() failed\n", rx_jesd->name);
		return FAILURE;
	}

	return SUCCESS;
}
