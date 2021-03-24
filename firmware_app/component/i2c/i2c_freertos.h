/*
 * Copyright 2018-2019 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __HAL_I2C_ADAPTER_FREERTOS_H__
#define __HAL_I2C_ADAPTER_FREERTOS_H__

#include "FreeRTOS.h"
#include "portable.h"
#include "semphr.h"
#include "i2c.h"
/*!
 * @addtogroup I2C_Adapter
 * @{
 */

/*******************************************************************************
 * API
 ******************************************************************************/

#if defined(__cplusplus)
extern "C" {
#endif /*_cplusplus. */

/*!
 * @name Initialization and de-initialization
 * @{
 */

/*!
 * @brief Initializes the HAL I2C master peripheral.
 *
 * @note This API should be called at the beginning of the application.
 * Otherwise, any operation to the HAL I2C module can cause a hard fault
 * because the clock is not enabled. This function configures the i2c master
 * with user-defined settings. The user can configure the configuration
 * structure. The parameter handle is a pointer to point to a memory space
 * of size #HAL_I2C_MASTER_HANDLE_SIZE allocated by the caller.
 *
 * Example below shows how to use this API to configure the I2C master.
 * @code
 *   HAL_I2C_MASTER_HANDLE_DEFINE(i2cMasterHandle);
 *   hal_i2c_master_config_t masterConfig;
 *   masterConfig.enableMaster   = true;
 *   masterConfig.baudRate_Bps   = 100000U;
 *   masterConfig.srcClock_Hz    = 12000000U;
 *   masterConfig.instance       = 0;
 *   HAL_I2cMasterInit((hal_i2c_master_handle_t)i2cMasterHandle, &masterConfig);
 * @endcode
 *
 * @param handle Pointer to point to a memory space of size #HAL_I2C_MASTER_HANDLE_SIZE allocated by the caller.
 * The handle should be 4 byte aligned, because unaligned access doesn't be supported on some devices.
 * You can define the handle in the following two ways:
 * #HAL_I2C_MASTER_HANDLE_DEFINE(handle);
 * or
 * uint32_t handle[((HAL_I2C_MASTER_HANDLE_SIZE + sizeof(uint32_t) - 1U) / sizeof(uint32_t))];
 * @param config A pointer to the master configuration structure
 * @retval kStatus_HAL_I2cError An error occurred.
 * @retval kStatus_HAL_I2cSuccess i2c master initialization succeed
 */
hal_i2c_status_t HAL_I2cMasterInit_FreeRTOS(hal_i2c_master_handle_t handle, const hal_i2c_master_config_t *config);

/*!
 * @brief De-initializes the HAL I2C master peripheral. Call this API to gate the HAL I2C clock.
 * The HAL I2C master module can't work unless the HAL_I2cMasterInit is called.
 *
 * @param handle i2c master handle pointer, this should be a static variable.
 * @retval kStatus_HAL_I2cSuccess i2c master de-initialization succeed */
hal_i2c_status_t HAL_I2cMasterDeinit_FreeRTOS(hal_i2c_master_handle_t handle);

/*! @} */

/*!
 * @name Bus Operations
 * @{
 */

/*!
 * @brief Performs a master polling transfer on the HAL I2C bus.
 *
 * @note The API does not return until the transfer succeeds or fails due
 * to arbitration lost or receiving a NAK.
 *
 * @param handle i2c master handle pointer, this should be a static variable.
 * @param xfer Pointer to the transfer structure.
 * @retval kStatus_HAL_I2cSuccess Successfully complete the data transmission.
 * @retval kStatus_HAL_I2cBusy Previous transmission still not finished.
 * @retval kStatus_HAL_I2cTimeout Transfer error, wait signal timeout.
 * @retval kStatus_HAL_I2cArbitrationLost Transfer error, arbitration lost.
 * @retval kStatus_HAL_I2cNak Transfer error, receive NAK during transfer.
 */
hal_i2c_status_t HAL_I2cMasterTransfer_FreeRTOS(hal_i2c_master_handle_t handle, hal_i2c_master_transfer_t *xfer);

/*! @} */

#if defined(__cplusplus)
}
#endif /*_cplusplus. */
/*! @} */

#endif /* __HAL_I2C_ADAPTER_H__*/
