/*
 * AlsaLibMapping -- provide low level interface with AUDIO lib (extracted from alsa-json-gateway code)
 * Copyright (C) 2015,2016,2017, Fulup Ar Foll fulup@iot.bzh
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef SHAREDHALLIB_H
#define SHAREDHALLIB_H

#include <stdio.h>
#include <afb/afb-binding.h>

#include "MiscHelpers.h"
#include "AlsaHalCtls.h"

typedef const struct {
    halControlEnumT control;
    int numid;
    halGroupEnumT group;
    int values;
    int minval;
    int maxval;
    int step;
    char* info;
    halAclEnumT acl;
    
} alsaHalCtlMapT;

typedef struct  {
    const char  *prefix;
    const char  *name;
    const char  *info;
    alsaHalCtlMapT *ctls;
    
} alsaHalSndCardT;

PUBLIC alsaHalSndCardT alsaHalSndCard;
PUBLIC struct afb_binding alsaHalBinding;

#endif /* SHAREDHALLIB_H */

