/*
 * u_uac1.h -- interface to USB gadget "ALSA AUDIO" utilities
 *
 * Copyright (C) 2008 Bryan Wu <cooloney@kernel.org>
 * Copyright (C) 2008 Analog Devices, Inc
 *
 * Enter bugs at http://blackfin.uclinux.org/
 *
 * Licensed under the GPL-2 or later.
 */

#ifndef __U_AUDIO_H
#define __U_AUDIO_H

#include <linux/device.h>
#include <linux/err.h>
#include <linux/usb/audio.h>
#include <linux/usb/composite.h>

#include <sound/core.h>
#include <sound/pcm.h>
#include <sound/pcm_params.h>

#define FILE_PCM_PLAYBACK	"/dev/snd/pcmC0D0p"
#define FILE_PCM_CAPTURE	"/dev/snd/pcmC0D0c"
#define FILE_CONTROL		"/dev/snd/controlC0"

#if defined(CONFIG_SS_GADGET) ||defined(CONFIG_SS_GADGET_MODULE)
#define UAC1_CAPTURE_SAMPLE_RATE		16000
#define UAC1_CAPTURE_CHANNEL_COUNT		1
#define UAC1_IN_EP_MAX_PACKET_SIZE		32
#define UAC1_IN_REQ_COUNT        		32
#define UAC1_AUDIO_CAPTURE_BUF_SIZE		1024

#define UAC1_PLAYBACK_SAMPLE_RATE		16000
#define UAC1_PLAYBACK_CHANNEL_COUNT		1
#endif
#define UAC1_OUT_EP_MAX_PACKET_SIZE		512
#define UAC1_OUT_REQ_COUNT        		8
#define UAC1_AUDIO_PLAYBACK_BUF_SIZE 	2048

/*
 * This represents the USB side of an audio card device, managed by a USB
 * function which provides control and stream interfaces.
 */

struct gaudio_snd_dev {
	struct gaudio			*card;
	struct file			*filp;
	struct snd_pcm_substream	*substream;
	int				access;
	int				format;
	int				channels;
	int				rate;
	int				period_bytes;
	int				buffer_bytes;
};

struct gaudio {
	struct usb_function		func;
	struct usb_gadget		*gadget;

	/* ALSA sound device interfaces */
	struct gaudio_snd_dev		control;
	struct gaudio_snd_dev		playback;
	struct gaudio_snd_dev		capture;

	/* TODO */
};

#if defined(CONFIG_SS_GADGET) ||defined(CONFIG_SS_GADGET_MODULE)
typedef enum audio_mode {
	DISABLE_UAC,
	ENABLE_SPEAKER,
	ENABLE_MICROPHONE,
	ENABLE_MIC_AND_SPK,
	UNKNOWN_COMMAND,
} audio_mode_e;
#endif
struct f_uac1_opts {
	struct usb_function_instance	func_inst;
	int				out_req_buf_size;
	int				out_req_count;
	int				audio_playback_buf_size;
#if defined(CONFIG_SS_GADGET) ||defined(CONFIG_SS_GADGET_MODULE)
	int				playback_channel_count;
	int				playback_sample_rate;

	int				capture_channel_count;
	int				capture_sample_rate;
	int				in_req_buf_size;
	int				in_req_count;
	int				audio_capture_buf_size;
	audio_mode_e			audio_play_mode;
#endif
	char				*fn_play;
	char				*fn_cap;
	char				*fn_cntl;
	unsigned			bound:1;
	unsigned			fn_play_alloc:1;
	unsigned			fn_cap_alloc:1;
	unsigned			fn_cntl_alloc:1;
	struct mutex			lock;
	int				refcnt;
};

int gaudio_setup(struct gaudio *card);
void gaudio_cleanup(struct gaudio *the_card);

size_t u_audio_playback(struct gaudio *card, void *buf, size_t count);
size_t u_audio_capture(struct gaudio *card, void *buf, size_t count);

int u_audio_get_playback_channels(struct gaudio *card);
int u_audio_get_playback_rate(struct gaudio *card);
int u_audio_get_capture_channels(struct gaudio *card);
int u_audio_get_capture_rate(struct gaudio *card);
#endif /* __U_AUDIO_H */
