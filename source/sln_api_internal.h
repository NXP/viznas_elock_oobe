/*
 * Copyright 2019 NXP.
 * This software is owned or controlled by NXP and may only be used strictly in accordance with the
 * license terms that accompany it. By expressly accepting such terms or by downloading, installing,
 * activating and/or otherwise using the software, you are agreeing that you have read, and that you
 * agree to comply with and are bound by, such license terms. If you do not agree to be bound by the
 * applicable license terms, then you may not retain, install, activate or otherwise use the software.
 */
#ifndef SLN_API_INTERNAL_H_
#define SLN_API_INTERNAL_H_

#include "sln_api.h"

/**
 * @brief Event that oasis lib calls when a face is recognized.
 *
 * @param apiHandle       The API handler on which clients are registered
 * @param faceInfo        FaceInfo that will be send to clients which registered hooks (callbacks) for recognition.
 * @return                Status of VIZN_RecognizeEvent
 */
vizn_api_status_t VIZN_RecognizeEvent(VIZN_api_handle_t apiHandle, face_info_t faceInfo);

/**
 * @brief Event that oasis lib calls when a face is detected.
 *
 * @param apiHandle       The API handler on which clients are registered
 * @param dt        Detection Time that will be send to clients which registered hooks (callbacks) for detection.
 * @return                Status of VIZN_DetectEvent
 */
vizn_api_status_t VIZN_DetectEvent(VIZN_api_handle_t apiHandle, int dt);

/**
 * @brief Event that oasis lib calls when a face is enrolled.
 *
 * @param apiHandle       The API handler on which clients are registered
 * @param faceInfo        FaceInfo that will be send to clients which registered hooks (callbacks) for enrolment.
 * @return                Status of VIZN_EnrolmentEvent
 */
vizn_api_status_t VIZN_EnrolmentEvent(VIZN_api_handle_t apiHandle, face_info_t faceInfo);

/**
 * @brief This is used to get event.
 *
 * @param apiHandle    		The API handler on which clients are registered
 * @param events              Get the events which resulted from oasis.
 */
vizn_api_status_t VIZN_GetEvents(VIZN_api_handle_t *apiHandle, uint32_t *events);

/**
 * @brief Start Recognition Process.
 *
 * @param clientHandle    The client handler which required this action
 * @return             Status of VIZN_StartRecognition
 */
vizn_api_status_t VIZN_StartRecognition(VIZN_api_client_t *clientHandle);

/**
 * @brief Start Recognition VIZN_EnrolmentAddNewFace.
 *
 * @param clientHandle    The client handler which required this action
 * @param face_id         Free id returned by the database
 * @param face            Points to a buffer in which face feature are saved for the face which will be added
 * @return                Status of VIZN_StartRecognition
 */
vizn_api_status_t VIZN_EnrolmentAddNewFace(VIZN_api_client_t *clientHandle, uint16_t *face_id, void *face);

#endif /*SLN_API_INTERNAL_H_*/
