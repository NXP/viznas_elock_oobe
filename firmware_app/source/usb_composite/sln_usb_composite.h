/*
 * Copyright 2019-2021 NXP.
 * This software is owned or controlled by NXP and may only be used strictly in accordance with the
 * license terms that accompany it. By expressly accepting such terms or by downloading, installing,
 * activating and/or otherwise using the software, you are agreeing that you have read, and that you
 * agree to comply with and are bound by, such license terms. If you do not agree to be bound by the
 * applicable license terms, then you may not retain, install, activate or otherwise use the software.
 */

#ifndef USB_COMPOSITE_SLN_USB_COMPOSITE_H_
#define USB_COMPOSITE_SLN_USB_COMPOSITE_H_

#if defined(__cplusplus)
extern "C" {
#endif /* _cplusplus */

/**
 * @brief Init the composite device  USB CDC + UVC
 */
void USB_CompositeInit(void);

/**
 * @brief DeInit the composite device  USB CDC + UVC
 */

void USB_CompositeDeInit(void);

#if defined(__cplusplus)
}
#endif /* _cplusplus */

#endif /* USB_COMPOSITE_SLN_USB_COMPOSITE_H_ */
