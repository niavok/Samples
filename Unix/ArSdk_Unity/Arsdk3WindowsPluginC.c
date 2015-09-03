#include <stdio.h>
#include <stdint.h>
#include <stdarg.h>

#ifdef WIN32

#include <stdint.h>
typedef uint8_t u_int8_t;
typedef uint16_t u_int16_t;
typedef uint32_t u_int32_t;
#endif

#include <libARSAL/ARSAL.h>

#include <libARController/ARController.h>
#include <libARDiscovery/ARDiscovery.h>
#include <libARCommands/ARCommands.h>
#include <libARController/ARCONTROLLER_Feature.h>
#include "UnityMultiplatformSendMessage.h"
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>

// called when the state of the device controller has changed
void stateChanged (eARCONTROLLER_DEVICE_STATE newState, eARCONTROLLER_ERROR error, void *customData);

extern void didReceiveFrameCallback (ARCONTROLLER_Frame_t *frame, void *customData);
extern void * allocateVideoHarwareController (void);

// IHM updates from commands
void batteryStateChanged (uint8_t percent);

void registerCallbacks (ARCONTROLLER_Device_t* deviceController);



#define ERROR_STR_LENGTH 2048

#define JS_IP_ADDRESS "192.168.42.1"
#define JS_DISCOVERY_PORT 44444

#define TAG "SDK3UnityPlugin"


struct ARdrone3Plugin {
	ARCONTROLLER_Device_t *deviceController;
    AVPacket avpkt;
    AVCodec *codec;
    AVFrame *picture;
    uint8_t *rgbData;
    AVCodecContext *codecContext;
    struct SwsContext* swsContext;
    int frameCount;
};

void SetTextureFromUnity (void* texturePtr)
{
	ARSAL_PRINT(ARSAL_PRINT_INFO, TAG, "UnityRenderEvent %p", texturePtr);
}

void UnityRenderEvent (int eventId, int width, int height)
{
	ARSAL_PRINT(ARSAL_PRINT_INFO, TAG, "UnityRenderEvent eventId=%d width=%d height=%d", eventId, width, height);
}

typedef void (* UnityVideoCallback) (char * data, int width, int height);


static UnityVideoCallback lastVideoCallback = NULL;

void ARdrone3_connectVideoCallback(UnityVideoCallback videoCallbackMethod) {
    ARSAL_PRINT(ARSAL_PRINT_INFO, TAG, "ARdrone3_connectVideoCallback");
    lastVideoCallback = videoCallbackMethod;
}

void SendFrame (char * data, int width, int height) {
    ARSAL_PRINT(ARSAL_PRINT_INFO, TAG, "SendFrame %p %d %d", data, width, height);
    if (lastVideoCallback != NULL) {
        ARSAL_PRINT(ARSAL_PRINT_INFO, TAG, "Callback ok");
        lastVideoCallback( data, width, height);
        ARSAL_PRINT(ARSAL_PRINT_INFO, TAG, "Callback done");
    }
}

void didReceiveFrameCallback (ARCONTROLLER_Frame_t *inputFrame, void *customData)
{
    ARSAL_PRINT(ARSAL_PRINT_INFO, TAG, "didReceiveFrameCallback");
    struct ARdrone3Plugin *self = (struct ARdrone3Plugin *) customData;

    if(!self->codecContext || !self->codec)
    {
        ARSAL_PRINT(ARSAL_PRINT_INFO, TAG, "Decoder not initialized");
        return;
    }

    ARSAL_PRINT(ARSAL_PRINT_INFO, TAG, "Decoder ok");

    self->avpkt.size = inputFrame->used;
    self->avpkt.data = inputFrame->data;

    int len, got_frame;
    char buf[1024];

    self->frameCount++;
    ARSAL_PRINT(ARSAL_PRINT_INFO, TAG, "avcodec_decode_video2 %d", self->frameCount);
    len = avcodec_decode_video2(self->codecContext, self->picture, &got_frame, &self->avpkt);
    if (len < 0) {
        ARSAL_PRINT(ARSAL_PRINT_ERROR, TAG, "Error while decoding frame %d\n", self->frameCount);
    }
    else if (got_frame) {
        ARSAL_PRINT(ARSAL_PRINT_INFO, TAG, "Decode frame %d success", self->frameCount);
        ARSAL_PRINT(ARSAL_PRINT_INFO, TAG, " - len = %d", len);
        ARSAL_PRINT(ARSAL_PRINT_INFO, TAG, " - picture->linesize[0] = %d", self->picture->linesize[0]);
        ARSAL_PRINT(ARSAL_PRINT_INFO, TAG, " - codecContext->width = %d", self->codecContext->width);
        ARSAL_PRINT(ARSAL_PRINT_INFO, TAG, " - codecContext->height = %d", self->codecContext->height);
        ARSAL_PRINT(ARSAL_PRINT_INFO, TAG, " - picture->format = %d", self->picture->format);

        switch(self->picture->format)
        {
            case AV_PIX_FMT_YUV420P:
                ARSAL_PRINT(ARSAL_PRINT_INFO, TAG, " - AV_PIX_FMT_YUV420P");
                break;
            case AV_PIX_FMT_YUYV422:
                ARSAL_PRINT(ARSAL_PRINT_INFO, TAG, " - AV_PIX_FMT_YUYV422");
                break;
            case AV_PIX_FMT_RGB24:
                ARSAL_PRINT(ARSAL_PRINT_INFO, TAG, " - AV_PIX_FMT_RGB24");
                break;
            case AV_PIX_FMT_NV12:
                ARSAL_PRINT(ARSAL_PRINT_INFO, TAG, " - AV_PIX_FMT_NV12");
                break;
            default:
                ARSAL_PRINT(ARSAL_PRINT_INFO, TAG, " - Unknown format %d", self->picture->format);
        }

       
        self->swsContext = sws_getCachedContext(self->swsContext, self->codecContext->width, self->codecContext->height,
              self->picture->format, self->codecContext->width, self->codecContext->height,
              AV_PIX_FMT_RGB24, 0, 0, 0, 0);

        if(!self->swsContext)
        {
            ARSAL_PRINT(ARSAL_PRINT_INFO, TAG, "Fail to initialiaze filter");
            return;
        }


        if(!self->rgbData)
        {
            self->rgbData = malloc(self->codecContext->width * self->codecContext->height * 3);
        }



        uint8_t * outData[1] = { self->rgbData }; // RGB24 have one plane
        int outLinesize[1] = { 3*self->codecContext->width }; // RGB stride

        const uint8_t* const* inData = (const uint8_t* const*) self->picture->data;
        
        sws_scale(self->swsContext, inData, self->picture->linesize, 0, self->codecContext->height, outData, outLinesize);
        
        SendFrame(self->rgbData, self->codecContext->width, self->codecContext->height);
    }
}

FILE* logFile = NULL;

int PrintLog (eARSAL_PRINT_LEVEL level, const char *tag, const char *format, va_list va)
{
	if(logFile == NULL)
	{
		logFile = fopen("Arsdk.log", "w");
	}
	vfprintf(logFile, format, va);
	fflush(logFile);
}

struct ARdrone3Plugin* ARdrone3_Connect() {
    ARSAL_Print_SetCallback(PrintLog);
    ARSAL_Print_SetMinimumLevel(ARSAL_PRINT_VERBOSE);
    ARSAL_PRINT(ARSAL_PRINT_INFO, TAG, "ARdrone3_Connect");
    
    // local declarations
    int failed = 0;
    struct ARdrone3Plugin* self = malloc(sizeof(struct ARdrone3Plugin));
    ARDISCOVERY_Device_t *device = NULL;
    ARCONTROLLER_Device_t *deviceController = NULL;
    eARCONTROLLER_ERROR error = ARCONTROLLER_OK;
    self->codec = NULL;
    self->picture = NULL;
    self->codecContext = NULL;
    self->swsContext = NULL;
    self->rgbData = NULL;
     self->frameCount = 0;
    //ARSAL_Sem_Init (&(self->stateSem), 0, 0);
    
    // create a discovery device
    if (!failed)
    {
        ARSAL_PRINT(ARSAL_PRINT_INFO, TAG, "- init discovery device ... ");
        eARDISCOVERY_ERROR errorDiscovery = ARDISCOVERY_OK;
        
        
        
        device = ARDISCOVERY_Device_New (&errorDiscovery);
        
        if (errorDiscovery == ARDISCOVERY_OK)
        {
            ARSAL_PRINT(ARSAL_PRINT_INFO, TAG, "    - ARDISCOVERY_Device_InitWifi ...");
            // create a JumpingSumo discovery device (ARDISCOVERY_PRODUCT_JS)
            errorDiscovery = ARDISCOVERY_Device_InitWifi (device, ARDISCOVERY_PRODUCT_ARDRONE, "JS", JS_IP_ADDRESS, JS_DISCOVERY_PORT);
            
            if (errorDiscovery != ARDISCOVERY_OK)
            {
                failed = 1;
                ARSAL_PRINT(ARSAL_PRINT_ERROR, TAG, "Discovery error :%s", ARDISCOVERY_Error_ToString(errorDiscovery));
            }
        }
        else
        {
            ARSAL_PRINT(ARSAL_PRINT_ERROR, TAG, "Discovery error :%s", ARDISCOVERY_Error_ToString(errorDiscovery));
            failed = 1;
        }
    }
    
    // create a device controller
    if (!failed)
    {
        deviceController = ARCONTROLLER_Device_New (device, &error);
        
        //UnitySendMessage("DeviceController", "onServiceFound", "");
        
        if (error != ARCONTROLLER_OK)
        {
            ARSAL_PRINT (ARSAL_PRINT_ERROR, TAG, "Creation of deviceController failed.");
            failed = 1;
        }
        else
        {
            self->deviceController = deviceController;
        }
    }
    
    if (!failed)
    {
        ARSAL_PRINT(ARSAL_PRINT_INFO, TAG, "- delete discovey device ... ");
        ARDISCOVERY_Device_Delete (&device);
    }
    
    // add the state change callback to be informed when the device controller starts, stops...
    if (!failed)
    {
        error = ARCONTROLLER_Device_AddStateChangedCallback (deviceController, stateChanged, self);
        
        if (error != ARCONTROLLER_OK)
        {
            ARSAL_PRINT (ARSAL_PRINT_ERROR, TAG, "add State callback failed.");
            failed = 1;
        }
    }

    // add the frame received callback to be informed when a streaming frame has been received from the device
    ARSAL_PRINT(ARSAL_PRINT_INFO, TAG, "- set Video callback ... ");
    error = ARCONTROLLER_Device_SetVideoReceiveCallback (self->deviceController, didReceiveFrameCallback, NULL , self);
    
    if (error != ARCONTROLLER_OK)
    {
        failed = 1;
        ARSAL_PRINT(ARSAL_PRINT_ERROR, TAG, "- error :%", ARCONTROLLER_Error_ToString(error));
    }
    
    if (!failed)
    {
        ////////IHM_PrintInfo(ihm, "Connecting ...");
        error = ARCONTROLLER_Device_Start (deviceController);
        
        if (error != ARCONTROLLER_OK)
        {
            failed = 1;
            ARSAL_PRINT(ARSAL_PRINT_ERROR, TAG, "- error :%s", ARCONTROLLER_Error_ToString(error));
        }
    }
    registerCallbacks(self->deviceController);
    
    ARSAL_PRINT(ARSAL_PRINT_INFO, TAG, "Connect done");
 
    
    return self;
}

void ARdrone3_Disconnect(struct ARdrone3Plugin *self) {
    
    //ARCONTROLLER_Device_t *deviceController = NULL;
    eARCONTROLLER_ERROR error = ARCONTROLLER_OK;
    eARCONTROLLER_DEVICE_STATE deviceState = ARCONTROLLER_DEVICE_STATE_MAX;
    //ARSAL_Sem_Init (&(self->stateSem), 0, 0);

    ARdrone3_connectCallback(NULL);

    //deviceController->aRDrone3->sendCameraOrientation();
    deviceState = ARCONTROLLER_Device_GetState (self->deviceController, &error);
    ARSAL_PRINT(ARSAL_PRINT_INFO, TAG, "ARCONTROLLER_Device_GetState deviceState=%d error=%d", deviceState, error);
    if ((error == ARCONTROLLER_OK) && (deviceState != ARCONTROLLER_DEVICE_STATE_STOPPED))
    {
        //////IHM_PrintInfo(ihm, "Disconnecting ...");

        ARSAL_PRINT(ARSAL_PRINT_INFO, TAG, "ARCONTROLLER_Device_Stop");
        error = ARCONTROLLER_Device_Stop (self->deviceController);
        
        if (error == ARCONTROLLER_OK)
        {
            // wait state update update 
            //ARSAL_Sem_Wait (&(self->stateSem));
        }
    }

    ARSAL_PRINT(ARSAL_PRINT_INFO, TAG, "-- ARdrone3_Disconnect END --");
    
}

void ARDrone3_StartVideo(struct ARdrone3Plugin* self) {
    
    eARCONTROLLER_ERROR error = ARCONTROLLER_OK;
    
    ARSAL_PRINT(ARSAL_PRINT_INFO, TAG, "ARDrone3_StartVideo ... ");
    error = self->deviceController->aRDrone3->sendMediaStreamingVideoEnable (self->deviceController->aRDrone3, 1);
    if (error != ARCONTROLLER_OK)
    {
        ARSAL_PRINT(ARSAL_PRINT_ERROR, TAG, "- error :%s", ARCONTROLLER_Error_ToString(error));
    }
    
    self->frameCount = 0;

    av_init_packet(&self->avpkt);

    // register all the codecs
    avcodec_register_all();

    // find the h264 video decoder
    self->codec = avcodec_find_decoder(AV_CODEC_ID_H264);    
    if (!self->codec) {
        ARSAL_PRINT(ARSAL_PRINT_ERROR, TAG, "H264 Codec not found\n");
        return;
    }

    self->codecContext = avcodec_alloc_context3(self->codec);
    self->picture= av_frame_alloc();

    // open it
     if (avcodec_open2(self->codecContext, self->codec, NULL) < 0) {
        ARSAL_PRINT(ARSAL_PRINT_ERROR, TAG, "Could not open codec\n");
        return;
    }


    
    ARSAL_PRINT(ARSAL_PRINT_INFO, TAG, "ARDrone3_StartVideo ... done");

}


void ARDrone3_refreshAllStates(struct ARdrone3Plugin* self) {
    self->deviceController->common->sendCommonAllStates(self->deviceController->common);
}

void ARDrone3_Emergency(struct ARdrone3Plugin *self) {
    self->deviceController->aRDrone3->sendPilotingEmergency(self->deviceController->aRDrone3);
}

void ARDrone3_TakeOff(struct ARdrone3Plugin *self) {
    self->deviceController->aRDrone3->sendPilotingTakeOff(self->deviceController->aRDrone3);
}

void ARDrone3_Land(struct ARdrone3Plugin *self) {
    self->deviceController->aRDrone3->sendPilotingLanding(self->deviceController->aRDrone3);
}

void ARDrone3_SendCameraOrientation(struct ARdrone3Plugin *self, int tilt, int pan) {
    self->deviceController->aRDrone3->sendCameraOrientation(self->deviceController->aRDrone3, tilt, pan);
}

void ARDrone3_SetPCMD(struct ARdrone3Plugin *self, int flag, int roll, int pitch, int yaw, int gaz) {
    self->deviceController->aRDrone3->setPilotingPCMD(self->deviceController->aRDrone3, flag , roll, pitch, yaw, gaz, 0);
}

void PilotingStateFlyingStateChangedCallback (eARCOMMANDS_ARDRONE3_PILOTINGSTATE_FLYINGSTATECHANGED_STATE state, void *custom) {
    char sendingArg[5];
    snprintf(sendingArg, 5, "%d", (int)state);
    UnitySendMessage("DeviceController", "updatePilotingState", sendingArg);
}

void PilotingAlertStateChangedCallback (eARCOMMANDS_ARDRONE3_PILOTINGSTATE_ALERTSTATECHANGED_STATE state, void *customData) {
    char sendingArg[5];
    snprintf(sendingArg, 5, "%d", (int)state);
    UnitySendMessage("DeviceController", "updateAlertState", sendingArg);
}

void PilotingAttitudeChangedCallback (float roll, float pitch, float yaw, void *custom) {
    int size = 50; // to be fitted because value is very often
    char *sendingArg = malloc(size);
    snprintf(sendingArg, size, "%.3f %.3f %.3f", roll, pitch, yaw);
    UnitySendMessage("DeviceController", "updateAttitude", sendingArg);
    free(sendingArg);
}


void CommonBatteryStateChangedCallback (uint8_t val, void *custom) {
    char sendingArg[5];
    snprintf(sendingArg, 5, "%d", (int)val);
    UnitySendMessage("DeviceController", "updateBatteryState", sendingArg);
}

void CameraStateOrientationCallback (int8_t tilt, int8_t pan, void *custom) {
    char sendingArg[11];
    snprintf(sendingArg, 11, "%d %d", tilt, pan);
    UnitySendMessage("DeviceController", "updateCameraOrientation", sendingArg);
}

/*****************************************
 *
 *             private implementation:
 *
 ****************************************/


void registerCallbacks (ARCONTROLLER_Device_t* deviceController) {
    
    // Commands of class : MediaRecordState:
    ARCOMMANDS_Decoder_SetARDrone3MediaRecordStatePictureStateChangedCallback (NULL, NULL);
    ARCOMMANDS_Decoder_SetARDrone3MediaRecordStateVideoStateChangedCallback (NULL, NULL);
    ARCOMMANDS_Decoder_SetARDrone3MediaRecordStatePictureStateChangedV2Callback (NULL, NULL);
    ARCOMMANDS_Decoder_SetARDrone3MediaRecordStateVideoStateChangedV2Callback (NULL, NULL);
    // Commands of class : MediaRecordEvent:
    ARCOMMANDS_Decoder_SetARDrone3MediaRecordEventPictureEventChangedCallback (NULL, NULL);
    ARCOMMANDS_Decoder_SetARDrone3MediaRecordEventVideoEventChangedCallback (NULL, NULL);
    // Commands of class : PilotingState:
    ARCOMMANDS_Decoder_SetARDrone3PilotingStateFlatTrimChangedCallback (NULL, NULL);
    ARCOMMANDS_Decoder_SetARDrone3PilotingStateFlyingStateChangedCallback (&PilotingStateFlyingStateChangedCallback , deviceController->aRDrone3);
    ARCOMMANDS_Decoder_SetARDrone3PilotingStateAlertStateChangedCallback (&PilotingAlertStateChangedCallback, deviceController->aRDrone3);
    ARCOMMANDS_Decoder_SetARDrone3PilotingStateNavigateHomeStateChangedCallback (NULL, NULL);
    ARCOMMANDS_Decoder_SetARDrone3PilotingStatePositionChangedCallback (NULL, NULL);
    ARCOMMANDS_Decoder_SetARDrone3PilotingStateSpeedChangedCallback (NULL, NULL);
    ARCOMMANDS_Decoder_SetARDrone3PilotingStateAttitudeChangedCallback (&PilotingAttitudeChangedCallback, deviceController->aRDrone3);
    ARCOMMANDS_Decoder_SetARDrone3PilotingStateAutoTakeOffModeChangedCallback (NULL, NULL);
    ARCOMMANDS_Decoder_SetARDrone3PilotingStateAltitudeChangedCallback (NULL, NULL);
    // Commands of class : NetworkState:
    ARCOMMANDS_Decoder_SetARDrone3NetworkStateWifiScanListChangedCallback (NULL, NULL);
    ARCOMMANDS_Decoder_SetARDrone3NetworkStateAllWifiScanChangedCallback (NULL, NULL);
    ARCOMMANDS_Decoder_SetARDrone3NetworkStateWifiAuthChannelListChangedCallback (NULL, NULL);
    ARCOMMANDS_Decoder_SetARDrone3NetworkStateAllWifiAuthChannelChangedCallback (NULL, NULL);
    // Commands of class : PilotingSettingsState:
    ARCOMMANDS_Decoder_SetARDrone3PilotingSettingsStateMaxAltitudeChangedCallback (NULL, NULL);
    ARCOMMANDS_Decoder_SetARDrone3PilotingSettingsStateMaxTiltChangedCallback (NULL, NULL);
    ARCOMMANDS_Decoder_SetARDrone3PilotingSettingsStateAbsolutControlChangedCallback (NULL, NULL);
    ARCOMMANDS_Decoder_SetARDrone3PilotingSettingsStateMaxDistanceChangedCallback (NULL, NULL);
    ARCOMMANDS_Decoder_SetARDrone3PilotingSettingsStateNoFlyOverMaxDistanceChangedCallback (NULL, NULL);
    // Commands of class : SpeedSettingsState:
    ARCOMMANDS_Decoder_SetARDrone3SpeedSettingsStateMaxVerticalSpeedChangedCallback (NULL, NULL);
    ARCOMMANDS_Decoder_SetARDrone3SpeedSettingsStateMaxRotationSpeedChangedCallback (NULL, NULL);
    ARCOMMANDS_Decoder_SetARDrone3SpeedSettingsStateHullProtectionChangedCallback (NULL, NULL);
    ARCOMMANDS_Decoder_SetARDrone3SpeedSettingsStateOutdoorChangedCallback (NULL, NULL);
    // Commands of class : NetworkSettingsState:
    ARCOMMANDS_Decoder_SetARDrone3NetworkSettingsStateWifiSelectionChangedCallback (NULL, NULL);
    // Commands of class : SettingsState:
    ARCOMMANDS_Decoder_SetARDrone3SettingsStateProductMotorVersionListChangedCallback (NULL, NULL);
    ARCOMMANDS_Decoder_SetARDrone3SettingsStateProductGPSVersionChangedCallback (NULL, NULL);
    ARCOMMANDS_Decoder_SetARDrone3SettingsStateMotorErrorStateChangedCallback (NULL, NULL);
    ARCOMMANDS_Decoder_SetARDrone3SettingsStateMotorSoftwareVersionChangedCallback (NULL, NULL);
    ARCOMMANDS_Decoder_SetARDrone3SettingsStateMotorFlightsStatusChangedCallback (NULL, NULL);
    ARCOMMANDS_Decoder_SetARDrone3SettingsStateMotorErrorLastErrorChangedCallback (NULL, NULL);
    ARCOMMANDS_Decoder_SetARDrone3SettingsStateP7IDCallback (NULL, NULL);
    // Commands of class : DirectorModeState:
    // Commands of class : PictureSettingsState:
    ARCOMMANDS_Decoder_SetARDrone3PictureSettingsStatePictureFormatChangedCallback (NULL, NULL);
    ARCOMMANDS_Decoder_SetARDrone3PictureSettingsStateAutoWhiteBalanceChangedCallback (NULL, NULL);
    ARCOMMANDS_Decoder_SetARDrone3PictureSettingsStateExpositionChangedCallback (NULL, NULL);
    ARCOMMANDS_Decoder_SetARDrone3PictureSettingsStateSaturationChangedCallback (NULL, NULL);
    ARCOMMANDS_Decoder_SetARDrone3PictureSettingsStateTimelapseChangedCallback (NULL, NULL);
    ARCOMMANDS_Decoder_SetARDrone3PictureSettingsStateVideoAutorecordChangedCallback (NULL, NULL);
    // Commands of class : MediaStreamingState:
    ARCOMMANDS_Decoder_SetARDrone3MediaStreamingStateVideoEnableChangedCallback (NULL, NULL);
    // Commands of class : GPSSettingsState:
    ARCOMMANDS_Decoder_SetARDrone3GPSSettingsStateHomeChangedCallback (NULL, NULL);
    ARCOMMANDS_Decoder_SetARDrone3GPSSettingsStateResetHomeChangedCallback (NULL, NULL);
    ARCOMMANDS_Decoder_SetARDrone3GPSSettingsStateGPSFixStateChangedCallback (NULL, NULL);
    ARCOMMANDS_Decoder_SetARDrone3GPSSettingsStateGPSUpdateStateChangedCallback (NULL, NULL);
    ARCOMMANDS_Decoder_SetARDrone3GPSSettingsStateHomeTypeChangedCallback (NULL, NULL);
    ARCOMMANDS_Decoder_SetARDrone3GPSSettingsStateReturnHomeDelayChangedCallback (NULL, NULL);
    // Commands of class : CameraState:
    ARCOMMANDS_Decoder_SetARDrone3CameraStateOrientationCallback (&CameraStateOrientationCallback, deviceController->aRDrone3);
    // Commands of class : AntiflickeringState:
    ARCOMMANDS_Decoder_SetARDrone3AntiflickeringStateElectricFrequencyChangedCallback (NULL, NULL);
    ARCOMMANDS_Decoder_SetARDrone3AntiflickeringStateModeChangedCallback (NULL, NULL);
    // Commands of class : GPSState:
    ARCOMMANDS_Decoder_SetARDrone3GPSStateNumberOfSatelliteChangedCallback (NULL, NULL);
    ARCOMMANDS_Decoder_SetARDrone3GPSStateHomeTypeAvailabilityChangedCallback (NULL, NULL);
    ARCOMMANDS_Decoder_SetARDrone3GPSStateHomeTypeChosenChangedCallback (NULL, NULL);
    // Commands of class : PROState:
    ARCOMMANDS_Decoder_SetARDrone3PROStateFeaturesCallback (NULL, NULL);
    
    ARCOMMANDS_Decoder_SetCommonCommonStateBatteryStateChangedCallback(&CommonBatteryStateChangedCallback , deviceController->aRDrone3);
    
    
}


// called when the state of the device controller has changed
void stateChanged (eARCONTROLLER_DEVICE_STATE newState, eARCONTROLLER_ERROR error, void *customData)
{
    ARSAL_PRINT(ARSAL_PRINT_INFO, TAG, "    - stateChanged newState: %d .....", newState);
    //struct ARdrone3Plugin* self = (struct ARdrone3Plugin*) customData;
    
    char sendingArg[5];
    snprintf(sendingArg, 5, "%d", (int)newState);
    UnitySendMessage("DeviceController", "updateDeviceControllerState", sendingArg);


    if(newState == ARCONTROLLER_DEVICE_STATE_STOPPED)
    {
        struct ARdrone3Plugin* self = (struct ARdrone3Plugin*) customData;
        //ARCONTROLLER_Device_t *deviceController = NULL;


        ARSAL_PRINT(ARSAL_PRINT_INFO, TAG, "    - delete and free the plugin");
        ////IHM_PrintInfo(ihm, "");
        ARCONTROLLER_Device_Delete (&self->deviceController);

        free(self);
        //ARSAL_Sem_Destroy (&(self->stateSem));
    }

    
}
