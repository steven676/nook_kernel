/*
 * omap-hdmi.c  --  OMAP ALSA SoC DAI driver for HDMI audio
 *
 * Copyright (C) 2009 Texas Instruments
 *
 * Contact: Jorge Candelaria <x0107209@ti.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301 USA
 *
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <sound/core.h>
#include <sound/pcm.h>
#include <sound/pcm_params.h>
#include <sound/initval.h>
#include <sound/soc.h>

#include <plat/control.h>
#include <plat/dma.h>
#include "omap-pcm.h"
#include "omap-hdmi.h"
#include <plat/hdmi_lib.h>

#define OMAP_HDMI_RATES	(SNDRV_PCM_RATE_48000)
#define OMAP_HDMI_FORMATS (SNDRV_PCM_FMTBIT_S16_LE | SNDRV_PCM_FMTBIT_S32_LE)

static struct omap_pcm_dma_data omap_hdmi_dai_dma_params = {
	.name = "HDMI playback",
	.dma_req = OMAP44XX_DMA_DSS_HDMI_REQ,
	.port_addr = HDMI_WP + HDMI_WP_AUDIO_DATA,
	.sync_mode = OMAP_DMA_SYNC_PACKET,
};

static int omap_hdmi_dai_startup(struct snd_pcm_substream *substream,
				  struct snd_soc_dai *dai)
{
	int err = 0;

	err = HDMI_W1_WrapperEnable(HDMI_WP);

	return err;
}

static void omap_hdmi_dai_shutdown(struct snd_pcm_substream *substream,
				    struct snd_soc_dai *dai)
{
	int err = 0;

	err = HDMI_W1_WrapperDisable(HDMI_WP);
}

static int omap_hdmi_dai_trigger(struct snd_pcm_substream *substream, int cmd,
				  struct snd_soc_dai *dai)
{
	int err = 0;

	switch (cmd) {
	case SNDRV_PCM_TRIGGER_START:
	case SNDRV_PCM_TRIGGER_RESUME:
	case SNDRV_PCM_TRIGGER_PAUSE_RELEASE:
		err = HDMI_W1_StartAudioTransfer(HDMI_WP);
		break;

	case SNDRV_PCM_TRIGGER_STOP:
	case SNDRV_PCM_TRIGGER_SUSPEND:
	case SNDRV_PCM_TRIGGER_PAUSE_PUSH:
		err = HDMI_W1_StopAudioTransfer(HDMI_WP);
		break;
	default:
		err = -EINVAL;
	}

	return err;
}

static int omap_hdmi_dai_hw_params(struct snd_pcm_substream *substream,
				    struct snd_pcm_hw_params *params,
				    struct snd_soc_dai *dai)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct snd_soc_dai *cpu_dai = rtd->dai->cpu_dai;
	int err = 0;

	switch (params_format(params)) {
	case SNDRV_PCM_FORMAT_S16_LE:
	case SNDRV_PCM_FORMAT_S32_LE:
		omap_hdmi_dai_dma_params.data_type = OMAP_DMA_DATA_TYPE_S32;
		break;

	default:
		err = -EINVAL;
	}
	omap_hdmi_dai_dma_params.packet_size = 0x20;

	cpu_dai->dma_data = &omap_hdmi_dai_dma_params;

	return err;
}

static struct snd_soc_dai_ops omap_hdmi_dai_ops = {
	.startup	= omap_hdmi_dai_startup,
	.shutdown	= omap_hdmi_dai_shutdown,
	.trigger	= omap_hdmi_dai_trigger,
	.hw_params	= omap_hdmi_dai_hw_params,
};

struct snd_soc_dai omap_hdmi_dai = {
	.name = "omap-hdmi-dai",
	.id = -1,
	.playback = {
		.channels_min = 2,
		.channels_max = 8,
		.rates = OMAP_HDMI_RATES,
		.formats = OMAP_HDMI_FORMATS,
	},
	.ops = &omap_hdmi_dai_ops,
};
EXPORT_SYMBOL_GPL(omap_hdmi_dai);

static int __init snd_omap_hdmi_init(void)
{
	return snd_soc_register_dai(&omap_hdmi_dai);
}
module_init(snd_omap_hdmi_init);

static void __exit snd_omap_hdmi_exit(void)
{
	snd_soc_unregister_dai(&omap_hdmi_dai);
}
module_exit(snd_omap_hdmi_exit);

MODULE_AUTHOR("Jorge Candelaria <x0107209@ti.com");
MODULE_DESCRIPTION("OMAP HDMI SoC Interface");
MODULE_LICENSE("GPL");