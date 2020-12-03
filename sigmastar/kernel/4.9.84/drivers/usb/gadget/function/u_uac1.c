/*
 * u_uac1.c -- ALSA audio utilities for Gadget stack
 *
 * Copyright (C) 2008 Bryan Wu <cooloney@kernel.org>
 * Copyright (C) 2008 Analog Devices, Inc
 *
 * Enter bugs at http://blackfin.uclinux.org/
 *
 * Licensed under the GPL-2 or later.
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/device.h>
#include <linux/delay.h>
#include <linux/ctype.h>
#include <linux/random.h>
#include <linux/syscalls.h>

#include "u_uac1.h"

/*
 * This component encapsulates the ALSA devices for USB audio gadget
 */

/*-------------------------------------------------------------------------*/

/**
 * Some ALSA internal helper functions
 */
static int snd_interval_refine_set(struct snd_interval *i, unsigned int val)
{
	struct snd_interval t;
	t.empty = 0;
	t.min = t.max = val;
	t.openmin = t.openmax = 0;
	t.integer = 1;
	return snd_interval_refine(i, &t);
}

static int _snd_pcm_hw_param_set(struct snd_pcm_hw_params *params,
				 snd_pcm_hw_param_t var, unsigned int val,
				 int dir)
{
	int changed;
	if (hw_is_mask(var)) {
		struct snd_mask *m = hw_param_mask(params, var);
		if (val == 0 && dir < 0) {
			changed = -EINVAL;
			snd_mask_none(m);
		} else {
			if (dir > 0)
				val++;
			else if (dir < 0)
				val--;
			changed = snd_mask_refine_set(
					hw_param_mask(params, var), val);
		}
	} else if (hw_is_interval(var)) {
		struct snd_interval *i = hw_param_interval(params, var);
		if (val == 0 && dir < 0) {
			changed = -EINVAL;
			snd_interval_none(i);
		} else if (dir == 0)
			changed = snd_interval_refine_set(i, val);
		else {
			struct snd_interval t;
			t.openmin = 1;
			t.openmax = 1;
			t.empty = 0;
			t.integer = 0;
			if (dir < 0) {
				t.min = val - 1;
				t.max = val;
			} else {
				t.min = val;
				t.max = val+1;
			}
			changed = snd_interval_refine(i, &t);
		}
	} else
		return -EINVAL;
	if (changed) {
		params->cmask |= 1 << var;
		params->rmask |= 1 << var;
	}
	return changed;
}
/*-------------------------------------------------------------------------*/

#if defined(CONFIG_SS_GADGET) ||defined(CONFIG_SS_GADGET_MODULE)
/**
 * Set default hardware params
 */
static int capture_default_hw_params(struct gaudio_snd_dev *snd)
{
	struct snd_pcm_substream *substream = snd->substream;
	struct snd_pcm_hw_params *params;
	struct snd_pcm_sw_params sparams;
	snd_pcm_sframes_t result;
	struct f_uac1_opts	*audio_opts =
            container_of(snd->card->func.fi, struct f_uac1_opts, func_inst);

	/*
	 * SNDRV_PCM_ACCESS_RW_INTERLEAVED,
	 * SNDRV_PCM_FORMAT_S16_LE
	 * CHANNELS: 1
	 * RATE: 16000
	 */
	snd->access = SNDRV_PCM_ACCESS_RW_INTERLEAVED;
	snd->format = SNDRV_PCM_FORMAT_S16_LE;
	snd->channels = audio_opts->capture_channel_count;
	snd->rate = audio_opts->capture_sample_rate;
	snd->period_bytes = audio_opts->audio_capture_buf_size / 2;
	snd->buffer_bytes = audio_opts->audio_capture_buf_size;

	params = kzalloc(sizeof(*params), GFP_KERNEL);
	if (!params)
		return -ENOMEM;

	_snd_pcm_hw_params_any(params);
	_snd_pcm_hw_param_set(params, SNDRV_PCM_HW_PARAM_ACCESS,
			snd->access, 0);
	_snd_pcm_hw_param_set(params, SNDRV_PCM_HW_PARAM_FORMAT,
			snd->format, 0);
	_snd_pcm_hw_param_set(params, SNDRV_PCM_HW_PARAM_CHANNELS,
			snd->channels, 0);
	_snd_pcm_hw_param_set(params, SNDRV_PCM_HW_PARAM_RATE,
			snd->rate, 0);
	_snd_pcm_hw_param_set(params, SNDRV_PCM_HW_PARAM_PERIOD_BYTES,
			snd->period_bytes, 0);
	_snd_pcm_hw_param_set(params, SNDRV_PCM_HW_PARAM_BUFFER_BYTES,
			snd->buffer_bytes, 0);

	snd_pcm_kernel_ioctl(substream, SNDRV_PCM_IOCTL_DROP, NULL);
	snd_pcm_kernel_ioctl(substream, SNDRV_PCM_IOCTL_HW_PARAMS, params);

	/*sw params*/
	memset(&sparams, 0, sizeof(sparams));
	sparams.tstamp_mode = SNDRV_PCM_TSTAMP_ENABLE;
	sparams.period_step = 1;
	sparams.avail_min = 1;

	sparams.start_threshold = 1;
	/* pick a high stop threshold - todo: does this need further tuning */
	sparams.stop_threshold = snd->buffer_bytes * 5;
	sparams.silence_size = 0;
	sparams.silence_threshold = 0;
	sparams.boundary = snd->buffer_bytes / snd->format;
	snd_pcm_kernel_ioctl(substream, SNDRV_PCM_IOCTL_SW_PARAMS, &sparams);

	result = snd_pcm_kernel_ioctl(substream, SNDRV_PCM_IOCTL_PREPARE, NULL);
	if (result < 0) {
		ERROR(snd->card,
			"Preparing sound card failed: %d\n", (int)result);
		kfree(params);
		return result;
	}

	/* Store the hardware parameters */
	snd->access = params_access(params);
	snd->format = params_format(params);
	snd->channels = params_channels(params);
	snd->rate = params_rate(params);

	kfree(params);

	INFO(snd->card,
		"Capture Dev Hw params: access %x, format %x, channels %d, rate %d\n",
		snd->access, snd->format, snd->channels, snd->rate);

	return 0;
}
#endif
/**
 * Set default hardware params
 */
static int playback_default_hw_params(struct gaudio_snd_dev *snd)
{
	struct snd_pcm_substream *substream = snd->substream;
	struct snd_pcm_hw_params *params;
	snd_pcm_sframes_t result;
	struct f_uac1_opts	*audio_opts =
            container_of(snd->card->func.fi, struct f_uac1_opts, func_inst);

   /*
	* SNDRV_PCM_ACCESS_RW_INTERLEAVED,
	* SNDRV_PCM_FORMAT_S16_LE
	* CHANNELS: 1
	* RATE: 16000
	*/
	snd->access = SNDRV_PCM_ACCESS_RW_INTERLEAVED;
	snd->format = SNDRV_PCM_FORMAT_S16_LE;
	snd->channels = audio_opts->playback_channel_count;
	snd->rate = audio_opts->playback_sample_rate;
	snd->period_bytes = audio_opts->audio_playback_buf_size / 2;
	snd->buffer_bytes = audio_opts->audio_playback_buf_size;

	params = kzalloc(sizeof(*params), GFP_KERNEL);
	if (!params)
		return -ENOMEM;

	_snd_pcm_hw_params_any(params);
	_snd_pcm_hw_param_set(params, SNDRV_PCM_HW_PARAM_ACCESS,
			snd->access, 0);
	_snd_pcm_hw_param_set(params, SNDRV_PCM_HW_PARAM_FORMAT,
			snd->format, 0);
	_snd_pcm_hw_param_set(params, SNDRV_PCM_HW_PARAM_CHANNELS,
			snd->channels, 0);
	_snd_pcm_hw_param_set(params, SNDRV_PCM_HW_PARAM_RATE,
			snd->rate, 0);
	_snd_pcm_hw_param_set(params, SNDRV_PCM_HW_PARAM_PERIOD_BYTES,
			snd->period_bytes, 0);
	_snd_pcm_hw_param_set(params, SNDRV_PCM_HW_PARAM_BUFFER_BYTES,
			snd->buffer_bytes, 0);

	snd_pcm_kernel_ioctl(substream, SNDRV_PCM_IOCTL_DROP, NULL);
	snd_pcm_kernel_ioctl(substream, SNDRV_PCM_IOCTL_HW_PARAMS, params);

	result = snd_pcm_kernel_ioctl(substream, SNDRV_PCM_IOCTL_PREPARE, NULL);
	if (result < 0) {
		ERROR(snd->card,
			"Preparing sound card failed: %d\n", (int)result);
		kfree(params);
		return result;
	}

	/* Store the hardware parameters */
	snd->access = params_access(params);
	snd->format = params_format(params);
	snd->channels = params_channels(params);
	snd->rate = params_rate(params);

	kfree(params);

	INFO(snd->card,
		"Capture Dev Hw params: access %x, format %x, channels %d, rate %d\n",
		snd->access, snd->format, snd->channels, snd->rate);

	return 0;
}
/**
 * Capture audio buffer data by ALSA PCM device
 */
size_t u_audio_capture(struct gaudio *card, void *buf, size_t count)
{
	struct gaudio_snd_dev    *snd = &card->capture;
	struct snd_pcm_substream *substream = snd->substream;
	struct snd_pcm_runtime   *runtime = substream->runtime;
	mm_segment_t old_fs;
	ssize_t result, retry_count = 0;
	snd_pcm_sframes_t frames;
#ifdef debug_uac
	static loff_t f_pos = 0;
	struct file* fp = NULL;
	int rcount = 0;
#endif
try_again:
	if(retry_count++ > 0 && retry_count < 1000)
		msleep(30);
	else if(retry_count > 1000)
		return -EINVAL;

	if (runtime->status->state == SNDRV_PCM_STATE_XRUN ||
	    runtime->status->state == SNDRV_PCM_STATE_SUSPENDED ||
	    runtime->status->state == SNDRV_PCM_STATE_SETUP) {
		result = snd_pcm_kernel_ioctl(substream,
			SNDRV_PCM_IOCTL_PREPARE, NULL);
		if (result < 0) {
			pr_err("Preparing capture failed: %d\n",
				(int)result);
			return result;
		}
	}
	frames = bytes_to_frames(runtime, count);
	old_fs = get_fs();
	set_fs(KERNEL_DS);

	pr_debug("frames = %d, count = %zd \n", (int)frames, count);
#ifdef debug_uac
	fp = filp_open("/config/input.pcm",O_RDONLY ,0);
	if(IS_ERR(fp)) {
		pr_err("No such PCM File: %s\n", "/config/input.pcm");
		set_fs(old_fs);
		goto try_again;//do nothing
	}
	else {
		rcount = vfs_read(fp,buf,count,&f_pos);
		if(count > rcount)
			f_pos = 0;
		filp_close(fp,NULL);
		pr_debug("read count %d need count %d \n",rcount,count);
	}
#else
	result = snd_pcm_lib_read(substream, buf, frames);
	if (result != frames) {
		pr_info("Capture warring: %d count%d, state%d \n", (int)result, count, runtime->status->state);
		set_fs(old_fs);
		goto try_again;
	}
#endif
	set_fs(old_fs);
	return 0;
}

/**
 * Playback audio buffer data by ALSA PCM device
 */
size_t u_audio_playback(struct gaudio *card, void *buf, size_t count)
{
	struct gaudio_snd_dev	*snd = &card->playback;
	struct snd_pcm_substream *substream = snd->substream;
	struct snd_pcm_runtime *runtime = substream->runtime;
	mm_segment_t old_fs;
	ssize_t result;
	snd_pcm_sframes_t frames;

try_again:
	if (runtime->status->state == SNDRV_PCM_STATE_XRUN ||
		runtime->status->state == SNDRV_PCM_STATE_SUSPENDED) {
		result = snd_pcm_kernel_ioctl(substream,
				SNDRV_PCM_IOCTL_PREPARE, NULL);
		if (result < 0) {
			ERROR(card, "Preparing sound card failed: %d\n",
					(int)result);
			return result;
		}
	}

	frames = bytes_to_frames(runtime, count);
	old_fs = get_fs();
	set_fs(KERNEL_DS);
	pr_debug("frames = %d, count = %zd \n", (int)frames, count);

	result = snd_pcm_lib_write(snd->substream, (void __user *)buf, frames);
	if (result != frames) {
		pr_debug("Playback error: %d\n", (int)result);
		set_fs(old_fs);
		goto try_again;
	}
	set_fs(old_fs);

	return 0;
}

int u_audio_get_playback_channels(struct gaudio *card)
{
	return card->playback.channels;
}

int u_audio_get_playback_rate(struct gaudio *card)
{
	return card->playback.rate;
}

int u_audio_get_capture_channels(struct gaudio *card)
{
	return card->capture.channels;
}

int u_audio_get_capture_rate(struct gaudio *card)
{
	return card->capture.rate;
}

/**
 * Open ALSA PCM and control device files
 * Initial the PCM or control device
 */
static int gaudio_open_snd_dev(struct gaudio *card)
{
	struct snd_pcm_file *pcm_file;
	struct gaudio_snd_dev *snd;
	struct f_uac1_opts *opts;
	char *fn_play, *fn_cap, *fn_cntl;

	opts = container_of(card->func.fi, struct f_uac1_opts, func_inst);
	fn_play = opts->fn_play;
	fn_cap = opts->fn_cap;
	fn_cntl = opts->fn_cntl;

	/* Open control device */
	snd = &card->control;
	snd->filp = filp_open(fn_cntl, O_RDWR, 0);
	if (IS_ERR(snd->filp)) {
		int ret = PTR_ERR(snd->filp);
		ERROR(card, "unable to open sound control device file: %s\n",
				fn_cntl);
		snd->filp = NULL;
		return ret;
	}
	snd->card = card;

#if defined(CONFIG_SS_GADGET) ||defined(CONFIG_SS_GADGET_MODULE)
	/* Open PCM playback device and setup substream */
	if(ENABLE_SPEAKER == opts->audio_play_mode ||
	   ENABLE_MIC_AND_SPK == opts->audio_play_mode )
#endif
	{
		snd = &card->playback;
		snd->filp = filp_open(fn_play, O_WRONLY, 0);
		if (IS_ERR(snd->filp)) {
			int ret = PTR_ERR(snd->filp);
			ERROR(card, "No such PCM playback device: %s\n", fn_play);
			snd->substream = NULL;
			snd->card = NULL;
			snd->filp = NULL;
			return ret;
		} else
		{
			pcm_file = snd->filp->private_data;
			snd->substream = pcm_file->substream;
			snd->card = card;
			playback_default_hw_params(snd);
		}
	}

#if defined(CONFIG_SS_GADGET) ||defined(CONFIG_SS_GADGET_MODULE)
	/* Open PCM capture device and setup substream */
	if(ENABLE_MICROPHONE == opts->audio_play_mode ||
	   ENABLE_MIC_AND_SPK == opts->audio_play_mode )
	{
		snd = &card->capture;
		snd->filp = filp_open(fn_cap, O_RDONLY, 0);
		if (IS_ERR(snd->filp)) {
			int ret = PTR_ERR(snd->filp);
			ERROR(card, "No such PCM capture device: %s\n", fn_cap);
			snd->substream = NULL;
			snd->card = NULL;
			snd->filp = NULL;
			return ret;
		} else {
			pcm_file = snd->filp->private_data;
			snd->substream = pcm_file->substream;
			snd->card = card;
			capture_default_hw_params(snd);
		}
	}
#endif
	return 0;
}

/**
 * Close ALSA PCM and control device files
 */
static int gaudio_close_snd_dev(struct gaudio *gau)
{
	struct gaudio_snd_dev	*snd;
	struct f_uac1_opts *opts;

	opts = container_of(gau->func.fi, struct f_uac1_opts, func_inst);

	/* Close control device */
	snd = &gau->control;
	if (snd->filp)
		filp_close(snd->filp, NULL);

#if defined(CONFIG_SS_GADGET) ||defined(CONFIG_SS_GADGET_MODULE)
	/* Close PCM playback device and setup substream */
	if(ENABLE_SPEAKER == opts->audio_play_mode ||
	   ENABLE_MIC_AND_SPK == opts->audio_play_mode )
#endif
	{
		snd = &gau->playback;
		if (snd->filp)
			filp_close(snd->filp, NULL);
	}

#if defined(CONFIG_SS_GADGET) ||defined(CONFIG_SS_GADGET_MODULE)
	/* Close PCM capture device and setup substream */
	if(ENABLE_MICROPHONE == opts->audio_play_mode ||
	   ENABLE_MIC_AND_SPK == opts->audio_play_mode )
	{
		snd = &gau->capture;
		if (snd->filp)
			filp_close(snd->filp, NULL);
	}
#endif

	return 0;
}

/**
 * gaudio_setup - setup ALSA interface and preparing for USB transfer
 *
 * This sets up PCM, mixer or MIDI ALSA devices fore USB gadget using.
 *
 * Returns negative errno, or zero on success
 */
int gaudio_setup(struct gaudio *card)
{
	int	ret;

	ret = gaudio_open_snd_dev(card);
	if (ret)
		ERROR(card, "we need at least one control device\n");

	return ret;

}

/**
 * gaudio_cleanup - remove ALSA device interface
 *
 * This is called to free all resources allocated by @gaudio_setup().
 */
void gaudio_cleanup(struct gaudio *the_card)
{
	if (the_card)
		gaudio_close_snd_dev(the_card);
}

