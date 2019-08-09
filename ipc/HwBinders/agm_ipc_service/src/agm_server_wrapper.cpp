/*
 * Copyright (c) 2019, The Linux Foundation. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 *       copyright notice, this list of conditions and the following
 *       disclaimer in the documentation and/or other materials provided
 *       with the distribution.
 *     * Neither the name of The Linux Foundation nor the names of its
 *       contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
 * IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#define LOG_TAG "agm_server_wrapper"
#include "inc/agm_server_wrapper.h"
#include <log/log.h>
#include <cutils/list.h>
#include <pthread.h>

namespace vendor {
namespace qti {
namespace hardware {
namespace AGMIPC {
namespace V1_0 {
namespace implementation {

struct listnode clbk_data_list;
pthread_mutex_t clbk_data_list_lock;
bool clbk_data_list_init = false;

typedef struct clbk_data {
   struct listnode list;
   uint64_t clbk_clt_data;
   SrvrClbk *srv_clt_data;
}clbk_data;

void ipc_callback (uint32_t session_id,
                   struct agm_event_cb_params *evt_param,
                   void *client_data)
{
    ALOGV("%s called with sess_id = %d, client_data = %p \n", __func__,
          session_id, client_data);
    SrvrClbk *sr_clbk_dat;
    sr_clbk_dat = (SrvrClbk *) client_data;
    hidl_vec<AgmEventCbParams> evt_param_l;
    evt_param_l.resize(sizeof(struct agm_event_cb_params) +
                            evt_param->event_payload_size);
    evt_param_l.data()->source_module_id = evt_param->source_module_id;
    evt_param_l.data()->event_payload_size = evt_param->event_payload_size;
    evt_param_l.data()->event_id = evt_param->event_id;
    evt_param_l.data()->event_payload.resize(evt_param->event_payload_size);
    int8_t *dst = (int8_t *)evt_param_l.data()->event_payload.data();
    int8_t *src = (int8_t *)evt_param->event_payload;
    memcpy(dst, src, evt_param->event_payload_size);
    sp<IAGMCallback> clbk_bdr = sr_clbk_dat->clbk_binder;
    clbk_bdr->event_callback(session_id, evt_param_l,
                              sr_clbk_dat->get_clnt_data());
}
// Methods from ::vendor::qti::hardware::AGMIPC::V1_0::IAGM follow.
Return<int32_t> AGM::ipc_agm_init() {
    ALOGV("%s called \n", __func__);
    return 0;
}

Return<int32_t> AGM::ipc_agm_deinit() {
    ALOGV("%s called \n", __func__);
    return 0;
}

Return<int32_t> AGM::ipc_agm_aif_set_media_config(uint32_t aif_id,
                                 const hidl_vec<AgmMediaConfig>& media_config) {
    ALOGV("%s called with aif_id = %d \n", __func__, aif_id);
    struct agm_media_config *med_config_l = NULL;
    med_config_l =
          (struct agm_media_config*)calloc(1, sizeof(struct agm_media_config));
    if (med_config_l == NULL) {
        ALOGE("%s: Cannot allocate memory for med_config_l\n", __func__);
        return -ENOMEM;
    }
    memcpy(med_config_l, media_config.data(), sizeof(struct agm_media_config));
    int32_t ret = agm_aif_set_media_config (aif_id, med_config_l);
    return ret;
}

Return<int32_t> AGM::ipc_agm_aif_set_metadata(uint32_t aif_id,
                                            uint32_t size,
                                            const hidl_vec<uint8_t>& metadata) {
    ALOGV("%s called with aif_id = %d, size = %d\n", __func__, aif_id, size);
    uint8_t * metadata_l = NULL;
    metadata_l = (uint8_t *) calloc(1,size);
    if (metadata_l == NULL) {
        ALOGE("%s: Cannot allocate memory for metadata_l\n", __func__);
        return -ENOMEM;
    }
    memcpy(metadata_l, metadata.data(), size);
    return agm_aif_set_metadata(aif_id, size, metadata_l);
}

Return<int32_t> AGM::ipc_agm_session_set_metadata(uint32_t session_id,
                                            uint32_t size,
                                            const hidl_vec<uint8_t>& metadata) {
    ALOGV("%s : session_id = %d, size = %d\n", __func__, session_id, size);
    uint8_t * metadata_l = NULL;
    metadata_l = (uint8_t *) calloc(1,size);
    if (metadata_l == NULL) {
        ALOGE("%s: Cannot allocate memory for metadata_l\n", __func__);
        return -ENOMEM;
    }
    memcpy(metadata_l, metadata.data(), size);
    return agm_session_set_metadata(session_id, size, metadata_l);
}

Return<int32_t> AGM::ipc_agm_session_aif_set_metadata(uint32_t session_id,
                                            uint32_t aif_id,
                                            uint32_t size,
                                            const hidl_vec<uint8_t>& metadata) {
    ALOGV("%s : session_id = %d, aif_id =%d, size = %d\n", __func__,
                                                      session_id, aif_id, size);
    uint8_t * metadata_l = NULL;
    metadata_l = (uint8_t *) calloc(1,size);
    if (metadata_l == NULL) {
        ALOGE("%s: Cannot allocate memory for metadata_l\n", __func__);
        return -ENOMEM;
    }
    memcpy(metadata_l, metadata.data(), size);
    return agm_session_aif_set_metadata(session_id, aif_id, size, metadata_l);
}

Return<int32_t> AGM::ipc_agm_session_aif_connect(uint32_t session_id,
                                                 uint32_t aif_id,
                                                 bool state) {
    ALOGV("%s : session_id = %d, aif_id =%d, state = %s\n", __func__,
                          session_id, aif_id, state ? "true" : "false");
    return agm_session_aif_connect(session_id, aif_id, state);
}

Return<void> AGM::ipc_agm_session_aif_get_tag_module_info(uint32_t session_id,
                          uint32_t aif_id,
                          uint32_t size,
                          ipc_agm_session_aif_get_tag_module_info_cb _hidl_cb) {
    ALOGV("%s : session_id = %d, aif_id =%d, size = %d\n", __func__,
                                                      session_id, aif_id, size);
    uint8_t * payload_local = NULL;
    size_t size_local;
    size_local = (size_t) size;
    hidl_vec<uint8_t> payload_hidl;
    if (size_local) {
        payload_local = (uint8_t *) calloc (1, size_local);
        if (payload_local == NULL) {
            ALOGE("%s: Cannot allocate memory for payload_local\n", __func__);
            _hidl_cb(-ENOMEM, payload_hidl, size);
            return Void();
        }
    }
    int32_t ret = agm_session_aif_get_tag_module_info(session_id,
                                                      aif_id,
                                                      payload_local,
                                                      &size_local);
    payload_hidl.resize(size_local);
    if (payload_local)
        memcpy(payload_hidl.data(), payload_local, size_local);
    uint32_t size_hidl = (uint32_t) size_local;
    _hidl_cb(ret, payload_hidl, size_hidl);
    return Void();
}

Return<int32_t> AGM::ipc_agm_session_aif_set_params(uint32_t session_id,
                                               uint32_t aif_id,
                                               const hidl_vec<uint8_t>& payload,
                                               uint32_t size) {
    ALOGV("%s : session_id = %d, aif_id =%d, size = %d\n", __func__,
                                                      session_id, aif_id, size);
    size_t size_local = (size_t) size;
    void * payload_local = NULL;
    payload_local = (void*) calloc (1,size);
    if (payload_local == NULL) {
        ALOGE("%s: Cannot allocate memory for payload_local\n", __func__);
        return -ENOMEM;
    }
    memcpy(payload_local, payload.data(), size);
    return agm_session_aif_set_params(session_id,
                                      aif_id,
                                      payload_local,
                                      size_local);
}

Return<int32_t> AGM::ipc_agm_session_aif_set_cal(uint32_t session_id,
                                    uint32_t aif_id,
                                    const hidl_vec<AgmCalConfig>& cal_config) {
    ALOGV("%s : session_id = %d, aif_id = %d\n", __func__, session_id, aif_id);
    struct agm_cal_config *cal_config_local = NULL;
    cal_config_local =
              (struct agm_cal_config*) calloc(1, sizeof(struct agm_cal_config));
    if (cal_config_local == NULL) {
            ALOGE("%s: Cannot allocate memory for cal_config_local\n", __func__);
            return -ENOMEM;
    }
    cal_config_local->num_ckvs = cal_config.data()->num_ckvs;
    AgmKeyValue * ptr = NULL;
    for (int i=0 ; i < cal_config.data()->num_ckvs ; i++ ) {
        ptr = (AgmKeyValue *) (cal_config.data() +
                                             sizeof(struct agm_cal_config) +
                                             (sizeof(AgmKeyValue)*i));
        cal_config_local->kv[i].key = ptr->key;
        cal_config_local->kv[i].value = ptr->value;
    }
    return agm_session_aif_set_cal(session_id, aif_id, cal_config_local);
}

Return<int32_t> AGM::ipc_agm_session_set_params(uint32_t session_id,
                                               const hidl_vec<uint8_t>& payload,
                                               uint32_t size) {
    ALOGV("%s : session_id = %d, size = %d\n", __func__, session_id, size);
    size_t size_local = (size_t) size;
    void * payload_local = NULL;
    payload_local = (void*) calloc (1,size);
    if (payload_local == NULL) {
        ALOGE("%s: Cannot allocate memory for payload_local\n", __func__);
        return -ENOMEM;
    }
    memcpy(payload_local, payload.data(), size);
    return agm_session_set_params(session_id, payload_local, size_local);
}

Return<int32_t> AGM::ipc_agm_set_params_with_tag(uint32_t session_id,
                                     uint32_t aif_id,
                                     const hidl_vec<AgmTagConfig>& tag_config) {
    ALOGV("%s : session_id = %d, aif_id = %d\n", __func__, session_id, aif_id);
    struct agm_tag_config *tag_config_local;
    size_t size_local = (sizeof(struct agm_tag_config) +
                        (tag_config.data()->num_tkvs) * sizeof(agm_key_value));
    tag_config_local = (struct agm_tag_config *) calloc(1,size_local);
    if (tag_config_local == NULL) {
        ALOGE("%s: Cannot allocate memory for tag_config_local\n", __func__);
        return -ENOMEM;
    }
    tag_config_local->num_tkvs = tag_config.data()->num_tkvs;
    tag_config_local->tag = tag_config.data()->tag;
    AgmKeyValue * ptr = NULL;
    for (int i=0 ; i < tag_config.data()->num_tkvs ; i++ ) {
        ptr = (AgmKeyValue *) (tag_config.data() +
                                             sizeof(struct agm_tag_config) +
                                             (sizeof(AgmKeyValue)*i));
        tag_config_local->kv[i].key = ptr->key;
        tag_config_local->kv[i].value = ptr->value;
    }
    return agm_set_params_with_tag(session_id, aif_id, tag_config_local) ;
}

Return<int32_t> AGM::ipc_agm_session_register_for_events(uint32_t session_id,
                                  const hidl_vec<AgmEventRegCfg>& evt_reg_cfg) {
    ALOGV("%s : session_id = %d\n", __func__, session_id);
    struct agm_event_reg_cfg *evt_reg_cfg_local;
    evt_reg_cfg_local = (struct agm_event_reg_cfg*)
              calloc(1,(sizeof(struct agm_event_reg_cfg) +
              (evt_reg_cfg.data()->event_config_payload_size)*sizeof(uint8_t)));
    if (evt_reg_cfg_local == NULL) {
        ALOGE("%s: Cannot allocate memory for evt_reg_cfg_local\n", __func__);
        return -ENOMEM;
    }
    memcpy(evt_reg_cfg_local, evt_reg_cfg.data(),
           sizeof(struct agm_event_reg_cfg) +
           (evt_reg_cfg.data()->event_config_payload_size)*sizeof(uint8_t));
    return agm_session_register_for_events(session_id, evt_reg_cfg_local);
}

Return<void> AGM::ipc_agm_session_open(uint32_t session_id,
                                            ipc_agm_session_open_cb _hidl_cb) {
    ALOGV("%s : session_id = %d\n", __func__, session_id);
    void *handle = NULL;
    int32_t ret = agm_session_open(session_id, &handle);
    hidl_vec<uint64_t> handle_ret;
    handle_ret.resize(sizeof(uint64_t));
    *handle_ret.data() = (uint64_t) handle;
    _hidl_cb(ret, handle_ret);
    return Void();
}

Return<int32_t> AGM::ipc_agm_session_set_config(uint64_t hndl,
                const hidl_vec<AgmSessionConfig>& session_config,
                const hidl_vec<AgmMediaConfig>& media_config,
                const hidl_vec<AgmBufferConfig>& buffer_config) {
    ALOGV("%s called with handle = %lu \n", __func__, hndl);

    struct agm_media_config *media_config_local = NULL;
    media_config_local = (struct agm_media_config*)
                                    calloc(1, sizeof(struct agm_media_config));
    if (media_config_local == NULL) {
        ALOGE("%s: Cannot allocate memory for media_config_local\n", __func__);
        return -ENOMEM;
    }
    memcpy(media_config_local,
           media_config.data(),
           sizeof(struct agm_media_config));

    struct agm_session_config *session_config_local = NULL;
    session_config_local = (struct agm_session_config*)
                                  calloc(1, sizeof(struct agm_session_config));
    if (session_config_local == NULL) {
        ALOGE("%s: Cannot allocate memory for session_config_local\n", __func__);
        return -ENOMEM;
    }
    memcpy(session_config_local,
           session_config.data(),
           sizeof(struct agm_session_config));

    struct agm_buffer_config *buffer_config_local = NULL;
    buffer_config_local = (struct agm_buffer_config*)
                                   calloc(1, sizeof(struct agm_buffer_config));
    if (buffer_config_local == NULL) {
        ALOGE("%s: Cannot allocate memory for buffer_config_local\n", __func__);
        return -ENOMEM;
    }
    memcpy(buffer_config_local,
           buffer_config.data(),
           sizeof(struct agm_buffer_config));

    void *handle = (void *) hndl;
    return agm_session_set_config(handle,
                                  session_config_local,
                                  media_config_local,
                                  buffer_config_local);
}

Return<int32_t> AGM::ipc_agm_session_close(uint64_t hndl) {
    ALOGV("%s called with handle = %lu \n", __func__, hndl);
    void *handle = (void *) hndl;
    return agm_session_close(handle);
}

Return<int32_t> AGM::ipc_agm_session_prepare(uint64_t hndl) {
    ALOGV("%s called with handle = %lu \n", __func__, hndl);
    void *handle = (void *) hndl;
    return agm_session_prepare(handle);
}

Return<int32_t> AGM::ipc_agm_session_start(uint64_t hndl) {
    ALOGV("%s called with handle = %lu \n", __func__, hndl);
    void *handle = (void *) hndl;
    return agm_session_start(handle);
}

Return<int32_t> AGM::ipc_agm_session_stop(uint64_t hndl) {
    ALOGV("%s called with handle = %lu \n", __func__, hndl);
    void *handle = (void *) hndl;
    return agm_session_stop(handle);
}

Return<int32_t> AGM::ipc_agm_session_pause(uint64_t hndl) {
    ALOGV("%s called with handle = %lu \n", __func__, hndl);
    void *handle = (void *) hndl;
    return agm_session_pause(handle);
}
Return<int32_t> AGM::ipc_agm_session_resume(uint64_t hndl) {
    ALOGV("%s called with handle = %lu \n", __func__, hndl);
    void *handle = (void *) hndl;
    return agm_session_resume(handle);
}

Return<void> AGM::ipc_agm_session_read(uint64_t hndl, uint32_t count,
                                             ipc_agm_session_read_cb _hidl_cb) {
    ALOGV("%s called with handle = %lu \n", __func__, hndl);
    hidl_vec <uint8_t> buff_ret;
    void *handle = (void *) hndl;
    void *buffer = NULL;
    buffer = (void*) calloc(1,count);
    if (buffer == NULL) {
        ALOGE("%s: Cannot allocate memory for buffer\n", __func__);
        _hidl_cb (-ENOMEM, buff_ret, count);
        return Void();
    }
    size_t cnt = (size_t) count;
    int ret = agm_session_read(handle, buffer, &cnt);
    buff_ret.resize(count);
    memcpy(buff_ret.data(), buffer, count);
    _hidl_cb (ret, buff_ret, cnt);
    return Void();
}

Return<void> AGM::ipc_agm_session_write(uint64_t hndl,
                                        const hidl_vec<uint8_t>& buff,
                                        uint32_t count,
                                        ipc_agm_session_write_cb _hidl_cb) {
    ALOGV("%s called with handle = %lu \n", __func__, hndl);
    void *handle = (void *) hndl;
    void* buffer = NULL;
    buffer = (void*) calloc(1,count);
    if (buffer == NULL) {
        ALOGE("%s: Cannot allocate memory for buffer\n", __func__);
        _hidl_cb (-ENOMEM, count);
        return Void();
    }
    memcpy(buffer, buff.data(), count);
    size_t cnt = (size_t) count;
    int ret = agm_session_write(handle, buffer, &cnt);
    _hidl_cb (ret, cnt);
    return Void();
}

Return<int32_t> AGM::ipc_agm_get_hw_processed_buff_cnt(uint64_t hndl,
                                                        Direction dir) {
    ALOGV("%s called with handle = %lu \n", __func__, hndl);
    void *handle = (void *) hndl;
    enum direction dir_local = (enum direction) dir;
    return agm_get_hw_processed_buff_cnt(handle, dir_local);
}

Return<void> AGM::ipc_agm_get_aif_info_list(uint32_t num_aif_info,
                                       ipc_agm_get_aif_info_list_cb _hidl_cb) {
    ALOGV("%s called with num_aif_info = %d\n", __func__, num_aif_info);
    int32_t ret;
    hidl_vec<AifInfo> aif_list_ret;
    struct aif_info * aif_list = NULL;
    if (num_aif_info != 0) {
        aif_list = (struct aif_info*)
                            calloc(1,(sizeof(struct aif_info) * num_aif_info));
        if (aif_list == NULL) {
            ALOGE("%s: Cannot allocate memory for aif_list\n", __func__);
            _hidl_cb(-ENOMEM, aif_list_ret, num_aif_info);
            return Void();
        }
    }
    size_t num_aif_info_ret = (size_t) num_aif_info;
    ret = agm_get_aif_info_list(aif_list, &num_aif_info_ret);
    aif_list_ret.resize(sizeof(struct aif_info) * num_aif_info);
    if ( aif_list != NULL) {
        for (int i=0 ; i<num_aif_info ; i++) {
            aif_list_ret.data()[i].aif_name = aif_list[i].aif_name;
            aif_list_ret.data()[i].dir = (Direction) aif_list[i].dir;
        }
    }
    num_aif_info = (uint32_t) num_aif_info_ret;
    ret = 0;
    _hidl_cb(ret, aif_list_ret, num_aif_info);
    return Void();
}
Return<int32_t> AGM::ipc_agm_session_set_loopback(uint32_t capture_session_id,
                                                  uint32_t playback_session_id,
                                                  bool state) {
    ALOGV("%s called capture_session_id = %d, playback_session_id = %d\n", __func__,
           capture_session_id, playback_session_id);
    return agm_session_set_loopback(capture_session_id,
                                    playback_session_id,
                                    state);
}


Return<int32_t> AGM::ipc_agm_session_set_ec_ref(uint32_t capture_session_id,
                                                uint32_t aif_id, bool state) {
    ALOGV("%s : cap_sess_id = %d, aif_id = %d\n", __func__,
                                  capture_session_id, aif_id);
    return agm_session_set_ec_ref(capture_session_id, aif_id, state);
}


Return<int32_t> AGM::ipc_agm_session_register_callback(uint32_t session_id,
                                                     const sp<IAGMCallback>& cb,
                                                     uint32_t evt_type,
                                                     uint64_t ipc_client_data,
                                                     uint64_t clnt_data) {
    agm_event_cb ipc_cb;
    SrvrClbk  *sr_clbk_data, *tmp_sr_clbk_data = NULL;
    clbk_data *clbk_data_obj = NULL;
    if (cb != NULL) {
        sr_clbk_data = new SrvrClbk (session_id, cb, evt_type, ipc_client_data);
        ALOGV("%s new SrvrClbk= %p, clntdata= %p, sess id= %d, evt_type= %d \n",
                __func__, (void *) sr_clbk_data,ipc_client_data,session_id,
                (uint32_t)evt_type);
        /*TODO: Free this clbk list when the client dies abruptly also
                deregister the callbacks*/
        if (clbk_data_list_init == false) {
            pthread_mutex_init(&clbk_data_list_lock,
            (const pthread_mutexattr_t *) NULL);
            list_init(&clbk_data_list);
            clbk_data_list_init = true;
        }
        clbk_data_obj = (clbk_data *)calloc(1, sizeof(clbk_data));
        if (clbk_data_obj == NULL) {
            ALOGE("%s: Cannot allocate memory for cb data object\n", __func__);
            return -ENOMEM;
        }
        pthread_mutex_lock(&clbk_data_list_lock);
        clbk_data_obj->clbk_clt_data = clnt_data;
        clbk_data_obj->srv_clt_data = sr_clbk_data;
        list_add_tail(&clbk_data_list, &clbk_data_obj->list);
        pthread_mutex_unlock(&clbk_data_list_lock);
        ipc_cb = &ipc_callback;
    } else {
        /*
         *This condition indicates that the client wants to deregister the
         *callback. Hence we pass the callback as NULL to AGM and also the
         *client data should match with the one that was used to register
         *with AGM.
         */
        struct listnode *node = NULL;
        pthread_mutex_lock(&clbk_data_list_lock);
        list_for_each(node, &clbk_data_list) {
            clbk_data_obj = node_to_item(node, clbk_data, list);
            tmp_sr_clbk_data = clbk_data_obj->srv_clt_data;
            if ((tmp_sr_clbk_data->session_id == session_id) &&
                (tmp_sr_clbk_data->event == evt_type) &&
                (clbk_data_obj->clbk_clt_data == clnt_data))
                sr_clbk_data = tmp_sr_clbk_data;
        }
        pthread_mutex_unlock(&clbk_data_list_lock);
        ipc_cb = NULL;
    }
    if (sr_clbk_data == NULL) {
        ALOGV("server callback data is NULL");
        return -ENOMEM;
    }
    return agm_session_register_cb(session_id,
                                   ipc_cb,
                                   (enum event_type) evt_type,
                                   (void *) sr_clbk_data);
}

Return<int32_t> AGM::ipc_agm_session_eos(uint64_t hndl){
    ALOGV("%s : handle = %lu \n", __func__, hndl);
    void *handle = (void *) hndl;
    return agm_session_eos(handle);
}

Return<void> AGM::ipc_agm_get_session_time(uint64_t hndl,
                                          ipc_agm_get_session_time_cb _hidl_cb){
    ALOGV("%s : handle = %lu \n", __func__, hndl);
    void *handle = (void *) hndl;
    uint64_t ts;
    int ret = agm_get_session_time(handle, &ts);
    _hidl_cb(ret,ts);
    return Void();
}

}  // namespace implementation
}  // namespace V1_0
}  // namespace AGMIPC
}  // namespace hardware
}  // namespace qti
}  // namespace vendor
