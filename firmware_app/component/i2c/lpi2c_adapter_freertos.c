/*
 * Copyright 2018 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_common.h"
#include "fsl_lpi2c_freertos.h"

#include "i2c.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
/*! @brief i2c master state structure. */
typedef struct _hal_i2c_master
{
    hal_i2c_master_transfer_callback_t callback;
    void *callbackParam;
    lpi2c_master_handle_t hardwareHandle;
    uint8_t instance;
} hal_i2c_master_t;

/*! @brief i2c slave state structure. */
typedef struct _hal_i2c_slave
{
    hal_i2c_slave_transfer_callback_t callback;
    void *callbackParam;
    hal_i2c_slave_transfer_t transfer;
    lpi2c_slave_handle_t hardwareHandle;
    uint8_t instance;
} hal_i2c_slave_t;

/*******************************************************************************
 * Variables
 ******************************************************************************/


/*! @brief Pointers to i2c bases for each instance. */
static LPI2C_Type *const s_i2cBases[] = LPI2C_BASE_PTRS;

/*! @brief Pointers to i2c bases for each instance. */
static IRQn_Type const s_i2cIRQs[] = LPI2C_IRQS;

/*! @brief Handle to a freertos i2c Handle*/
static lpi2c_rtos_handle_t s_i2cHandle[sizeof(s_i2cBases)];

/*******************************************************************************
 * Code
 ******************************************************************************/

static hal_i2c_status_t HAL_I2cGetStatus(status_t status)
{
    hal_i2c_status_t returnStatus;
    switch (status)
    {
        case kStatus_Success:
        {
            returnStatus = kStatus_HAL_I2cSuccess;
            break;
        }
        case kStatus_LPI2C_Busy:
        {
            returnStatus = kStatus_HAL_I2cBusy;
            break;
        }
        case kStatus_LPI2C_Idle:
        {
            returnStatus = kStatus_HAL_I2cIdle;
            break;
        }
        case kStatus_LPI2C_Nak:
        {
            returnStatus = kStatus_HAL_I2cNak;
            break;
        }
        case kStatus_LPI2C_ArbitrationLost:
        {
            returnStatus = kStatus_HAL_I2cArbitrationLost;
            break;
        }
        case kStatus_LPI2C_Timeout:
        {
            returnStatus = kStatus_HAL_I2cTimeout;
            break;
        }
        default:
        {
            returnStatus = kStatus_HAL_I2cError;
            break;
        }
    }
    return returnStatus;
}

hal_i2c_status_t HAL_I2cMasterInit_FreeRTOS(hal_i2c_master_handle_t handle, const hal_i2c_master_config_t *config)
{
    hal_i2c_master_t *i2cMasterHandle;
    lpi2c_master_config_t i2cConfig;

    assert(handle);
    assert(config);
    assert(HAL_I2C_MASTER_HANDLE_SIZE >= sizeof(hal_i2c_master_t));

    i2cMasterHandle = (hal_i2c_master_t *)handle;

    LPI2C_MasterGetDefaultConfig(&i2cConfig);
    i2cConfig.enableMaster    = config->enableMaster;
    i2cConfig.baudRate_Hz     = config->baudRate_Bps;
    i2cMasterHandle->instance = config->instance;
    if(NULL == s_i2cHandle[i2cMasterHandle->instance].base)
    {
		LPI2C_RTOS_Init(&s_i2cHandle[i2cMasterHandle->instance], s_i2cBases[i2cMasterHandle->instance], &i2cConfig, config->srcClock_Hz);
		NVIC_SetPriority(s_i2cIRQs[i2cMasterHandle->instance], configMAX_SYSCALL_INTERRUPT_PRIORITY - 1);
    }
    return kStatus_HAL_I2cSuccess;
}

hal_i2c_status_t HAL_I2cMasterDeinit_FreeRTOS(hal_i2c_master_handle_t handle)
{
    hal_i2c_master_t *i2cMasterHandle;

    assert(handle);

    i2cMasterHandle = (hal_i2c_master_t *)handle;

    LPI2C_RTOS_Deinit(&s_i2cHandle[i2cMasterHandle->instance]);

    return kStatus_HAL_I2cSuccess;
}

hal_i2c_status_t HAL_I2cMasterTransfer_FreeRTOS(hal_i2c_master_handle_t handle, hal_i2c_master_transfer_t *xfer)
{
    hal_i2c_master_t *i2cMasterHandle;
    lpi2c_master_transfer_t transfer;

    assert(handle);
    assert(xfer);

    i2cMasterHandle = (hal_i2c_master_t *)handle;

    transfer.flags          = xfer->flags;
    transfer.slaveAddress   = xfer->slaveAddress;
    transfer.direction      = (kHAL_I2cRead == xfer->direction) ? kLPI2C_Read : kLPI2C_Write;
    transfer.subaddress     = xfer->subaddress;
    transfer.subaddressSize = xfer->subaddressSize;
    transfer.data           = xfer->data;
    transfer.dataSize       = xfer->dataSize;

    return HAL_I2cGetStatus(LPI2C_RTOS_Transfer(&s_i2cHandle[i2cMasterHandle->instance], &transfer));
}

