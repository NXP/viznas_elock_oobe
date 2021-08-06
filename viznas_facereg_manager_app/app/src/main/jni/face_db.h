/*
 * Copyright 2019 NXP.
 * This software is owned or controlled by NXP and may only be used strictly in accordance with the
 * license terms that accompany it. By expressly accepting such terms or by downloading, installing,
 * activating and/or otherwise using the software, you are agreeing that you have read, and that you
 * agree to comply with and are bound by, such license terms. If you do not agree to be bound by the
 * applicable license terms, then you may not retain, install, activate or otherwise use the software.d
 *
 * Created by: NXP China Solution Team.
 */

#ifndef _FACE_DB_H_
#define _FACE_DB_H_

#include "featuredb.h"
#include "oasislite_runtime.h"

int face_db_init(float thres);

//int face_db_add(std::vector<oasis::Face>& faces);
int face_db_add(std::string name, std::vector<float> &feature);

//int face_db_find(std::vector<Face>& faces);

int face_db_exit();

#endif /* _FACE_DB_H_ */
