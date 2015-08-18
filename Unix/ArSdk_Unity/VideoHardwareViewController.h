//
//  VideoHardwareViewController.h
//  ARUtils
//
//  Created by Djavan Bertrand on 28/04/2015.
//  Copyright (c) 2015 Parrot SA. All rights reserved.
//





#import "libARController/ARController.h"

//#import "DeviceController.h"
//#import "ARVideoViewController.h"


void didReceiveFrameCallback (ARCONTROLLER_Frame_t *frame, void *customData);
void *allocateVideoHarwareController();

@interface TextureDisplayer : NSObject
@property (atomic) CIImage *imageToRender;
@property (atomic) boolean  decodeStarted;

+ (id) gInstance;

-(void) renderEvent:(int) eventId width:(int)width height:(int)height;
-(void) initOpenGL:(void*)texturePtr;

@end

@interface VideoHardwareController : NSObject

+ (id) gInstance;
    
- (void) didReceiveFrame:(ARCONTROLLER_Frame_t *)frame;
- (void) createDecompSession;
//- (void) render:(CMSampleBufferRef);

@end
