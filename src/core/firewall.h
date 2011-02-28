//
//  firewall.h
//  knet-core
//
//  Created by Mathias Hansen on 28/02/11.
//  Copyright 2011 Mathias Hansen. All rights reserved.
//

#define ERROR_INVALID_LOGIN     -1
#define ERROR_ALREADY_LOGGED_IN -2
#define ERROR_BANNED            -3
#define ERROR_CONNECTION        -4

int connectionAlive;

#ifdef __cplusplus
extern "C" {
#endif
    
    int firewall_connect(char *username, char *password);
    int firewall_disconnect();
    int firewall_isconnected();
    
#ifdef __cplusplus
}
#endif