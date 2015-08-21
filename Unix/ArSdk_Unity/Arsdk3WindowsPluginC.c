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

};

void SetTextureFromUnity (void* texturePtr)
{
	
}

void UnityRenderEvent (int eventId, int width, int height)
{
	
}


void didReceiveFrameCallback (ARCONTROLLER_Frame_t *frame, void *customData)
{
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
    error = ARCONTROLLER_Device_SetVideoReceiveCallback (self->deviceController, didReceiveFrameCallback, NULL , NULL);
    
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
    
    
 
    
    return self;
}

void ARdrone3_Disconnect(struct ARdrone3Plugin *self) {
    
    ARCONTROLLER_Device_t *deviceController = NULL;
    eARCONTROLLER_ERROR error = ARCONTROLLER_OK;
    eARCONTROLLER_DEVICE_STATE deviceState = ARCONTROLLER_DEVICE_STATE_MAX;
    //ARSAL_Sem_Init (&(self->stateSem), 0, 0);

    //deviceController->aRDrone3->sendCameraOrientation();
    deviceState = ARCONTROLLER_Device_GetState (deviceController, &error);
    if ((error == ARCONTROLLER_OK) && (deviceState != ARCONTROLLER_DEVICE_STATE_STOPPED))
    {
        //////IHM_PrintInfo(ihm, "Disconnecting ...");
        
        error = ARCONTROLLER_Device_Stop (deviceController);
        
        if (error == ARCONTROLLER_OK)
        {
            // wait state update update 
            //ARSAL_Sem_Wait (&(self->stateSem));
        }
    }
    
    ////IHM_PrintInfo(ihm, "");
    ARCONTROLLER_Device_Delete (&deviceController);
                
    //ARSAL_Sem_Destroy (&(self->stateSem));
    
    ARSAL_PRINT(ARSAL_PRINT_INFO, TAG, "-- END --");
    
    free(self);
}

void ARDrone3_StartVideo(struct ARdrone3Plugin* self) {
    
    eARCONTROLLER_ERROR error = ARCONTROLLER_OK;
    boolean failed = 0;

    
    
    if (!failed)
    {
        ARSAL_PRINT(ARSAL_PRINT_INFO, TAG, "- send StreamingVideoEnable ... ");
        error = self->deviceController->aRDrone3->sendMediaStreamingVideoEnable (self->deviceController->aRDrone3, 1);
        if (error != ARCONTROLLER_OK)
        {
            ARSAL_PRINT(ARSAL_PRINT_ERROR, TAG, "- error :%s", ARCONTROLLER_Error_ToString(error));
            failed = 1;
        }
    }
}


void ARDrone3_refreshAllStates(struct ARdrone3Plugin* self) {
    self->deviceController->common->sendCommonAllStates(self->deviceController->common);
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

void ARDrone3_SendPCMD(struct ARdrone3Plugin *self, int flag, int roll, int pitch, int yaw, int gaz) {
    self->deviceController->aRDrone3->sendPilotingPCMD (self->deviceController->aRDrone3, flag , roll, pitch, yaw, gaz, 0);
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

    
}
