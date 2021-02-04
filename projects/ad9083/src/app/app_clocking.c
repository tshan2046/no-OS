/***************************************************************************//**
 *   @file   app_clocking.c
 *   @brief  Clock setup and initialization routines.
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
// stdlibs
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "ad9528.h"

// platform drivers
#include "spi.h"
#include "spi_extra.h"
#include "gpio.h"
#include "gpio_extra.h"
#include "error.h"
#include "delay.h"
#include "util.h"

// hal
#include "parameters.h"
//#include "adi_hal.h"

// header
#include "app_clocking.h"

#define FPGA_SYSREF_CLK	0
#define FPGA_GLBL_CLK	1
#define FPGA_REF_CLK	3
#define ADC_SYSREF_CLK	12
#define ADC_REF_CLK	13

struct ad9528_dev *clkchip_device;
struct axi_clkgen *rx_clkgen;
struct axi_clkgen *tx_clkgen;
struct axi_clkgen *rx_os_clkgen;
extern uint64_t clk_hz[][3];
int32_t app_clocking_init(uint8_t uc)
{
	int32_t status;
	uint64_t dev_ref_clk, fpga_ref_clk, fpga_glb_clk;
	int ret;

	struct ad9528_channel_spec ad9528_channels[14];
	struct ad9528_init_param ad9528_param;
	struct ad9528_platform_data ad9528_pdata;

	struct xil_gpio_init_param  xil_gpio_param = {
		.type = GPIO_PS,
		.device_id = GPIO_DEVICE_ID
	};
	struct gpio_init_param	gpio_phy_ref_sel = {
		.number = AD9528_REFSEL,
		.platform_ops = &xil_gpio_platform_ops,
		.extra = &xil_gpio_param
	};

	// ad9528 defaults
	ad9528_param.pdata = &ad9528_pdata;
	ad9528_param.pdata->num_channels = 14;
	ad9528_param.pdata->channels = &ad9528_channels[0];

	status = ad9528_init(&ad9528_param);
	if(status) {
		printf("error: ad9528_init() failed with %d\n", status);
		goto error_0;
	}

	// ad9528 channel defaults
	for(unsigned int ch = 0; ch < ad9528_param.pdata->num_channels; ch++) {
		ad9528_channels[ch].channel_num = ch;
		ad9528_channels[ch].output_dis = 1;
	}

	// ad9528 channel specifics

	// SYSREF to FPGA
	ad9528_channels[0].output_dis = 0;
	ad9528_channels[0].driver_mode = DRIVER_MODE_LVDS;
	ad9528_channels[0].divider_phase = 0;
	ad9528_channels[0].signal_source = SOURCE_SYSREF_VCO;

	// GLBLCLK to FPGA
	ad9528_channels[1].output_dis = 0;
	ad9528_channels[1].driver_mode = DRIVER_MODE_LVDS;
	ad9528_channels[1].divider_phase = 0;
	ad9528_channels[1].signal_source = SOURCE_VCO;

	// REFCLK to FPGA
	ad9528_channels[3].output_dis = 0;
	ad9528_channels[3].driver_mode = DRIVER_MODE_LVDS;
	ad9528_channels[3].divider_phase = 0;
	ad9528_channels[3].signal_source = SOURCE_VCO;

	// SYSREF to ADC
	ad9528_channels[12].output_dis = 0;
	ad9528_channels[12].driver_mode = DRIVER_MODE_LVDS;
	ad9528_channels[12].divider_phase = 0;
	ad9528_channels[12].signal_source = SOURCE_SYSREF_VCO;

	// ADC CLK reference, used for ADC sample clock and JESD
	ad9528_channels[13].output_dis = 0;
	ad9528_channels[13].driver_mode = DRIVER_MODE_LVDS;
	ad9528_channels[13].divider_phase = 0;
	ad9528_channels[13].signal_source = SOURCE_VCO;

	// ad9528 settings
	ad9528_param.pdata->spi3wire = 1;
	ad9528_param.pdata->vcxo_freq = 100000000;
	ad9528_param.pdata->refa_en = 1;
	ad9528_param.pdata->refa_diff_rcv_en = 1;
	ad9528_param.pdata->refa_r_div = 1;
	ad9528_param.pdata->osc_in_cmos_neg_inp_en = 1;
	ad9528_param.pdata->pll1_feedback_div = 4;
	ad9528_param.pdata->pll1_feedback_src_vcxo = 0; /* VCO */
	ad9528_param.pdata->pll1_charge_pump_current_nA = 5000;
	ad9528_param.pdata->pll1_bypass_en = 0;
	ad9528_param.pdata->pll2_vco_div_m1 = 4;
	ad9528_param.pdata->pll2_n2_div = 10;
	ad9528_param.pdata->pll2_r1_div = 1;
	ad9528_param.pdata->pll2_charge_pump_current_nA = 805000;
	ad9528_param.pdata->pll2_bypass_en = false;
	ad9528_param.pdata->sysref_src = SYSREF_SRC_INTERNAL;
	ad9528_param.pdata->sysref_pattern_mode = SYSREF_PATTERN_CONTINUOUS;
	ad9528_param.pdata->sysref_req_en = true;
	ad9528_param.pdata->sysref_nshot_mode = SYSREF_NSHOT_4_PULSES;
	ad9528_param.pdata->sysref_req_trigger_mode = SYSREF_LEVEL_HIGH;
	ad9528_param.pdata->rpole2 = RPOLE2_900_OHM;
	ad9528_param.pdata->rzero = RZERO_1850_OHM;
	ad9528_param.pdata->cpole1 = CPOLE1_16_PF;
	/* Status pin function selection */
	ad9528_param.pdata->stat0_pin_func_sel = 0xFF; /* No funtion */
	ad9528_param.pdata->stat1_pin_func_sel = 0xFF; /* No function */

	struct xil_spi_init_param xil_spi_param = {
		.type = SPI_PS,
		.device_id = 0,
	};

	// clock chip spi settings
	struct spi_init_param clkchip_spi_init_param = {
		.max_speed_hz = 10000000,
		.mode = SPI_MODE_0,
		.chip_select = CLK_AD9258_CS,
		.platform_ops = &xil_platform_ops,
		.extra = &xil_spi_param
	};

	ad9528_param.spi_init = clkchip_spi_init_param;

	/* Reset pin is tied up to P3V3_CLK_PLL trough a pull up resistor */
	ad9528_param.gpio_resetb = NULL;


	/** < Insert User System Clock(s) Initialization Code Here >
	* System Clock should provide a device clock and SYSREF signal
	* to the Talise device.
	**/
	status = ad9528_setup(&clkchip_device, ad9528_param);
	if(status < 0) {
		printf("error: ad9528_setup() failed with %d\n", status);
		goto error_1;
	}

	fpga_glb_clk = ad9528_clk_round_rate(clkchip_device, FPGA_GLBL_CLK, clk_hz[uc][1]);
	fpga_ref_clk = ad9528_clk_round_rate(clkchip_device, FPGA_REF_CLK, clk_hz[uc][1]);
	dev_ref_clk = ad9528_clk_round_rate(clkchip_device, ADC_REF_CLK, clk_hz[uc][0]);


	ad9528_clk_set_rate(clkchip_device, FPGA_GLBL_CLK, fpga_glb_clk);
	ad9528_clk_set_rate(clkchip_device, FPGA_REF_CLK, fpga_ref_clk);
	ad9528_clk_set_rate(clkchip_device, ADC_REF_CLK, dev_ref_clk);

	ad9528_clk_set_rate(clkchip_device, FPGA_SYSREF_CLK, clk_hz[uc][1]/64);
	ad9528_clk_set_rate(clkchip_device, ADC_SYSREF_CLK, clk_hz[uc][1]/64);

	return SUCCESS;

error_1:
	ad9528_remove(clkchip_device);
error_0:
	return FAILURE;
}

void app_clocking_deinit(void)
{
	ad9528_remove(clkchip_device);
}
