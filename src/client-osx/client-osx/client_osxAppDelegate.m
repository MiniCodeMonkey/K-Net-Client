//
//  client_osxAppDelegate.m
//  client-osx
//
//  Created by Mathias Hansen on 28/02/11.
//  Copyright 2011 Mathias Hansen. All rights reserved.
//

#import "client_osxAppDelegate.h"

@implementation client_osxAppDelegate

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification
{
    [[NSApplication sharedApplication] activateIgnoringOtherApps:YES];
    
    statusItem = [[[NSStatusBar systemStatusBar] statusItemWithLength:NSSquareStatusItemLength] retain];
    
    [statusItem setImage:[NSImage imageNamed:@"IconInactive.png"]];
    [statusItem setHighlightMode:YES];
    
    [statusItem setMenu:statusMenu];
}

- (IBAction)connectClicked:(id)sender
{
    if ([menuStatusItem.title isEqualToString:@"Connect"])
    {
        [statusItem setLength:110.0f];
        [statusItem setTitle:@"Connecting.."];
        [menuStatusItem setTitle:@"Connecting..."];
        
        [self performSelectorInBackground:@selector(waitForConnection) withObject:nil];
        [self performSelectorInBackground:@selector(connectionThread) withObject:nil];
    }
    else if ([menuStatusItem.title isEqualToString:@"Disconnect"])
    {
        [statusItem setImage:[NSImage imageNamed:@"IconInactive.png"]];
        [menuStatusItem setTitle:@"Connect"];
        firewall_disconnect();
    }
}

- (void)connectionThread
{
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
    
    int result = firewall_connect("*", "*");
    NSNumber *resultObj = [NSNumber numberWithInt:result];
    
    [self performSelectorOnMainThread:@selector(setStatus:) withObject:resultObj waitUntilDone:NO];
    
    [pool release];
}

- (void)waitForConnection
{
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
    
    // Wait for firewall connection
    errorReceived = NO;
    while (!firewall_isconnected() && !errorReceived)
    {
        [NSThread sleepForTimeInterval:0.5f];
    }
    
    if (!errorReceived)
    {
        // Hurray, we are connected
        [statusItem setImage:[NSImage imageNamed:@"IconConnected.png"]];
        [menuStatusItem setTitle:@"Disconnect"];
    }
    
    [statusItem setTitle:nil];
    [statusItem setLength:NSSquareStatusItemLength];
    
    [pool release];
}

- (void)setStatus:(id)status
{
    int statusCode = [(NSNumber*)status intValue];
    bool errorIcon = YES;
    
    switch (statusCode)
    {
        case ERROR_CONNECTION:
            NSRunAlertPanel(@"Connection error", @"Could not connect to K-Net firewall", @"OK", nil, nil);
            break;
            
        case ERROR_ALREADY_LOGGED_IN:
            NSRunAlertPanel(@"Already logged in", @"You are already logged in, please try again in a few minutes", @"OK", nil, nil);
            break;
            
        case ERROR_BANNED:
            NSRunAlertPanel(@"Traffic limit reached", @"Oooh snap! You have reached your traffic limit.", @"OK", nil, nil);
            break;
            
        case ERROR_INVALID_LOGIN:
            NSRunAlertPanel(@"Invalid login", @"The supplied username/password password is not valid credentials for the K-Net firewall.", @"OK", nil, nil);
            break;
            
        default:
            errorIcon = NO;
            break;
    }
    
    if (errorIcon)
    {
        errorReceived = YES;
        [statusItem setImage:[NSImage imageNamed:@"IconError.png"]];
    }
    else
    {
        [statusItem setImage:[NSImage imageNamed:@"IconInactive.png"]];
    }
    
    [menuStatusItem setTitle:@"Connect"];
}

@end
