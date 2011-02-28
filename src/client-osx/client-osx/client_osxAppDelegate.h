//
//  client_osxAppDelegate.h
//  client-osx
//
//  Created by Mathias Hansen on 28/02/11.
//  Copyright 2011 Mathias Hansen. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#include "firewall.h"

@interface client_osxAppDelegate : NSObject <NSApplicationDelegate> {
@private
    NSStatusItem *statusItem;
    IBOutlet NSMenu *statusMenu;
    IBOutlet NSMenuItem *menuStatusItem;
    bool errorReceived;
}

- (IBAction)connectClicked:(id)sender;
- (void)connectionThread;
- (void)setStatus:(id)status;
- (void)waitForConnection;

@end
