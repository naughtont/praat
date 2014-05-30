//
//  PTAppDelegate.h
//  PraatTouch
//
//  Created by Tom Naughton on 1/14/14.
//  Copyright (c) 2014 Tom Naughton. All rights reserved.
//
// Audio portions based on
//  CH10_iOSBackgroundingTone
//
//  Created by Chris Adamson on 4/22/11.
//  Copyright 2011 Subsequently and Furthermore, Inc. All rights reserved.
//
//

#import <UIKit/UIKit.h>
#import <AudioToolbox/AudioToolbox.h>
#import <AVFoundation/AVFoundation.h>

#import "PTViewController.h"
#import "PTTableViewController.h"


@interface PTAppDelegate : UIResponder <UIApplicationDelegate>

@property (strong, nonatomic) IBOutlet UIWindow *window;

@property (nonatomic, assign) AudioStreamBasicDescription streamFormat;
@property (nonatomic, assign) UInt32 bufferSize;
@property (nonatomic, assign) double currentFrequency;
@property (nonatomic, assign) double startingFrameCount;
@property (nonatomic, assign) AudioQueueRef	audioQueue;

-(OSStatus) fillBuffer: (AudioQueueBufferRef) buffer;

@property (strong, nonatomic) PTTableViewController *tableViewController;


@end
