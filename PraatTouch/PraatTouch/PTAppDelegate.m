//
//  PTAppDelegate.m
//  PraatTouch
//
//  Created by Tom Naughton on 1/14/14.
//  Copyright (c) 2014 Tom Naughton. All rights reserved.
//

#import "PTAppDelegate.h"

#include "praat.h"
#include "praat_version.h"

static void logo (Graphics g) {
	Graphics_setWindow (g, 0, 1, 0.00, 0.80);
	Graphics_setTextAlignment (g, Graphics_CENTRE, Graphics_HALF);
	Graphics_setFont (g, kGraphics_font_TIMES);
	Graphics_setFontSize (g, 45);
	Graphics_setColour (g, Graphics_MAROON);
	Graphics_text (g, 0.385, 0.66, L"P");
	Graphics_text (g, 0.448, 0.66, L"\\s{R}");
	Graphics_text (g, 0.510, 0.66, L"\\s{A}");
	Graphics_text (g, 0.575, 0.66, L"\\s{A}");
	Graphics_text (g, 0.628, 0.66, L"\\s{T}");
	Graphics_setFontSize (g, 15);
	Graphics_text (g, 0.5, 0.55, L"%%doing phonetics by computer");
#define xstr(s) str(s)
#define str(s) #s
	Graphics_text (g, 0.5, 0.45, L"version " xstr(PRAAT_VERSION_STR));
	Graphics_setColour (g, Graphics_BLACK);
	Graphics_setFontSize (g, 14);
	Graphics_text (g, 0.5, 0.33, L"www.praat.org");
	Graphics_setFont (g, kGraphics_font_HELVETICA);
	Graphics_setFontSize (g, 10);
	Graphics_text (g, 0.5, 0.16, L"Copyright \\co 1992-" xstr(PRAAT_YEAR) " by Paul Boersma and David Weenink");
}


#pragma mark - #defines

#define FOREGROUND_FREQUENCY    880.0
#define BACKGROUND_FREQUENCY    523.25
#define BUFFER_COUNT            3
#define BUFFER_DURATION         0.5


@implementation PTAppDelegate

#pragma mark - @synthesizes

@synthesize window=_window;
@synthesize streamFormat=_streamFormat;
@synthesize bufferSize;
@synthesize currentFrequency;
@synthesize startingFrameCount;
@synthesize audioQueue;

#pragma mark helpers

// generic error handler - if err is nonzero, prints error message and exits program.
static void CheckError(OSStatus error, const char *operation)
{
	if (error == noErr) return;
	
	char str[20];
	// see if it appears to be a 4-char-code
	*(UInt32 *)(str + 1) = CFSwapInt32HostToBig(error);
	if (isprint(str[1]) && isprint(str[2]) && isprint(str[3]) && isprint(str[4])) {
		str[0] = str[5] = '\'';
		str[6] = '\0';
	} else
		// no, format it as an integer
		sprintf(str, "%d", (int)error);
    
	fprintf(stderr, "Error: %s (%s)\n", operation, str);
    
	exit(1);
}

static void CheckNSError(NSError *error, const char *operation)
{
}

#pragma mark callbacks
-(OSStatus) fillBuffer: (AudioQueueBufferRef) buffer {
	
   	double j = self.startingFrameCount;
	double cycleLength = 44100. / self.currentFrequency;
	int frame = 0;
    double frameCount = bufferSize / self.streamFormat.mBytesPerFrame;
	for (frame = 0; frame < frameCount; ++frame)
	{
		SInt16 *data = (SInt16*)buffer->mAudioData;
		(data)[frame] = (SInt16) (sin (2 * M_PI * (j / cycleLength)) * SHRT_MAX);
        
		j += 1.0;
		if (j > cycleLength)
			j -= cycleLength;
	}
	
	self.startingFrameCount = j;
    
    buffer->mAudioDataByteSize = bufferSize;
	
    return noErr;
}

static void MyAQOutputCallback(void *inUserData, AudioQueueRef inAQ, AudioQueueBufferRef inCompleteAQBuffer)
{
	PTAppDelegate *appDelegate = (__bridge PTAppDelegate*)inUserData;
    CheckError([appDelegate fillBuffer: inCompleteAQBuffer],
               "can't refill buffer");
    CheckError(AudioQueueEnqueueBuffer(inAQ,
                                       inCompleteAQBuffer,
                                       0,
                                       NULL),
               "Couldn't enqueue buffer (refill)");
	
}

- (void)audioSessionDidChangeInterruptionType:(NSNotification *)notification
{
    NSNumber *type = [notification userInfo][AVAudioSessionInterruptionTypeKey];
    AVAudioSessionInterruptionType interruptionType = (AVAudioSessionInterruptionType)[type intValue];

    if (AVAudioSessionInterruptionTypeBegan == interruptionType)
    {
    }
    else if (AVAudioSessionInterruptionTypeEnded == interruptionType)
    {
        CheckError(AudioQueueStart(self.audioQueue, 0),
                   "Couldn't restart audio queue");

    }
}



- (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions
{
    // http://stackoverflow.com/questions/19710046/a-fix-for-audiosessioninitialize-deprecated
    // set up audio session
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(audioSessionDidChangeInterruptionType:)
                                                 name:AVAudioSessionInterruptionNotification object:[AVAudioSession sharedInstance]];

    NSError *error = nil;
    [[AVAudioSession sharedInstance]
     setCategory:AVAudioSessionCategoryPlayback error:&error];
    CheckNSError(error, "Couldn't set category on audio session");
    
    
    // set stream format
    self.currentFrequency = FOREGROUND_FREQUENCY;
    _streamFormat.mSampleRate = 44100.0;
	_streamFormat.mFormatID = kAudioFormatLinearPCM;
	_streamFormat.mFormatFlags = kAudioFormatFlagsCanonical;
	_streamFormat.mChannelsPerFrame = 1;
	_streamFormat.mFramesPerPacket = 1;
	_streamFormat.mBitsPerChannel = 16;
	_streamFormat.mBytesPerFrame = 2;
	_streamFormat.mBytesPerPacket = 2;
    
    // create the audio queue
    CheckError( AudioQueueNewOutput(&_streamFormat,
                                    MyAQOutputCallback,
                                    (__bridge void *)(self),
                                    NULL,
                                    kCFRunLoopCommonModes,
                                    0,
                                    &audioQueue),
               "Couldn't create output AudioQueue");
    
    
    // create and enqueue buffers
    AudioQueueBufferRef buffers [BUFFER_COUNT];
    bufferSize = BUFFER_DURATION * self.streamFormat.mSampleRate * self.streamFormat.mBytesPerFrame;
    NSLog (@"bufferSize is %ld", bufferSize);
    for (int i=0; i<BUFFER_COUNT; i++) {
        CheckError (AudioQueueAllocateBuffer(audioQueue,
                                             bufferSize,
                                             &buffers[i]),
                    "Couldn't allocate Audio Queue buffer");
        CheckError([self fillBuffer:buffers[i]],
                   "Couldn't fill buffer (priming)");
        CheckError(AudioQueueEnqueueBuffer(audioQueue,
                                           buffers[i],
                                           0,
                                           NULL),
                   "Couldn't enqueue buffer (priming)");
    }
    
    CheckError(AudioQueueStart(audioQueue, NULL),
               "Couldn't start AudioQueue");

    UINavigationController *navigationController = (UINavigationController*) [self.window rootViewController];
    self.tableViewController = navigationController.viewControllers[0];
    navigationController.navigationBarHidden = YES;
    praat_setLogo (130, 80, logo);
    praat_init ("Praat", 0, nil);
    INCLUDE_LIBRARY (praat_uvafon_init)
    INCLUDE_LIBRARY (praat_contrib_Ola_KNN_init)
    praat_run ();

    return YES;
}

- (void)applicationWillResignActive:(UIApplication *)application
{
    // Sent when the application is about to move from active to inactive state. This can occur for certain types of temporary interruptions (such as an incoming phone call or SMS message) or when the user quits the application and it begins the transition to the background state.
    // Use this method to pause ongoing tasks, disable timers, and throttle down OpenGL ES frame rates. Games should use this method to pause the game.
}

- (void)applicationDidEnterBackground:(UIApplication *)application
{
    /*
     Use this method to release shared resources, save user data, invalidate timers, and store enough application state information to restore your application to its current state in case it is terminated later.
     If your application supports background execution, this method is called instead of applicationWillTerminate: when the user quits.
     */
    self.currentFrequency = BACKGROUND_FREQUENCY;
}

- (void)applicationWillEnterForeground:(UIApplication *)application
{
    
    // http://stackoverflow.com/questions/18807157/how-do-i-route-audio-to-speaker-without-using-audiosessionsetproperty
    
    /*
     Called as part of the transition from the background to the inactive state; here you can undo many of the changes made on entering the background.
     */
	CheckError(AudioSessionSetActive(true),
			   "Couldn't re-set audio session active");
	CheckError(AudioQueueStart(self.audioQueue, 0),
			   "Couldn't restart audio queue");
    self.currentFrequency = FOREGROUND_FREQUENCY;
}

- (void)applicationDidBecomeActive:(UIApplication *)application
{
    // Restart any tasks that were paused (or not yet started) while the application was inactive. If the application was previously in the background, optionally refresh the user interface.
}

- (void)applicationWillTerminate:(UIApplication *)application
{
    // Called when the application is about to terminate. Save data if appropriate. See also applicationDidEnterBackground:.
}


@end
