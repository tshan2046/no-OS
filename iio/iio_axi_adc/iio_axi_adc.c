/***************************************************************************//**
 *   @file   iio_axi_adc.c
 *   @brief  Implementation of iio_axi_adc.
 *   This module is a wrapper over "axi_adc", and it is used by "iio".
 *   @author Cristian Pop (cristian.pop@analog.com)
********************************************************************************
 * Copyright 2019(c) Analog Devices, Inc.
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

#include <inttypes.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include "iio_axi_adc.h"
#include "error.h"
#include "axi_adc_core.h"
#include "axi_dmac.h"
#include "xml.h"
#include "util.h"

/******************************************************************************/
/************************ Functions Definitions *******************************/
/******************************************************************************/

/**
 * @brief Init and create iio_axi_adc.
 * @param iio_axi_adc - Pointer to iio_axi_adc.
 * @param init - Init parameters.
 * @return SUCCESS in case of success or negative value otherwise.
 */
ssize_t iio_axi_adc_init(struct iio_axi_adc **iio_axi_adc,
			 struct iio_axi_adc_init_par *init)
{
	struct iio_axi_adc *iio_adc = (struct iio_axi_adc *)calloc(1,
				      sizeof(struct iio_axi_adc));

	if (!(iio_adc))
		return FAILURE;

	iio_adc->adc = init->adc;
	iio_adc->dmac = init->dmac;
	iio_adc->adc_ddr_base = init->adc_ddr_base;
	iio_adc->dcache_invalidate_range = init->dcache_invalidate_range;

	*iio_axi_adc = iio_adc;

	return SUCCESS;
}

/**
 * @brief Free the resources allocated by iio_axi_adc_init().
 * @param iio_axi_adc - Pointer to iio_axi_adc.
 * @return SUCCESS in case of success or negative value otherwise.
 */
ssize_t iio_axi_adc_remove(struct iio_axi_adc *iio_axi_adc)
{
	free(iio_axi_adc);

	return SUCCESS;
}

/**
 * @brief get_cf_calibphase.
 * @param *device - Physical instance of a iio_axi_adc device.
 * @param *buf - Where value is stored.
 * @param len - Maximum length of value to be stored in buf.
 * @param *channel - Channel properties.
 * @return length of chars written in buf, or negative value on failure.
 */
static ssize_t get_calibphase(void *device, char *buf, size_t len,
			      const struct iio_ch_info *channel)
{
	int32_t val, val2;
	int32_t i = 0;
	struct iio_axi_adc *iio_adc = (struct iio_axi_adc *)device;
	ssize_t ret = axi_adc_get_calib_phase(iio_adc->adc, channel->ch_num, &val,
					      &val2);
	if (ret < 0)
		return ret;

	if (val2 < 0 && val >= 0) {
		snprintf(buf, len, "-");
		i++;
	}

	return i + snprintf(&buf[i], len, "%"PRIi32".%.6"PRIi32"", val, labs(val2));
}

/**
 * @brief get_cf_calibbias.
 * @param *device - Physical instance of a iio_axi_adc device.
 * @param *buf - Where value is stored.
 * @param len - Maximum length of value to be stored in buf.
 * @param *channel - Channel properties.
 * @return length of chars written in buf, or negative value on failure.
 */
static ssize_t get_calibbias(void *device, char *buf, size_t len,
			     const struct iio_ch_info *channel)
{
	int32_t val;
	ssize_t ret;
	struct iio_axi_adc *iio_adc = (struct iio_axi_adc *)device;

	ret = axi_adc_get_calib_bias(iio_adc->adc,
				     channel->ch_num,
				     &val,
				     NULL);
	if (ret < 0)
		return ret;

	return snprintf(buf, len, "%"PRIi32"", val);
}

/**
 * @brief get_cf_calibscale.
 * @param *device - Physical instance of a iio_axi_adc device.
 * @param *buf - Where value is stored.
 * @param len - Maximum length of value to be stored in buf.
 * @param *channel - Channel properties.
 * @return length of chars written in buf, or negative value on failure.
 */
static ssize_t get_calibscale(void *device, char *buf, size_t len,
			      const struct iio_ch_info *channel)
{
	int32_t val, val2;
	int32_t i = 0;
	struct iio_axi_adc *iio_adc = (struct iio_axi_adc *)device;
	ssize_t ret = axi_adc_get_calib_scale(iio_adc->adc, channel->ch_num, &val,
					      &val2);
	if (ret < 0)
		return ret;

	if (val2 < 0 && val >= 0) {
		ret = snprintf(buf, len, "-");
		if (ret < 0)
			return ret;
		i++;
	}
	ret = i + snprintf(&buf[i], len, "%"PRIi32".%.6"PRIi32"", val,
			   labs(val2));

	return ret;
}

/**
 * @brief get_cf_samples_pps.
 * @param *device - Physical instance of a iio_axi_adc device.
 * @param *buf - Where value is stored.
 * @param len - Maximum length of value to be stored in buf.
 * @param *channel - Channel properties.
 * @return length of chars written in buf, or negative value on failure.
 */
static ssize_t get_samples_pps(void *device, char *buf, size_t len,
			       const struct iio_ch_info *channel)
{
	/* This function doesn't have an equivalent function in axi_adc_core,
	 * and it should be implemented there first */

	return -ENOENT;
}

/**
 * @brief get_cf_sampling_frequency.
 * @param *device - Physical instance of a iio_axi_adc device.
 * @param *buf - Where value is stored.
 * @param len - Maximum length of value to be stored in buf.
 * @param *channel - Channel properties.
 * @return length of chars written in buf, or negative value on failure.
 */
static ssize_t get_sampling_frequency(void *device, char *buf, size_t len,
				      const struct iio_ch_info *channel)
{
	uint64_t sampling_freq_hz;
	struct iio_axi_adc *iio_adc = (struct iio_axi_adc *)device;
	ssize_t ret = axi_adc_get_sampling_freq(iio_adc->adc, channel->ch_num,
						&sampling_freq_hz);
	if (ret < 0)
		return ret;

	return snprintf(buf, len, "%"PRIi64"", sampling_freq_hz);
}

/**
 * @brief set_cf_calibphase.
 * @param *device - Physical instance of a iio_axi_dac device.
 * @param *buf - Value to be written to attribute.
 * @param len - Length of the data in "buf".
 * @param *channel - Channel properties.
 * @return number of bytes written to device, or negative value on failure.
 */
static ssize_t set_calibphase(void *device, char *buf, size_t len,
			      const struct iio_ch_info *channel)
{
	ssize_t ret;
	float calib = strtof(buf, NULL);
	int32_t val = (int32_t)calib;
	int32_t val2 = (int32_t)(calib * 1000000) % 1000000;
	struct iio_axi_adc *iio_adc = (struct iio_axi_adc *)device;

	ret = axi_adc_set_calib_phase(iio_adc->adc, channel->ch_num, val, val2);
	if (ret < 0)
		return ret;

	return len;
}

/**
 * @brief set_cf_calibbias.
 * @param *device - Physical instance of a iio_axi_dac device.
 * @param *buf - Value to be written to attribute.
 * @param len - Length of the data in "buf".
 * @param *channel - Channel properties.
 * @return number of bytes written to device, or negative value on failure.
 */
static ssize_t set_calibbias(void *device, char *buf, size_t len,
			     const struct iio_ch_info *channel)
{
	int32_t val = str_to_int32(buf);
	struct iio_axi_adc *iio_adc = (struct iio_axi_adc *)device;
	ssize_t ret;

	ret = axi_adc_set_calib_bias(iio_adc->adc,
				     channel->ch_num,
				     val,
				     0);
	if (ret < 0)
		return ret;

	return len;
}

/**
 * @brief set_cf_calibscale.
 * @param *device - Physical instance of a iio_axi_dac device.
 * @param *buf - Value to be written to attribute.
 * @param len - Length of the data in "buf".
 * @param *channel - Channel properties.
 * @return number of bytes written to device, or negative value on failure.
 */
static ssize_t set_calibscale(void *device, char *buf, size_t len,
			      const struct iio_ch_info *channel)
{
	float calib= strtof(buf, NULL);
	int32_t val = (int32_t)calib;
	int32_t val2 = (int32_t)(calib* 1000000) % 1000000;
	struct iio_axi_adc *iio_adc = (struct iio_axi_adc *)device;
	ssize_t ret = axi_adc_set_calib_scale(iio_adc->adc, channel->ch_num, val,
					      val2);
	if (ret < 0)
		return ret;

	return len;
}

/**
 * @brief set_cf_samples_pps.
 * @param *device - Physical instance of a iio_axi_dac device.
 * @param *buf - Value to be written to attribute.
 * @param len - Length of the data in "buf".
 * @param *channel - Channel properties.
 * @return number of bytes written to device, or negative value on failure.
 */
static ssize_t set_samples_pps(void *device, char *buf, size_t len,
			       const struct iio_ch_info *channel)
{
	/* This function doesn't have an equivalent function in axi_adc_core,
	 * and it should be implemented there first */

	return -ENOENT;
}

/**
 * @brief set_cf_sampling_frequency.
 * @param *device - Physical instance of a iio_axi_dac device.
 * @param *buf - Value to be written to attribute.
 * @param len - Length of the data in "buf".
 * @param *channel - Channel properties.
 * @return number of bytes written to device, or negative value on failure.
 */
static ssize_t set_sampling_frequency(void *device, char *buf, size_t len,
				      const struct iio_ch_info *channel)
{
	/* This function doesn't have an equivalent function in axi_adc_core,
	 * and it should be implemented there first */

	return -ENOENT;
}

static struct iio_attribute iio_attr_calibphase = {
	.name = "calibphase",
	.show = get_calibphase,
	.store = set_calibphase,
};

static struct iio_attribute iio_attr_calibbias = {
	.name = "calibbias",
	.show = get_calibbias,
	.store = set_calibbias,
};

static struct iio_attribute iio_attr_calibscale = {
	.name = "calibscale",
	.show = get_calibscale,
	.store = set_calibscale,
};

static struct iio_attribute iio_attr_samples_pps = {
	.name = "samples_pps",
	.show = get_samples_pps,
	.store = set_samples_pps,
};

static struct iio_attribute iio_attr_sampling_frequency = {
	.name = "sampling_frequency",
	.show = get_sampling_frequency,
	.store = set_sampling_frequency,
};

static struct iio_attribute *iio_voltage_attributes[] = {
	&iio_attr_calibphase,
	&iio_attr_calibbias,
	&iio_attr_calibscale,
	&iio_attr_samples_pps,
	&iio_attr_sampling_frequency,
	NULL,
};

/**
 * @brief Get an axi_adc xml.
 * @param **xml - Xml containing description of a device.
 * @param *iio_dev - Structure describing a device, channels and attributes.
 * @return SUCCESS in case of success or negative value otherwise.
 */
ssize_t iio_axi_adc_get_xml(char** xml, struct iio_device *iio_dev)
{
	struct xml_document *document = NULL;
	struct xml_node *attribute = NULL;
	struct xml_attribute *att = NULL;
	struct xml_node *channel = NULL;
	struct xml_node *device = NULL;
	char buff[256];
	ssize_t ret;
	uint16_t i;

	if (!xml)
		return FAILURE;
	if (!iio_dev)
		return FAILURE;

	ret = xml_create_node(&device, "device");
	if (ret < 0)
		goto error;
	ret = xml_create_attribute(&att, "id", iio_dev->name);
	if (ret < 0)
		goto error;
	ret = xml_add_attribute(device, att);
	if (ret < 0)
		goto error;
	ret = xml_create_attribute(&att, "name", iio_dev->name);
	if (ret < 0)
		goto error;
	ret = xml_add_attribute(device, att);
	if (ret < 0)
		goto error;

	for (i = 0; i < iio_dev->num_ch; i++) {
		ret = xml_create_node(&channel, "channel");
		if (ret < 0)
			goto error;
		ret = xml_create_attribute(&att, "id", iio_dev->channels[i]->name);
		if (ret < 0)
			goto error;
		ret = xml_add_attribute(channel, att);
		if (ret < 0)
			goto error;
		ret = xml_create_attribute(&att, "type", "input");
		if (ret < 0)
			goto error;
		ret = xml_add_attribute(channel, att);
		if (ret < 0)
			goto error;

		ret = xml_create_node(&attribute, "scan-element");
		if (ret < 0)
			goto error;
		sprintf(buff, "%d", i);
		ret = xml_create_attribute(&att, "index", buff);
		if (ret < 0)
			goto error;
		ret = xml_add_attribute(attribute, att);
		if (ret < 0)
			goto error;
		ret = xml_create_attribute(&att, "format", "le:S16/16&gt;&gt;0");
		if (ret < 0)
			goto error;
		ret = xml_add_attribute(attribute, att);
		if (ret < 0)
			goto error;
		ret = xml_add_node(channel, attribute);
		if (ret < 0)
			goto error;

		for (uint8_t j = 0; iio_voltage_attributes[j] != NULL; j++) {
			ret = xml_create_node(&attribute, "attribute");
			if (ret < 0)
				goto error;
			ret = xml_create_attribute(&att, "name",
						   iio_voltage_attributes[j]->name);
			if (ret < 0)
				goto error;
			ret = xml_add_attribute(attribute, att);
			if (ret < 0)
				goto error;
			sprintf(buff, "in_voltage%d_%s", i, iio_voltage_attributes[j]->name);
			ret = xml_create_attribute(&att, "filename", buff);
			if (ret < 0)
				goto error;
			ret = xml_add_attribute(attribute, att);
			if (ret < 0)
				goto error;
			ret = xml_add_node(channel, attribute);
			if (ret < 0)
				goto error;
		}
		ret = xml_add_node(device, channel);
		if (ret < 0)
			goto error;
	}

	ret = xml_create_document(&document, device);
	if (ret < 0) {
		if (document)
			xml_delete_document(document);
		goto error;
	}
	*xml = document->buff;

error:
	if (device)
		xml_delete_node(device);

	return ret;
}

/**
 * Delete iio_device.
 * @param *iio_device - Structure describing a device, channels and attributes.
 * @return iio_device or NULL, in case of failure.
 */
ssize_t iio_axi_adc_delete_device(struct iio_device *iio_device)
{
	uint16_t i = 0;

	if (!iio_device)
		return SUCCESS;

	if (iio_device->channels) {
		while (iio_device->channels[i]) {
			if (iio_device->channels[i]->name)
				free(iio_device->channels[i]->name);
			if (iio_device->channels[i])
				free(iio_device->channels[i]);
			i++;
		}
		free(iio_device->channels);
	}
	if (iio_device)
		free(iio_device);

	return SUCCESS;
}

/**
 * Create structure describing a device, channels and attributes.
 * @param *device - Device name.
 * @param num_ch - Number of channels that the device has.
 * @return iio_device or NULL, in case of failure.
 */
struct iio_device *iio_axi_adc_create_device(const char *device_name,
		uint16_t num_ch)
{
	struct iio_device *iio_device;
	const uint8_t num_ch_digits = 3;
	char ch_voltage[] = "voltage";
	uint16_t i, len;
	ssize_t ret;

	if (!device_name)
		return NULL;

	iio_device = (struct iio_device *)calloc(1, sizeof(struct iio_device));
	if (!iio_device)
		return NULL;

	iio_device->name = device_name;
	iio_device->num_ch = num_ch;
	iio_device->attributes = NULL; /* no device attribute */
	iio_device->channels = calloc(num_ch + 1, sizeof(struct iio_channel *));
	if (!iio_device->channels)
		goto error;

	for (i = 0; i < num_ch; i++) {
		iio_device->channels[i] = calloc(1, sizeof(struct iio_channel));
		if (!iio_device->channels[i])
			goto error;
		len = strlen(ch_voltage) + num_ch_digits + 1;
		iio_device->channels[i]->name = calloc(1, len);
		if (!iio_device->channels[i]->name)
			goto error;
		ret = sprintf(iio_device->channels[i]->name, "%s%d", ch_voltage, i);
		if (ret < 0)
			goto error;
		iio_device->channels[i]->attributes = iio_voltage_attributes;
	}

	iio_device->channels[i] = NULL;

	return iio_device;

error:
	iio_axi_adc_delete_device(iio_device);

	return NULL;
}

/**
 * @brief Transfer data from ADC into RAM.
 * @param *iio_inst - Physical instance of a iio_axi_adc device.
 * @param bytes_count - Number of bytes to transfer.
 * @param ch_mask - Opened channels mask.
 * @return bytes_count or negative value in case of error.
 */
ssize_t iio_axi_adc_transfer_dev_to_mem(void *iio_inst, size_t bytes_count,
					uint32_t ch_mask)
{
	struct iio_axi_adc *iio_adc;
	ssize_t ret, bytes;

	if (!iio_inst)
		return FAILURE;

	iio_adc = (struct iio_axi_adc *)iio_inst;
	bytes = (bytes_count * iio_adc->adc->num_channels) / hweight8(ch_mask);

	iio_adc->dmac->flags = 0;
	ret = axi_dmac_transfer(iio_adc->dmac,
				iio_adc->adc_ddr_base, bytes);
	if (ret < 0)
		return ret;

	if (iio_adc->dcache_invalidate_range)
		iio_adc->dcache_invalidate_range(iio_adc->adc_ddr_base, bytes);

	return bytes_count;
}

/**
 * @brief Read chunk of data from RAM to pbuf.
 * Call "iio_axi_adc_transfer_dev_to_mem" first.
 * This function is probably called multiple times by libtinyiiod after a
 * "iio_axi_adc_transfer_dev_to_mem" call, since we can only read "bytes_count"
 * bytes.
 * @param *iio_inst - Physical instance of a iio_axi_adc device.
 * @param *pbuf - Buffer where value is stored.
 * @param *offset - Offset to the remaining data after reading n chunks.
 * @param bytes_count - Number of bytes to read.
 * @param ch_mask - Opened channels mask.
 * @return bytes_count or negative value in case of error.
 */
ssize_t iio_axi_adc_read_dev(void *iio_inst, char *pbuf, size_t offset,
			     size_t bytes_count, uint32_t ch_mask)
{
	struct iio_axi_adc *iio_adc;
	uint32_t i, j = 0, current_ch = 0;
	uint16_t *pbuf16;
	size_t samples;

	if (!iio_inst)
		return FAILURE;

	if (!pbuf)
		return FAILURE;

	iio_adc = (struct iio_axi_adc *)iio_inst;
	pbuf16 = (uint16_t*)pbuf;
	samples = (bytes_count * iio_adc->adc->num_channels) / hweight8(
			  ch_mask);

	samples /= 2; /* because of uint16_t *pbuf16 = (uint16_t*)pbuf; */
	offset = (offset * iio_adc->adc->num_channels) / hweight8(ch_mask);

	for (i = 0; i < samples; i++) {

		if (ch_mask & BIT(current_ch)) {
			pbuf16[j] = *(uint16_t*)(iio_adc->adc_ddr_base + offset + i * 2);
			j++;
		}

		if (current_ch < iio_adc->adc->num_channels - 1)
			current_ch++;
		else
			current_ch = 0;
	}

	return bytes_count;
}
