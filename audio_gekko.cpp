#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <math.h>
#include <gccore.h>
#include <wiiuse/wpad.h>

/* part of this is copied from SDL_wii, which is LGPL, everything here is under GPL, so that's fine. */
/* I tried to use asndlib but that only gave crappy output */

#include "audio.h"

#define SAMPLES_PER_DMA_BUFFER (512)
#define SAMPLES_PER_REQ        (470)

static unsigned int dma_buffers[2][SAMPLES_PER_DMA_BUFFER*8] __attribute__((aligned(32)));
static unsigned int whichab = 0;

#define AUDIOSTACK 16384*2
static lwpq_t audioqueue;
static lwp_t athread;
static unsigned char astack[AUDIOSTACK];

static void * AudioThread (void *arg)
{
	while (1)
	{
		whichab ^= 1;
		memset(dma_buffers[whichab], 0, sizeof(dma_buffers[0]));
		audio_callback(NULL, (unsigned char *)dma_buffers[whichab], SAMPLES_PER_DMA_BUFFER*4);
		LWP_ThreadSleep (audioqueue);
	}
	return NULL;
}

static void DMACallback()
{
	AUDIO_StopDMA();
	DCFlushRange(dma_buffers[whichab], sizeof(dma_buffers[0]));
	AUDIO_InitDMA((unsigned int)dma_buffers[whichab], SAMPLES_PER_DMA_BUFFER*4);
	AUDIO_StartDMA();

	LWP_ThreadSignal (audioqueue);
}

void initAudio_sys()
{
	AUDIO_Init(NULL);
	AUDIO_SetDSPSampleRate(AI_SAMPLERATE_48KHZ);

	LWP_InitQueue (&audioqueue);

	LWP_CreateThread (&athread, AudioThread, NULL, astack, AUDIOSTACK, 127);

	AUDIO_RegisterDMACallback(DMACallback);
	DMACallback();
}
