//
//  client_osxAppDelegate.h
//  client-osx
//
//  Created by Mathias Hansen on 28/02/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@interface client_osxAppDelegate : NSObject <NSApplicationDelegate> {
@private
    NSStatusItem *statusItem;
    IBOutlet NSMenu *statusMenu;
    IBOutlet NSMenuItem *menuStatusItem;
}

- (IBAction)connectClicked:(id)sender;

@end
