/***************************************************************************//**
 * @file adc_demo.h
 * @brief Header file of ADC Demo Driver.
 * @author RNechita (ramona.nechita@analog.com)
 ********************************************************************************
 * Copyright 2021(c) Analog Devices, Inc.
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * - Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 * - Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in
 * the documentation and/or other materials provided with the
 * distribution.
 * - Neither the name of Analog Devices, Inc. nor the names of its
 * contributors may be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * - The use of this software may or may not infringe the patent rights
 * of one or more patent holders. This license does not release you
 * from the requirement that you obtain separate licenses from these
 * patent holders to use this software.
 * - Use of the software either in source or binary form, must be run
 * on or directly connected to an Analog Devices Inc. component.
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

#ifndef IIO_DEMO_ADC_
#define IIO_DEMO_ADC_

/******************************************************************************/
/***************************** Include Files **********************************/
/******************************************************************************/

#include <stdint.h>

/******************************************************************************/
/*************************** Types Declarations *******************************/
/******************************************************************************/

#define MAX_REG_ADDR		16
#define DEFAULT_LOCAL_SAMPLES 128
#define SINE_VECTOR_SIZE 128

/**
 * @struct iio_demo_adc_desc
 * @brief Desciptor.
 */
struct adc_demo_desc {
	/** Dummy registers of device for testing */
	uint8_t reg[MAX_REG_ADDR];
	/** Active channel**/
	uint32_t active_ch;
	/** Vector for communication between adc&dac */
	uint16_t *loopback;
};

/**
 * @struct iio_demo_adc_init_param
 * @brief iio demo adc configuration.
 */
struct adc_demo_init_param {
	/**Vector for communication between adc&dac*/
	uint16_t *loopback;
};

/******************************************************************************/
/************************ Functions Declarations ******************************/
/******************************************************************************/

int32_t adc_demo_init(struct adc_demo_desc **desc,
		      struct adc_demo_init_param *param);

int32_t adc_demo_remove(struct adc_demo_desc *desc);


int32_t update_active_adc_channels(void *dev, int32_t mask);

int32_t close_adc_channels(void* dev);

int32_t adc_read_samples(void* dev, uint16_t* buff, uint32_t samples);

int32_t adc_demo_reg_read(struct adc_demo_desc *desc, uint8_t reg_index,
			  uint8_t *readval);

int32_t adc_demo_reg_write(struct adc_demo_desc *desc, uint8_t reg_index,
			   uint8_t writeval);

#endif /*IIO_DEMO_ADC_*/
