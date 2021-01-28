/***************************************************************************//**
 * @file adc_demo.c
 * @brief Implementation of ADC Demo Driver.
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

/******************************************************************************/
/***************************** Include Files **********************************/
/******************************************************************************/

#include <stdlib.h>
#include "adc_demo.h"
#include "error.h"
#include "util.h"

/******************************************************************************/
/************************ Functions Definitions *******************************/
/******************************************************************************/

/***************************************************************************//**
 * @brief init function for the adc demo driver
 * @param desc - descriptor for the adc
 * @param param - initialization param for adc
 * @return SUCCESS in case of success, negative error code otherwise.
*******************************************************************************/
int32_t adc_demo_init(struct adc_demo_desc **desc,
		      struct adc_demo_init_param *param)
{
	struct adc_demo_desc *adesc;
	adesc = (struct adc_demo_desc*)calloc(1, sizeof(*adesc));

	if(!adesc)
		return -ENOMEM;

	*desc = adesc;

	return SUCCESS;
}

/***************************************************************************//**
 * @brief free allocated resources
 * @param desc - descriptor for the adc
 * @return SUCCESS in case of success, FAILURE otherwise.
*******************************************************************************/
int32_t adc_demo_remove(struct adc_demo_desc *desc)
{
	if(!desc)
		return -EINVAL;

	free(desc);

	return SUCCESS;
}

/***************************************************************************//**
 * @brief read function for the adc demo driver
 * @param desc - descriptor for the adc
 * @param reg_index - the address at which we want to read
 * @param readval- the value read from register
 * @return SUCCESS in case of success, negative error code otherwise.
*******************************************************************************/
int32_t adc_demo_reg_read(struct adc_demo_desc *desc, uint8_t reg_index,
			  uint8_t *readval)
{
	if(!desc || reg_index >= ARRAY_SIZE(desc->reg))
		return -EINVAL;

	*readval =  desc->reg[reg_index];

	return SUCCESS;
}

/***************************************************************************//**
 * @brief write function for the adc demo driver
 * @param desc - descriptor for the adc
 * @param reg_index - the address at which we want to write
 * @param writeval - the value to be written
 * @return SUCCESS in case of success, negative error code otherwise.
*******************************************************************************/
int32_t adc_demo_reg_write(struct adc_demo_desc *desc, uint8_t reg_index,
			   uint8_t writeval)
{

	if(!desc || reg_index >= ARRAY_SIZE(desc->reg))
		return -EINVAL;

	desc->reg[reg_index] = writeval;

	return SUCCESS;
}
