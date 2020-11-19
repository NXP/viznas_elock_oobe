/*
 * Copyright 2020 NXP.
 * This software is owned or controlled by NXP and may only be used strictly in accordance with the
 * license terms that accompany it. By expressly accepting such terms or by downloading, installing,
 * activating and/or otherwise using the software, you are agreeing that you have read, and that you
 * agree to comply with and are bound by, such license terms. If you do not agree to be bound by the
 * applicable license terms, then you may not retain, install, activate or otherwise use the software.
 */
#include "FreeRTOS.h"
#include "event_groups.h"
#include "task.h"
#include "sln_streamer_pcm.h"
#include "sln_amplifier.h"
#include "sln_audio.h"

#include "welcome.h"
#include "regFailed.h"
#include "regSuccessful.h"

#define AUDIO_BYTE_WIDTH       2
#define AUDIO_SAMPLE_RATE      48000
#define AUDIO_TIME_MAX_SECOND  2 // limited to lock timer MS_SYSTEM_LOCKED.
#define AUDIO_LENGTH_MAX       (AUDIO_SAMPLE_RATE * AUDIO_BYTE_WIDTH * AUDIO_TIME_MAX_SECOND)

static EventGroupHandle_t s_offlineAudioEventGroup  = NULL;
static uint8_t s_offlineAudioPlayerCurrentStatus    = 0xFF;

void offline_audio_task(void *arg)
{
    EventBits_t offlineAudioEventBits;

    const clock_audio_pll_config_t audioPllConfig = {
                .loopDivider = 32,  /* PLL loop divider. Valid range for DIV_SELECT divider value: 27~54. */
                .postDivider = 1,   /* Divider after the PLL, should only be 1, 2, 4, 8, 16. */
                .numerator = 77,    /* 30 bit numerator of fractional loop divider. */
                .denominator = 100, /* 30 bit denominator of fractional loop divider */
        };

    CLOCK_InitAudioPll(&audioPllConfig);

    streamer_pcm_init();

    // Play offline alert audio if needed
    s_offlineAudioEventGroup = xEventGroupCreate();

    if (s_offlineAudioEventGroup != NULL)
    {
        /* Make it 0 after init finished. We can now play audio */
        s_offlineAudioPlayerCurrentStatus = 0;
        while(1)
        {
            offlineAudioEventBits = xEventGroupWaitBits(s_offlineAudioEventGroup, 0xff, pdTRUE, pdFALSE, portMAX_DELAY);

            // TODO:Choose what audio to play, expend audio samples.
            if ((1 << AUDIO_HOME_WELCOME) & offlineAudioEventBits)
            {
                configPRINTF(("Starting offline timer audio...\r\n"));
                /* Need to set the alerting state, put individual assignments as this may change depending on the type of alert
                ** This needs to be set to ensure that the resume UX shows alerting when offline
                */
                SLN_AMP_SetVolume(100);
                SLN_AMP_WriteBlocking((uint8_t *)welcome_female_friendly_wav,
                        MIN(welcome_female_friendly_wav_len, AUDIO_LENGTH_MAX));
                SLN_AMP_SetVolume(0);
            }
            if ((1 << AUDIO_REGISTRATION_FAILED) & offlineAudioEventBits)
            {
                configPRINTF(("Starting offline timer audio...\r\n"));
                /* Need to set the alerting state, put individual assignments as this may change depending on the type of alert
                ** This needs to be set to ensure that the resume UX shows alerting when offline
                */
                SLN_AMP_SetVolume(100);
                SLN_AMP_WriteBlocking((uint8_t *)regFailed_wav, MIN(regFailed_wav_len, AUDIO_LENGTH_MAX));
                SLN_AMP_SetVolume(0);
            }
            if ((1 << AUDIO_REGISTRATION_SUCCESSFUL) & offlineAudioEventBits)
            {
                configPRINTF(("Starting offline timer audio...\r\n"));
                /* Need to set the alerting state, put individual assignments as this may change depending on the type of alert
                ** This needs to be set to ensure that the resume UX shows alerting when offline
                */
                SLN_AMP_SetVolume(100);
                SLN_AMP_WriteBlocking((uint8_t *)regSuccessful_wav, MIN(regSuccessful_wav_len, AUDIO_LENGTH_MAX));
                SLN_AMP_SetVolume(0);
            }

            s_offlineAudioPlayerCurrentStatus = 0;
        }
    }
}

void Audio_Start(void)
{
    /* Setup task for playing offline audio */
    xTaskCreate(offline_audio_task, "Offline_Audio_Task", 256, NULL, configMAX_PRIORITIES - 1, NULL);
}

int Audio_Play_Samples(AudioSamplesID audioSampleId)
{
    //idle, send evt play audio.
    if (s_offlineAudioPlayerCurrentStatus == 0)
    {
        s_offlineAudioPlayerCurrentStatus = 1 << audioSampleId;
        xEventGroupSetBits(s_offlineAudioEventGroup, 1 << audioSampleId);
        return 0;
    }

    return -1;
}

uint8_t Get_Audio_Player_Status(void)
{
    return s_offlineAudioPlayerCurrentStatus;
}
