/*
 * Copyright 2021 NXP.
 * This software is owned or controlled by NXP and may only be used strictly in accordance with the
 * license terms that accompany it. By expressly accepting such terms or by downloading, installing,
 * activating and/or otherwise using the software, you are agreeing that you have read, and that you
 * agree to comply with and are bound by, such license terms. If you do not agree to be bound by the
 * applicable license terms, then you may not retain, install, activate or otherwise use the software.
 */

#ifndef SLN_OASIS_STATE_H_
#define SLN_OASIS_STATE_H_

#include "sln_api.h"
#include <stdint.h>

/*******************************************************************************
 * Code API
 ******************************************************************************/
/**
 * @brief Start recognition process
 */
void StartRecognitionProcess(void);

/**
 * @brief Stop recognition process
 * @param event    Even with which the recognition will be stopped
 */
void StopRecognitionProcess(uint8_t event);

/**
 * @brief Start registration process
 */
//void StartRegistrationProcess(void);
void StartRegistrationProcess(char* name);

/**
 * @brief Stop registration process
 * @param event    Even with which the registration will be stopped
 */
void StopRegistrationProcess(uint8_t event);

/**
 * @brief Start deregistration process
 */
void StartDeregistrationProcess(void);

/**
 * @brief Stop deregistration process
 * @param event    Even with which the deregistration will be stopped
 */
void StopDeregistrationProcess(uint8_t event);

/**
 * @brief Start lock process
 */
void StartLockProcess(bool timerEnable);

/**
 * @brief Stop lock process
 */
void StopLockProcess(void);

/**
 * @brief Oasis callback when enrolment event ocurs
 * @param face_info    Face_info which resulted from oasis after enrolment
 */
void Oasis_API_Enrolment(face_info_t face_info);

/**
 * @brief Oasis callback when recognize event ocurs
 * @param face_info    Face_info which resulted from oasis after recognize
 */
void Oasis_API_Recognize(face_info_t face_info);

/**
 * @brief Oasis callback when detect event ocurs
 * @param dt    Detection time
 */
void Oasis_API_Detect(int dt);

/**
 * @brief Oasis init
 * @param appType    AppType for which the oasis is init
 */
void Oasis_State_Init(uint8_t appType);

/**
 * @brief Oasis timer callback
 * @param id_timer    id of the timer which generated the event
 */
void Oasis_TimerCallback(uint8_t id_timer);

/**
 * @brief Get the internal state of the event
 * @return   Status of VIZN_UnregisterClient
 */
uint32_t Oasis_Get_Events(void);

#endif
