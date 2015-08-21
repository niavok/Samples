//
//  unityOSXSendMessage.h
//  ARsdk3OSXPlugin
//
//  Created by Luc on 25/07/2015.
//  Copyright (c) 2015 Parrot. All rights reserved.
//


#include <stdio.h>

typedef void (* UnityCommandCallback) (const char * objectName, const char* commandName, const char *commandData);

void ARdrone3_connectCallback(UnityCommandCallback callbackMethod);

void UnitySendMessage (const char * objectName, const char* commandName, const char *commandData);
