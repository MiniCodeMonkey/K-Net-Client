//
//  firewall.c
//  knet-core
//
//  Created by Mathias Hansen on 28/02/11.
//  Copyright 2011 Mathias Hansen. All rights reserved.
//

#include "libssh2_config.h"

#include <libssh2.h>

#ifdef HAVE_WINSOCK2_H
#include <winsock2.h>
#endif
#ifdef HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif
#ifdef HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif
#ifdef HAVE_SYS_SELECT_H
#include <sys/select.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_ARPA_INET_H
#include <arpa/inet.h>
#endif

#include <sys/time.h>
#include <sys/types.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <ctype.h>

#include "firewall.h"

static int waitsocket(int socket_fd, LIBSSH2_SESSION *session)
{
    struct timeval timeout;
    int rc;
    fd_set fd;
    fd_set *writefd = NULL;
    fd_set *readfd = NULL;
    int dir;
    
    timeout.tv_sec = 10;
    timeout.tv_usec = 0;
    
    FD_ZERO(&fd);
    
    FD_SET(socket_fd, &fd);
    
    /* now make sure we wait in the correct direction */ 
    dir = libssh2_session_block_directions(session);
    
    
    if(dir & LIBSSH2_SESSION_BLOCK_INBOUND)
        readfd = &fd;
    
    if(dir & LIBSSH2_SESSION_BLOCK_OUTBOUND)
        writefd = &fd;
    
    rc = select(socket_fd + 1, readfd, writefd, NULL, &timeout);
    
    return rc;
}

int firewall_connect(char *username, char *password)
{
    int return_value = 0;
    
    connectionAlive = 0;
    const char *hostname = "82.211.192.131";
    unsigned long hostaddr;
    int sock;
    struct sockaddr_in sin;
    const char *fingerprint;
    LIBSSH2_SESSION *session;
    LIBSSH2_CHANNEL *channel;
    int rc;
    int exitcode;
    int bytecount = 0;
    size_t len;
    LIBSSH2_KNOWNHOSTS *nh;
    int type;
    
#ifdef WIN32
    WSADATA wsadata;
    WSAStartup(MAKEWORD(2,0), &wsadata);
#endif
    
    rc = libssh2_init(0);
    
    if (rc != 0)
    {
        fprintf (stderr, "libssh2 initialization failed (%d)\n", rc);
        return ERROR_CONNECTION;
    }
    
    hostaddr = inet_addr(hostname);
    
    sock = socket(AF_INET, SOCK_STREAM, (in_addr_t)0);
    
    sin.sin_family = AF_INET;
    sin.sin_port = htons(22);
    sin.sin_addr.s_addr = (in_addr_t)hostaddr;
    if (connect(sock, (struct sockaddr*)(&sin),
                sizeof(struct sockaddr_in)) != 0)
    {
        fprintf(stderr, "failed to connect!\n");
        return ERROR_CONNECTION;
    }
    
    /* Create a session instance */ 
    session = libssh2_session_init();
    
    if (!session)
        return ERROR_CONNECTION;
    
    /* tell libssh2 we want it all done non-blocking */ 
    libssh2_session_set_blocking(session, 0);
    
    /* ... start it up. This will trade welcome banners, exchange keys,
     * and setup crypto, compression, and MAC layers
     */ 
    while ((rc = libssh2_session_startup(session, sock)) == LIBSSH2_ERROR_EAGAIN);
    
    if (rc)
    {
        fprintf(stderr, "Failure establishing SSH session: %d\n", rc);
        return ERROR_CONNECTION;
    }
    
    nh = libssh2_knownhost_init(session);
    
    if (!nh)
    {
        /* eeek, do cleanup here */ 
        return ERROR_CONNECTION;
    }
    
    fingerprint = libssh2_session_hostkey(session, &len, &type);
    
    if (fingerprint)
    {
        /*****
         * At this point, we could verify that 'check' tells us the key is
         * fine or bail out.
         *****/ 
    }
    else {
        /* eeek, do cleanup here */ 
        return ERROR_CONNECTION;
    }
    libssh2_knownhost_free(nh);
    
    
    /* Authenticate via password */ 
    while ((rc = libssh2_userauth_password(session, username, password)) == LIBSSH2_ERROR_EAGAIN);
    if (rc)
    {
        fprintf(stderr, "Authentication by password failed.\n");
        return_value = ERROR_INVALID_LOGIN;
    }
    else
    {
        /* Exec non-blocking on the remote host */ 
        while ((channel = libssh2_channel_open_session(session)) == NULL && libssh2_session_last_error(session,NULL,NULL,0) == LIBSSH2_ERROR_EAGAIN )
        {
            waitsocket(sock, session);
        }
        
        if (channel == NULL)
        {
            fprintf(stderr,"Error\n");
            return ERROR_CONNECTION;
        }
        
        while ((rc = libssh2_channel_exec(channel, "\n")) == LIBSSH2_ERROR_EAGAIN)
        {
            waitsocket(sock, session);
        }
        
        connectionAlive = 2;
        int emptyPacketsReceived = 0;
        while (connectionAlive > 0)
        {
            /* loop until we block */ 
            int rc;
            do
            {
                char buffer[0x4000];
                rc = libssh2_channel_read(channel, buffer, sizeof(buffer));
                
                if (rc > 0)
                {
                    int i;
                    bytecount += rc;
                    fprintf(stdout, "We read:\n");
                    
                    if (strtok(buffer, "You are authenticated") == NULL)
                    {
                        connectionAlive = 1;
                    }
                    else if (strtok(buffer, "You are already logged on") == NULL)
                    {
                        return_value = ERROR_ALREADY_LOGGED_IN;
                        connectionAlive = 0;
                        break;
                    }
                    else if (strtok(buffer, "you have been banned") == NULL)
                    {
                        return_value = ERROR_BANNED;
                        connectionAlive = 0;
                        break;
                    }
                    
                    for (i = 0; i < rc; ++i)
                        fputc( buffer[i], stdout);
                    
                    fprintf(stdout, "\n");
                }
                else
                {
                    fprintf(stdout, "libssh2_channel_read returned %d\n", rc);
                    if (connectionAlive == 2 && rc == LIBSSH2_ERROR_EAGAIN)
                        emptyPacketsReceived++;
                    
                    if (connectionAlive == 2 && emptyPacketsReceived > 5)
                    {
                        // Disconnect with error
                        return_value = ERROR_CONNECTION;
                        connectionAlive = 0;
                        break;
                    }
                }
            }
            while (rc > 0);
            
            /* this is due to blocking that would occur otherwise so we loop on
             this condition */ 
            if (rc == LIBSSH2_ERROR_EAGAIN)
            {
                waitsocket(sock, session);
            }
            else
                break;
        }
        exitcode = 127;
        while ((rc = libssh2_channel_close(channel)) == LIBSSH2_ERROR_EAGAIN)
        {
            waitsocket(sock, session);
        }
        
        libssh2_channel_free(channel);
        
        channel = NULL;
    }
    
    libssh2_session_disconnect(session, "Normal Shutdown, Thank you for playing");
    libssh2_session_free(session);
    
    
#ifdef WIN32
    closesocket(sock);
#else
    close(sock);
#endif
    fprintf(stderr, "all done\n");
    printf("Return value: %d\n", return_value);
    
    libssh2_exit();
    
    connectionAlive = 0;
    
    return return_value;
}

int firewall_disconnect()
{
    connectionAlive = 0;
    
    return 0;
}

int firewall_isconnected()
{
    return connectionAlive;
}