/*
 * Copyright 2020-2021 NXP.
 * This software is owned or controlled by NXP and may only be used strictly in accordance with the
 * license terms that accompany it. By expressly accepting such terms or by downloading, installing,
 * activating and/or otherwise using the software, you are agreeing that you have read, and that you
 * agree to comply with and are bound by, such license terms. If you do not agree to be bound by the
 * applicable license terms, then you may not retain, install, activate or otherwise use the software.
 */

/*
 * @brief flexio camera driver adapter implementation.
 */

#ifndef _FSL_FLEXIO_CAMERA_ADAPTER_H_
#define _FSL_FLEXIO_CAMERA_ADAPTER_H_

#include "fsl_common.h"
#include "fsl_flexio_camera.h"
#include "fsl_component_generic_list.h"
#include "FreeRTOS.h"
#include "task.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define FLEXIO_CAMERA_ADAPTER_BUFFER_MAX 4
#define FLEXIO_CAMERA_ADAPTER_MALLOC     pvPortMalloc
#define FLEXIO_CAMERA_ADAPTER_FREE       vPortFree

typedef enum _flexio_camera_status_t
{
    kStatus_Flexio_Camera_Success,
    kStatus_Flexio_Camera_Error,
    kStatus_Flexio_Camera_NoFullBuffer,
    kStatus_Flexio_Camera_NoEmptyBuffer,
    kStatus_Flexio_Camera_NoActiveBuffer,
} flexio_camera_status_t;

typedef struct _flexio_camera_list_element_t
{
    list_element_t listElement;
    uint8_t *bufferAddress;
} flexio_camera_list_element_t;

typedef struct _flexio_camera_list_t
{
    list_label_t emptyList;
    list_label_t fullList;
    list_element_handle_t currentElement;
} flexio_camera_list_t;

typedef struct _flexio_camera_private_data_t
{
    flexio_camera_list_t lists;          /*!< driver lists use by low level. */
    camera_receiver_callback_t callback; /*!< Save the callback. */
    void *userData;                      /*!< Parameter for the callback. */
} flexio_camera_private_data_t;

typedef struct _flexio_camera_xclk_t
{
    uint32_t pinIdx;
    uint32_t timerIdx;
} flexio_camera_xclk_t;

typedef struct _flexio_camera_dma_t
{
    uint32_t dmaCh;
    uint32_t majorLoop;
    uint32_t tcdLinkOffset;
} flexio_camera_dma_t;

typedef struct _flexio_camera_vsync_t
{
    GPIO_Type *portBase;
    uint8_t pinIdx;
    IRQn_Type irq;
    uint8_t irqPriority;
} flexio_camera_vsync_t;

/*!
 * @brief The resources used by the flexio csi camera receiver.
 * Don't need to initialize the resource before flexio csi initialization.
 */
typedef struct _flexio_csi_resource
{
    FLEXIO_CAMERA_Type *flexioCsi;      /*!< flexio camera setting. */
    flexio_camera_xclk_t *flexioXclk;   /*!< flexio camera xclk config. */
    flexio_camera_dma_t *flexioDma;     /*!< flexio camera dama config. */
    flexio_camera_vsync_t *flexioVsync; /*!< flexio camera vsync signal. */
} flexio_csi_resource_t;

/*! @brief flexio csi camera receiver operations structure. */
extern const camera_receiver_operations_t flexio_csi_ops;

/*******************************************************************************
 * API
 ******************************************************************************/
#ifdef __cplusplus
extern "C" {
#endif

status_t FLEXIO_CSI_ADAPTER_Callback(camera_receiver_handle_t *handle);

#ifdef __cplusplus
}
#endif

#endif /* _FSL_FLEXIO_CAMERA_ADAPTER_H_ */
