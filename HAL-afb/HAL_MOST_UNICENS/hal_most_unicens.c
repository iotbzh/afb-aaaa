/*
 * Copyright (C) 2017, Microchip Technology Inc. and its subsidiaries.
 * Author Tobias Jahnke
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
 */
#define _GNU_SOURCE
#include <string.h>
#include "hal-interface.h"
#include "audio-common.h"
#include "wrap-json.h"
#include "wrap_unicens.h"
#include "wrap_volume.h"

#ifndef UCS2_CFG_PATH
# define UCS2_CFG_PATH "/home/agluser/DEVELOPMENT/AGL/BINDING/unicens2-binding/data"
#endif

#define ALSA_CARD_NAME    "Microchip MOST:1"
#define ALSA_DEVICE_ID    "hw:1"
#define PCM_MAX_CHANNELS  6

static int master_volume;
static json_bool master_switch;
static int pcm_volume[PCM_MAX_CHANNELS];

static void unicens_request_card_values_cb(void *closure, int status, struct json_object *j_response) {
    
    json_object *j_obj;
    int num_id;
    int values[6];
    
    AFB_INFO("unicens_request_card_values_cb: closure=%p status=%d, res=%s", closure, status, json_object_to_json_string(j_response));
    
    json_object_object_get_ex(j_response, "response", &j_obj);
    
    if (json_object_is_type(j_obj, json_type_object)) {
       
    }
    else if (json_object_is_type(j_obj, json_type_array)) {
        AFB_ERROR("unicens_request_card_values_cb: array not handled yet");
        return;
    }
    else {
        AFB_ERROR("unicens_request_card_values_cb: unknown response type");
        return;
    }
    
    if (wrap_json_unpack(j_obj, "{s:i, s:[iiiiii!]}", "id", &num_id, "val", 
            &values[0], &values[1], &values[2], &values[3], &values[4], &values[5])
            == 0) {
        AFB_NOTICE("unicens_request_card_values_cb: success, num id:%d", num_id);
    }
    else {
        AFB_ERROR("unicens_request_card_values_cb: unpack failure");
        return;
    }
    
#if 0
    if (num_id == 2) {
        wrap_volume_pcm(&values[0], 6);
    }
#endif
    
}

__attribute__ ((unused)) static void unicens_request_card_values(const char* dev_id) {

    int err;
    json_object *j_query = NULL;
    
    err = wrap_json_pack(&j_query, "{s:s, s:i}", "devid", dev_id, "mode", 0);

    if (err) {
        AFB_ERROR("Failed to call wrap_json_pack");
        goto OnErrorExit;
    }   

    afb_service_call("alsacore", "ctlget", j_query,
            &unicens_request_card_values_cb,
            NULL);
    
    if (err) {
        AFB_ERROR("Failed to call listconfig");
        goto OnErrorExit;
    }
    
OnErrorExit:
    return;
}

void unicens_master_vol_cb(halCtlsTagT tag, alsaHalCtlMapT *control, void* handle,  json_object *j_obj) {

    const char *j_str = json_object_to_json_string(j_obj);

    if (wrap_json_unpack(j_obj, "[i!]", &master_volume) == 0) {
        AFB_NOTICE("master_volume: %s, value=%d", j_str, master_volume);
        wrap_volume_master(master_volume);
    }
    else {
        AFB_NOTICE("master_volume: INVALID STRING %s", j_str);
    }
}

void unicens_master_switch_cb(halCtlsTagT tag, alsaHalCtlMapT *control, void* handle,  json_object *j_obj) {

    const char *j_str = json_object_to_json_string(j_obj);

    if (wrap_json_unpack(j_obj, "[b!]", &master_switch) == 0) {
        AFB_NOTICE("master_switch: %s, value=%d", j_str, master_switch);
    }
    else {
        AFB_NOTICE("master_switch: INVALID STRING %s", j_str);
    }
}

void unicens_pcm_vol_cb(halCtlsTagT tag, alsaHalCtlMapT *control, void* handle,  json_object *j_obj) {

    const char *j_str = json_object_to_json_string(j_obj);

    if (wrap_json_unpack(j_obj, "[iiiiii!]", &pcm_volume[0], &pcm_volume[1], &pcm_volume[2], &pcm_volume[3],
                                             &pcm_volume[4], &pcm_volume[5]) == 0) {
        AFB_NOTICE("pcm_vol: %s", j_str);
        wrap_volume_pcm(pcm_volume, PCM_MAX_CHANNELS/*array size*/);
    }
    else {
        AFB_NOTICE("pcm_vol: INVALID STRING %s", j_str);
    }
}

/* declare ALSA mixer controls */
STATIC alsaHalMapT  alsaHalMap[]= {
  { .tag=Master_Playback_Volume, .cb={.callback=unicens_master_vol_cb, .handle=&master_volume}, .info="Sets master playback volume",
    .ctl={.numid=CTL_AUTO, .type=SND_CTL_ELEM_TYPE_INTEGER, .count=1, .minval=0, .maxval=100, .step=1, .value=50, .name="Master Playback Volume"}
  },
  /*{ .tag=Master_OnOff_Switch, .cb={.callback=unicens_master_switch_cb, .handle=&master_switch}, .info="Sets master playback switch",
    .ctl={.numid=CTL_AUTO, .type=SND_CTL_ELEM_TYPE_BOOLEAN, .count=1, .minval=0, .maxval=1, .step=1, .value=1, .name="Master Playback Switch"}
  },*/
  { .tag=PCM_Playback_Volume, .cb={.callback=unicens_pcm_vol_cb, .handle=&pcm_volume}, .info="Sets PCM playback volume",
    .ctl={.numid=CTL_AUTO, .type=SND_CTL_ELEM_TYPE_INTEGER, .count=6, .minval=0, .maxval=100, .step=1, .value=100, .name="PCM Playback Volume"}
  },
  { .tag=EndHalCrlTag}              /* marker for end of the array */
} ;

/* HAL sound card mapping info */
STATIC alsaHalSndCardT alsaHalSndCard  = {
    .name  = ALSA_CARD_NAME,   /*  WARNING: name MUST match with 'aplay -l' */
    .info  = "HAL for MICROCHIP MOST sound card controlled by UNICENS binding",
    .ctls  = alsaHalMap,
    .volumeCB = NULL,               /* use default volume normalization function */
};

/* initializes ALSA sound card, UNICENS API */
STATIC int unicens_service_init() {
    int err = 0;
    char *config_file = NULL;
    AFB_NOTICE("Initializing HAL-MOST-UNICENS-BINDING");

    err = halServiceInit(afbBindingV2.api, &alsaHalSndCard);
    if (err) {
        AFB_ERROR("Cannot initialize ALSA soundcard.");
        goto OnErrorExit;
    }

    err= afb_daemon_require_api("UNICENS", 1);
    if (err) {
        AFB_ERROR("Failed to access UNICENS API");
        goto OnErrorExit;
    }

    err = wrap_ucs_getconfig_sync(UCS2_CFG_PATH, &config_file);
    if (err || (config_file == NULL)) {
        AFB_ERROR("Failed to retrieve configuration");
        goto OnErrorExit;
    }
    else {
        AFB_NOTICE("Found configuration: %s", config_file);
    }

    err = wrap_ucs_subscribe_sync();
    if (err) {
        AFB_ERROR("Failed to subscribe to UNICENS binding");
        goto OnErrorExit;
    }

    err = wrap_ucs_initialize_sync(config_file);
    free(config_file);
    config_file = NULL;

    if (err) {
        AFB_ERROR("Failed to initialize UNICENS binding");
        goto OnErrorExit;
    }

    err = wrap_volume_init();
    if (err) {
        AFB_ERROR("Failed to initialize wrapper for volume library");
        goto OnErrorExit;
    }
    
    /* request of initial card values */
    /* unicens_request_card_values(ALSA_DEVICE_ID); */
    
OnErrorExit:
    AFB_NOTICE("Initializing HAL-MOST-UNICENS-BINDING done..");
    return err;
}

// This receive all event this binding subscribe to
PUBLIC void unicens_event_cb(const char *evtname, json_object *j_event) {

    if (strncmp(evtname, "alsacore/", 9) == 0) {
        halServiceEvent(evtname, j_event);
        return;
    }

    if (strncmp(evtname, "UNICENS/", 8) == 0) {
        //AFB_NOTICE("unicens_event_cb: evtname=%s, event=%s", evtname, json_object_get_string(j_event));
        if (strcmp(evtname, "UNICENS/node-availibility") == 0) {
            
            int node;
            int available;
            if (wrap_json_unpack(j_event, "{s:i,s:b}", "node", &node, "available", &available) == 0) {
                AFB_NOTICE("Node-Availability: node=0x%03X, available=%d", node, available);
                wrap_volume_node_avail(node, available);
            }
        }
        
        return;
    }
    
    AFB_NOTICE("unicens_event_cb: UNHANDLED EVENT, evtname=%s, event=%s", evtname, json_object_get_string(j_event));
}

/* API prefix should be unique for each snd card */
PUBLIC const struct afb_binding_v2 afbBindingV2 = {
    .api     = "hal-most-unicens",
    .init    = unicens_service_init,
    .verbs   = halServiceApi,
    .onevent = unicens_event_cb,
};
