/* Copyright (c) 2011-2013, The Linux Foundation. All rights reserved.
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
 *     * Neither the name of The Linux Foundation, nor the names of its
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
 *
 */
#ifndef LOC_API_BASE_H
#define LOC_API_BASE_H

#include <stddef.h>
#include <ctype.h>
#include <gps_extended.h>
#include <MsgTask.h>
#include <log_util.h>

namespace loc_core {

int hexcode(char *hexstring, int string_size,
            const char *data, int data_size);
int decodeAddress(char *addr_string, int string_size,
                  const char *data, int data_size);

#define MAX_ADAPTERS          10

#define TO_ALL_ADAPTERS(adapters, call)                                \
    for (int i = 0; i < MAX_ADAPTERS && NULL != (adapters)[i]; i++) {  \
        call;                                                          \
    }

#define TO_1ST_HANDLING_ADAPTER(adapters, call)                              \
    for (int i = 0; i <MAX_ADAPTERS && NULL != (adapters)[i] && !(call); i++);

class LocAdapterBase;
struct LocSsrMsg;
struct LocOpenMsg;

class LocApiProxyBase {
public:
    inline LocApiProxyBase() {}
    inline virtual ~LocApiProxyBase() {}
    inline virtual void* getSibling2() { return NULL; }
};

class LocApiBase {
    friend struct LocSsrMsg;
    //LocOpenMsg calls open() which makes it necessary to declare
    //it as a friend
    friend struct LocOpenMsg;
    friend class ContextBase;
    const MsgTask* mMsgTask;
    LocAdapterBase* mLocAdapters[MAX_ADAPTERS];

protected:
    virtual enum loc_api_adapter_err
        open(LOC_API_ADAPTER_EVENT_MASK_T mask);
    virtual enum loc_api_adapter_err
        close();
    LOC_API_ADAPTER_EVENT_MASK_T getEvtMask();
    LOC_API_ADAPTER_EVENT_MASK_T mMask;
    LocApiBase(const MsgTask* msgTask,
               LOC_API_ADAPTER_EVENT_MASK_T excludedMask);
    inline virtual ~LocApiBase() { close(); }
    bool isInSession();
    const LOC_API_ADAPTER_EVENT_MASK_T mExcludedMask;

public:
    inline void sendMsg(const LocMsg* msg) const {
        mMsgTask->sendMsg(msg);
    }

    void addAdapter(LocAdapterBase* adapter);
    void removeAdapter(LocAdapterBase* adapter);

    // upward calls
    void handleEngineUpEvent();
    void handleEngineDownEvent();
    void reportPosition(UlpLocation &location,
                        GpsLocationExtended &locationExtended,
                        void* locationExt,
                        enum loc_sess_status status,
                        LocPosTechMask loc_technology_mask =
                                  LOC_POS_TECH_MASK_DEFAULT);
    void reportSv(GpsSvStatus &svStatus,
                  GpsLocationExtended &locationExtended,
                  void* svExt);
    void reportStatus(GpsStatusValue status);
    void reportNmea(const char* nmea, int length);
    void reportXtraServer(const char* url1, const char* url2,
                          const char* url3, const int maxlength);
    void requestXtraData();
    void requestTime();
    void requestLocation();
    void requestATL(int connHandle, AGpsType agps_type);
    void releaseATL(int connHandle);
    void requestSuplES(int connHandle);
    void reportDataCallOpened();
    void reportDataCallClosed();
    void requestNiNotify(GpsNiNotification &notify, const void* data);

    // downward calls
    // All below functions are to be defined by adapter specific modules:
    // RPC, QMI, etc.  The default implementation is empty.

    virtual void* getSibling();
    virtual LocApiProxyBase* getLocApiProxy();
    virtual enum loc_api_adapter_err
        startFix(const LocPosMode& posMode);
    virtual enum loc_api_adapter_err
        stopFix();
    virtual enum loc_api_adapter_err
        deleteAidingData(GpsAidingData f);
    virtual enum loc_api_adapter_err
        enableData(int enable);
    virtual enum loc_api_adapter_err
        setAPN(char* apn, int len);
    virtual enum loc_api_adapter_err
        injectPosition(double latitude, double longitude, float accuracy);
    virtual enum loc_api_adapter_err
        setTime(GpsUtcTime time, int64_t timeReference, int uncertainty);
    virtual enum loc_api_adapter_err
        setXtraData(char* data, int length);
    virtual enum loc_api_adapter_err
        requestXtraServer();
    virtual enum loc_api_adapter_err
        atlOpenStatus(int handle, int is_succ, char* apn, AGpsBearerType bear, AGpsType agpsType);
    virtual enum loc_api_adapter_err
        atlCloseStatus(int handle, int is_succ);
    virtual enum loc_api_adapter_err
        setPositionMode(const LocPosMode& posMode);
    virtual enum loc_api_adapter_err
        setServer(const char* url, int len);
    virtual enum loc_api_adapter_err
        setServer(unsigned int ip, int port,
                  LocServerType type);
    virtual enum loc_api_adapter_err
        informNiResponse(GpsUserResponseType userResponse, const void* passThroughData);
    virtual enum loc_api_adapter_err
        setSUPLVersion(uint32_t version);
    virtual enum loc_api_adapter_err
        setLPPConfig(uint32_t profile);
    virtual enum loc_api_adapter_err
        setSensorControlConfig(int sensorUsage);
    virtual enum loc_api_adapter_err
        setSensorProperties(bool gyroBiasVarianceRandomWalk_valid,
                            float gyroBiasVarianceRandomWalk,
                            bool accelBiasVarianceRandomWalk_valid,
                            float accelBiasVarianceRandomWalk,
                            bool angleBiasVarianceRandomWalk_valid,
                            float angleBiasVarianceRandomWalk,
                            bool rateBiasVarianceRandomWalk_valid,
                            float rateBiasVarianceRandomWalk,
                            bool velocityBiasVarianceRandomWalk_valid,
                            float velocityBiasVarianceRandomWalk);
    virtual enum loc_api_adapter_err
        setSensorPerfControlConfig(int controlMode,
                               int accelSamplesPerBatch,
                               int accelBatchesPerSec,
                               int gyroSamplesPerBatch,
                               int gyroBatchesPerSec,
                               int accelSamplesPerBatchHigh,
                               int accelBatchesPerSecHigh,
                               int gyroSamplesPerBatchHigh,
                               int gyroBatchesPerSecHigh,
                               int algorithmConfig);
    virtual enum loc_api_adapter_err
        setExtPowerConfig(int isBatteryCharging);
    virtual enum loc_api_adapter_err
        setAGLONASSProtocol(unsigned long aGlonassProtocol);
    virtual enum loc_api_adapter_err
        getZppFix(GpsLocation & zppLoc);
    virtual enum loc_api_adapter_err
        getZppFix(GpsLocation & zppLoc, LocPosTechMask & tech_mask);
    virtual int initDataServiceClient();
    virtual int openAndStartDataCall();
    virtual void stopDataCall();
    virtual void closeDataCall();

    inline virtual void setInSession(bool inSession) {}

    /*Values for lock
      1 = Do not lock any position sessions
      2 = Lock MI position sessions
      3 = Lock MT position sessions
      4 = Lock all position sessions
     */
    virtual int setGpsLock(unsigned int lock);
    /*
      Returns
      Current value of GPS Lock on success
      -1 on failure
     */
    virtual int getGpsLock(void);
};

typedef LocApiBase* (getLocApi_t)(const MsgTask* msgTask,
                                  LOC_API_ADAPTER_EVENT_MASK_T exMask);

} // namespace loc_core

#endif //LOC_API_BASE_H
