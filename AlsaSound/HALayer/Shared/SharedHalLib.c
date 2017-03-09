/*
 * Copyright (C) 2016 "IoT.bzh"
 * Author Fulup Ar Foll <fulup@iot.bzh>
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
 * 
 * reference: 
 *   amixer contents; amixer controls;
 *   http://www.tldp.org/HOWTO/Alsa-sound-6.html 
 */
#define _GNU_SOURCE  // needed for vasprintf
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <netdb.h>
#include <fcntl.h>
#include <math.h>
#include <sys/time.h>
#include <sys/types.h>

#include <json-c/json.h>
#include <afb/afb-binding.h>
#include <afb/afb-service-itf.h>

#include "MiscHelpers.h"
#include "AlsaHalIface.h"

typdef struct {
    int numid;
} shareHallMap_T;

static struct afb_service srvitf;
static const struct afb_binding_interface *afbIface;
static shareHallMap_T *shareHallMap;


STATIC void localping(struct afb_req request) {
    json_object *query = afb_req_json(request);
    afb_req_success(request, query, NULL); 
}

// This callback when api/alsacore/subscribe returns
STATIC void alsaSubcribeCB (void *handle, int iserror, struct json_object *result) {
	struct afb_req request = afb_req_unstore(handle);
        struct json_object *x, *resp = NULL;
	const char *info = NULL;

	if (result) {
	    INFO (afbIface, "result=[%s]\n", json_object_to_json_string (result));
            if (json_object_object_get_ex(result, "request", &x) && json_object_object_get_ex(x, "info", &x))
            info = json_object_get_string(x);
            if (!json_object_object_get_ex(result, "response", &resp)) resp = NULL;
        }
        
        // push message respond
	if (iserror) afb_req_fail_f(request,  "Fail", info);
	else         afb_req_success(request, resp, info);
        
        // free calling request
	afb_req_unref(request);
}

// Create and subscribe to alsacore ctl events
STATIC void halMonitor(struct afb_req request) {
    
    // save request in session as it might be used after return by callback
    struct afb_req *handle = afb_req_store(request);

    // push request to low level binding
    if (!handle) afb_req_fail(request, "error", "out of memory");
    else    afb_service_call(srvitf, "alsacore", "subctl", json_object_get(afb_req_json(request)), alsaSubcribeCB, handle);

    // success/failure messages return from callback    
}

// Subscribe to AudioBinding events
STATIC void halSubscribe (struct afb_req request) {
    const char *devid = afb_req_value(request, "devid");
    if (devid == NULL) {
        afb_req_fail_f (request, "devid-missing", "devid=hw:xxx missing");
    }   
}

// Call when all bindings are loaded and ready to accept request
STATIC void halGetVol(struct afb_req request) {
   
    // Should call here Hardware Alsa Abstraction Layer for corresponding Sound Card
    afb_req_success (request, NULL, NULL);
    return;
    
}

STATIC void halSetVol(struct afb_req request) {
    const char *arg;
    const char *pcm;
    
    arg = afb_req_value(request, "vol");
    if (arg == NULL) {
        afb_req_fail_f (request, "argument-missing", "vol=[0,100] missing");
        goto OnErrorExit;
    }
    
    pcm = afb_req_value(request, "pcm");
    if (pcm == NULL) pcm="Master";
   
    // Should call here Hardware Alsa Abstraction Layer for corresponding Sound Card
    afb_req_success (request, NULL, NULL);
    return;
    
  OnErrorExit:
    return;
    
}

// this is call when after all bindings are loaded
STATIC int halGetControl(struct afb_service service) {
    srvitf = service;
    struct json_object *queryin, *queryout, *ctls, *devid;

    // get query from request
    queryin = afb_req_json(request);

    // check devid was given
    devid= json_object_object_get(queryin,"devid");
    if (!ctls) {
        afb_req_fail_f(request, "devid-notfound", "No DevID given query=[%s]", json_object_get_string(queryin));
        goto OnErrorExit;
    }
    
    // loop on requested controls
    ctls= json_object_object_get(queryin,"ctls");
    if (!ctls || json_object_array_length(ctls) <= 0) {
        afb_req_fail_f(request, "ctls-notfound", "No Controls given query=[%s]", json_object_get_string(queryin));
        goto OnErrorExit;
    }
    
    for (int idx=0; idx< json_object_array_length(ctls), idx++) {
        struct json_object *ctl;
        halControlEnumT control;
        int value;
        
        // each controls should be halControlEnumT+value
        ctl = json_object_array_get_idx(ctls, idx);
        if (json_object_array_length(ctl != 2)) {
            afb_req_fail_f(request, "ctl-invalid", "Invalid Control devid=%s ctl=[%s]"json_object_get_string(devid), json_object_get_string(cls)); 
            goto OnErrorExit;                     
        }
        
        // As HAL and Business logic use the same AlsaMixerHal.h direct conversion is not an issue
        control = (halControlEnumT)json_object_get_int(json_object_array_get_idx(ctl,0));
        value   = json_object_get_int(json_object_array_get_idx(ctl,0));
        
        if (control >= EndHalCrlTag || control <= StartHalCrlTag) {
            afb_req_fail_f(request, "ctl-invalid", "Invalid Control devid=%s ctl=[%s] should be [%d=%d]"
                          , json_object_get_string(devid), json_object_get_string(cls)), StartHalCrlTag, EndHalCrlTag; 
            goto OnErrorExit;                                 
        }
        
 
            default:
          NOTICE (afbIface, "audioLogicOpenCB2 unknown HAL control=[%s]", json_object_get_string(ctl)):
        }       
    }
    
    
    // register HAL with Alsa Low Level Binder devid=hw:0&numid=1&quiet=0
    queryurl=json_object_new_object();
    json_object_object_add(queryurl, "prefix",json_object_new_string(alsaHalBinding.v1.prefix));
    json_object_object_add(queryurl, "name"  ,json_object_new_string(alsaHalSndCard.name));
    afb_service_call(srvitf, "alsacore",  "registerHal", queryurl, halGetControlCB, queryurl);
    
 
    afb_req_success (request, sndctrls, NULL);
    return;
    
OnErrorExit:
        
};

STATIC void halInitCB (void *handle, int iserror, struct json_object *result) {
    struct json_object *queryurl = (json_object*)handle;

    if (iserror) NOTICE (afbIface, "halInitCB: registration alsaHAL query=[%s] Fail", json_object_to_json_string(queryurl));
    else DEBUG(afbIface, "halInitCB: registration alsaHAL card=[%s] Success", json_object_to_json_string(queryurl));
}

// This receive all event this binding subscribe to 
PUBLIC void afbBindingV1ServiceEvent(const char *evtname, struct json_object *object) {
  
    NOTICE (afbIface, "afbBindingV1ServiceEvent evtname=%s [msg=%s]", evtname, json_object_to_json_string(object));
}

// this is call when after all bindings are loaded
PUBLIC int afbBindingV1ServiceInit(struct afb_service service) {
    srvitf = service;
    struct json_object *queryurl;
    
    // API prefix is used as sndcard halname
   alsaHalBinding.v1.prefix= prefix;
    
    // register HAL with Alsa Low Level Binder
    queryurl=json_object_new_object();
    json_object_object_add(ctx->queryurl, "prefix",json_object_new_string(alsaHalBinding.v1.prefix));
    json_object_object_add(ctx->queryurl, "name"  ,json_object_new_string(alsaHalSndCard.name));
    afb_service_call(srvitf, "alsacore",  "registerHal", queryurl, halInitCB, NULL);

    return 0;
};


// Every HAL export the same API & Interface Mapping from SndCard to AudioLogic is done through alsaHalSndCardT
STATIC const struct afb_verb_desc_v1 halSharedApi[] = {
  /* VERB'S NAME            SESSION MANAGEMENT          FUNCTION TO CALL         SHORT DESCRIPTION */
  { .name= "ping"   ,    .session= AFB_SESSION_NONE, .callback= localping,    .info= "Ping Binding" },
  { .name= "getcontrol", .session= AFB_SESSION_NONE, .callback= halGetControl,.info= "Get Control" },
  { .name= "setvolume",  .session= AFB_SESSION_NONE, .callback= halSetVol,    .info= "Set Volume" },
  { .name= "getvolume",  .session= AFB_SESSION_NONE, .callback= halGetVol,    .info= "Get Volume" },
  { .name= "subscribe",  .session= AFB_SESSION_NONE, .callback= halSubscribe, .info= "Subscribe AudioBinding Events" },
  { .name= NULL } /* marker for end of the array */
};

// Process HAL mapping from alsaHalSndCardT before registering HAL binder
PUBLIC const struct afb_binding *afbBindingV1Register(const struct afb_binding_interface *itf) {
    int count;
    afbIface= itf;                     // need to keep a static trace of binder interface for avances functions
    alsaHalBinding.verbs=halSharedApi; // complete sndcard specific alsaHalBinding with standard HAL APIs
    alsaHalCtlMapT  *alsaHalCtls = MapalsaHalSndCard.alsaHalCtlsMap; // Get sndcard specific HAL control mapping
    
    if (alsaHalCtls == NULL) {
        ERROR (afbIface, "afbBindingV1Register Fail alsaHalCtlsMap==NULL");
        return NULL;
    }
    
    // Create a zone to store HAL high/low level mapping
    shareHallMap = calloc (EndHalCrlTag * sizeof(shareHallMap_T));
    for (int idx= 0; alsaHalCtlsMap[idx].numid != 0; idx++) {
        shareHallMap[alsaHalCtlsMap[idx].]->numid  = alsaHalCtlsMap[idx].numid;
    }
    
    return alsaHalBinding;	/* returns the description of the binding */
}
