/*
 * Copyright 2019 NXP.
 * This software is owned or controlled by NXP and may only be used strictly in accordance with the
 * license terms that accompany it. By expressly accepting such terms or by downloading, installing,
 * activating and/or otherwise using the software, you are agreeing that you have read, and that you
 * agree to comply with and are bound by, such license terms. If you do not agree to be bound by the
 * applicable license terms, then you may not retain, install, activate or otherwise use the software.
 *
 * Created by: dongsheng.zhang@nxp.com
 */

#ifndef _USER_ID_UI_H_
#define _USER_ID_UI_H_

#if defined(__cplusplus)
extern "C" {
#endif /* __cplusplus */

/*******************************************************************************
 * API
 *******************************************************************************/
void UIInfo_Update(uint16_t *pBufferAddr, QUIInfoMsg* infoMsg, uint8_t p_DisplayInterfaceMode);

#if defined(__cplusplus)
}
#endif /* __cplusplus */

#endif /* _USER_ID_UI_H_ */
