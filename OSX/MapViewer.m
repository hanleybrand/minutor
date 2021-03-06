/*
 Copyright (c) 2010, Sean Kasun
 All rights reserved.
 
 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:
 
 * Redistributions of source code must retain the above copyright notice, this
 list of conditions and the following disclaimer.
 
 * Redistributions in binary form must reproduce the above copyright notice,
 this list of conditions and the following disclaimer in the documentation
 and/or other materials provided with the distribution.
 
 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 THE POSSIBILITY OF SUCH DAMAGE.
 */

#import "MapViewer.h"
#include "MinutorMap.h"


@implementation MapViewer
-(NSObject *)init
{
	self = [super init];
	curDepth=0;
	return self;
}
-(BOOL)isVisible
{
	return [window isVisible];
}

-(void)openWorld:(NSString *)world
{
	CloseAll();
	[map setProgress:progressBar];
	[map setStatus:status];
	[map setWorld:world];
	GetSpawn([world UTF8String], &spawnX, &spawnY, &spawnZ);
	GetPlayer([world UTF8String], &playerX, &playerY, &playerZ);
	[map setX:spawnX andZ:spawnZ];
	[map setDepth:255-curDepth];
	
	[window makeKeyAndOrderFront:self];
	[window setTitle:[world lastPathComponent]];
	[status setStringValue:@""];
	[window setAcceptsMouseMovedEvents: YES];
}
-(void)jumpToSpawn
{
	[map setX:spawnX andZ:spawnZ];
}
-(void)jumpToPlayer
{
	[map setX:playerX andZ:playerZ];
}
-(IBAction)sliderChanged:(NSSlider *)sender
{
	int newDepth=[slider intValue];
	if (newDepth!=curDepth)
	{
		curDepth=newDepth;
		[depthText setStringValue:[NSString stringWithFormat:@"%d",255-curDepth]];
		[map setDepth:255-curDepth];
	}
}
-(void)setOpts:(int)options
{
	[map setOptions:options];
}
-(void)setColorScheme:(unsigned int *)colors
{
	[map setColorScheme:colors];
}

@end
