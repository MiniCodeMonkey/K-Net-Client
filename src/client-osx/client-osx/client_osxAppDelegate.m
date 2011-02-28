//
//  client_osxAppDelegate.m
//  client-osx
//
//  Created by Mathias Hansen on 28/02/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "client_osxAppDelegate.h"

@implementation client_osxAppDelegate

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification
{
    statusItem = [[[NSStatusBar systemStatusBar] statusItemWithLength:NSSquareStatusItemLength] retain];
    
    NSImage *statusImage = [NSImage imageNamed:@"StatusIcon.png"];
    [statusItem setImage:statusImage];
    [statusItem setHighlightMode:YES];
    
    [statusItem setMenu:statusMenu];
}

@end
