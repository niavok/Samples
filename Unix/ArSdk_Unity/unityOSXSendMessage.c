//
//  unityOSXSendMessage.c
//  ARsdk3OSXPlugin
//
//  Created by Luc on 25/07/2015.
//  Copyright (c) 2015 Parrot. All rights reserved.
//

#include "unityMultiplatformSendMessage.h"

static UnityCommandCallback lastCallback = NULL;

void connectCallback(UnityCommandCallback callbackMethod) {
    lastCallback = callbackMethod;
}

void UnitySendMessage (const char * objectName, const char* commandName, const char *commandData) {
    if (lastCallback != NULL) {
        lastCallback( objectName, commandName, commandData);
    }
}
