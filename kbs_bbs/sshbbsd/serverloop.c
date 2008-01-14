/*

serverloop.c

Author: Tatu Ylonen <ylo@cs.hut.fi>

Copyright (c) 1995 Tatu Ylonen <ylo@cs.hut.fi>, Espoo, Finland
                   All rights reserved

Created: Sun Sep 10 00:30:37 1995 ylo

Server main loop for handling the interactive session.

*/

#include "includes.h"
#include "bbs.h"
#include "ssh_funcs.h"
#include "xmalloc.h"
#include "ssh.h"
#include "packet.h"
#include "buffer.h"

int ssh_write(int fd, const void *buf, size_t count)
{
    int len;
    const char *data = buf;
    int result = count;

    while (count > 0) {
        len = count > 512 ? 512 : count;
        packet_start(SSH_SMSG_STDOUT_DATA);
        packet_put_string(data, len);
        packet_send();
        packet_write_wait();
        count -= len;
        data += len;
    }
    return result;
}
static Buffer NetworkBuf;
void ProcessOnePacket(int wait);
ssize_t ssh_read(int fd, void *buf, size_t count)
{
    int retlen = 0;

    if (count < 0)
        return count;
    ProcessOnePacket(0);
    while (buffer_len(&NetworkBuf) <= 0) {
        ProcessOnePacket(1);
        ProcessOnePacket(0);
    }
    retlen = buffer_len(&NetworkBuf);
    retlen = retlen > count ? count : retlen;
    buffer_get(&NetworkBuf, buf, retlen);
    return retlen;
}
int ssh_select(int n, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, struct timeval *timeout)
{
	if (buffer_len(&NetworkBuf) > 0)
		return 1;
	return select(n, readfds, writefds, exceptfds, timeout);
}
int ssh_init(void)
{
    buffer_init(&NetworkBuf);
    return 0;
}
void ProcessOnePacket(int wait)
{
    int type;
    char *data;
    unsigned int data_len;
    int row, col, xpixel, ypixel;

    while (1) {
        if (wait)
            type = packet_read();
        else
            type = packet_read_poll();
        if (type == SSH_MSG_NONE)
            goto read_done;
        switch (type) {
        case SSH_CMSG_STDIN_DATA:
            /* Stdin data from the client.  Append it to the buffer. */
            data = packet_get_string(&data_len);
            buffer_append(&NetworkBuf, data, data_len);
            memset(data, 0, data_len);
            xfree(data);
            if (wait)
                goto read_done;
            break;

        case SSH_CMSG_EOF:
            /* Eof from the client.  The stdin descriptor to the program
               will be closed when all buffered data has drained. */
            debug("EOF received for stdin.");
            goto read_done;
            break;

        case SSH_CMSG_WINDOW_SIZE:
            debug("Window change received.");
            row = packet_get_int();
            col = packet_get_int();
            xpixel = packet_get_int();
            ypixel = packet_get_int();
            do_naws(row, col);
//            pty_change_window_size(fdin, row, col, xpixel, ypixel);
            break;

        case SSH_MSG_PORT_OPEN:
            break;

        case SSH_MSG_CHANNEL_OPEN_CONFIRMATION:
            debug("Received channel open confirmation.");
            break;

        case SSH_MSG_CHANNEL_OPEN_FAILURE:
            debug("Received channel open failure.");
            break;

        case SSH_MSG_CHANNEL_DATA:
            break;

#ifdef SUPPORT_OLD_CHANNELS
        case SSH_MSG_CHANNEL_CLOSE:
            debug("Received channel close.");
            break;

        case SSH_MSG_CHANNEL_CLOSE_CONFIRMATION:
            debug("Received channel close confirmation.");
            break;
#else
        case SSH_MSG_CHANNEL_INPUT_EOF:
            debug("Received channel input eof.");
            break;

        case SSH_MSG_CHANNEL_OUTPUT_CLOSED:
            debug("Received channel output closed.");
            break;

#endif

        default:
            /* In this phase, any unexpected messages cause a protocol
               error.  This is to ease debugging; also, since no 
               confirmations are sent messages, unprocessed unknown 
               messages could cause strange problems.  Any compatible 
               protocol extensions must be negotiated before entering the 
               interactive session. */
            packet_disconnect("Protocol error during session: type %d", type);
        }
    }
read_done:
	return;

}
