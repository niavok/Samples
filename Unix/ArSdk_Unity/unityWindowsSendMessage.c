//
//  unityWindowsSendMessage.c
//  ARsdk3Plugin
//
//  Created by Luc on 25/07/2015.
//  Copyright (c) 2015 Parrot. All rights reserved.
//

#include <libARSAL/ARSAL.h>

#define TAG "SDK3UnityPlugin"
#include "UnityMultiplatformSendMessage.h"

static UnityCommandCallback lastCallback = NULL;

void ARdrone3_connectCallback(UnityCommandCallback callbackMethod) {
    lastCallback = callbackMethod;
}

void UnitySendMessage (const char * objectName, const char* commandName, const char *commandData) {
    if (lastCallback != NULL) {
        lastCallback( objectName, commandName, commandData);
    }
}
