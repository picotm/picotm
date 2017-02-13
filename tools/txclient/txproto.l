
%option reentrant
%option debug
%option noyywrap

%{
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include "txproto.h"
#include "txproto.tab.h"

%}

b64char     [A-Za-z0-9+/]

%%

[[:space:]]

TXCHAL      {return TOK_TXCHAL;}
TXRESP      {return TOK_TXRESP;}
TXCMMT      {return TOK_TXCMMT;}
TXABRT      {return TOK_TXABRT;}
TXDATA      {return TOK_TXDATA;}
TXEOTX      {return TOK_TXEOTX;}
TXNACK      {return TOK_TXNACK;}
TXNNDO      {return TOK_TXNNDO;}

{b64char}   {yylval->uc = *yytext; return TOK_B64CHR;}

=           {return '=';}

%%

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "base64.h"

int
txproto_wait_data(int fd, int s)
{
    fd_set rdset;

    FD_ZERO(&rdset);
    FD_SET(fd, &rdset);

    int res;

    if (s >= 0) {

        struct timeval timeout;
        timeout.tv_sec = s;
        timeout.tv_usec = 0;

        if (select(FD_SETSIZE, &rdset, NULL, NULL, &timeout) < 0) {
            perror("select");
        }
    } else {
        if (select(FD_SETSIZE, &rdset, NULL, NULL, NULL) < 0) {
            perror("select");
        }
    }

    int isset = FD_ISSET(fd, &rdset);

    FD_ZERO(&rdset);

    return isset;
}

int
txproto_recv_packet(int fd, struct packet *packet, int flags)
{
    assert(packet);

    /* Read line from socket
     */

    char *buf = NULL;
    size_t buflen = 0;

    char c;
    ssize_t len;

    do {
        len = recv(fd, &c, sizeof(c), flags);

        if (len < 0) {
            if (errno == EAGAIN) {
                goto process;
            }
            perror("recv");
            return -1;
        }

        if ((len > 0) && (c != '\n')) {

            void *tmp = realloc(buf, buflen+1);

            if (!tmp) {
                perror("realloc");
                abort();
            }
            buf = tmp;

            buf[buflen] = c;
            ++buflen;
        }
    } while (buflen && (len > 0) && (c != '\n'));

    process:

    if (!buflen) {
        abort();
        packet->cmd = CMD_EMPTY;
    } else {

        void *tmp = realloc(buf, buflen+2);

        if (!tmp) {
            perror("realloc");
            abort();
        }
        buf = tmp;

        buf[buflen+0] = YY_END_OF_BUFFER_CHAR;
        buf[buflen+1] = YY_END_OF_BUFFER_CHAR;

        /* Parse line to packet
         */

        /*extern int txprotodebug;
        txprotodebug = 1;*/
        yyscan_t scanner;
        yylex_init(&scanner);

        YY_BUFFER_STATE buffer = yy_scan_buffer(buf, buflen+2, scanner);

        if (!buffer) {
            abort();
            return -1;
        }

        yy_switch_to_buffer(buffer, scanner);

        int res = txprotoparse(packet);

        switch (res) {
            case 1:
                /* Invalid input */
                packet->cmd = CMD_PLAIN;
                packet->data.msg = malloc(buflen);
                memcpy(packet->data.msg, buf, buflen);
                packet->data.len = buflen;
                break;
            case 2:
                /* Memory exhaused */
                break;
            default:
                /* Packet received */
                break;
        }

        yy_delete_buffer(buffer, scanner);

        yylex_destroy(scanner);

        /*txprotodebug = 0;*/
    }

    static const unsigned char *cmdstr[] = {"CMD_EMPTY",
                                            "CMD_PLAIN",
                                            "CMD_TXCHAL",
                                            "CMD_TXRESP",
                                            "CMD_TXCMMT",
                                            "CMD_TXABRT",
                                            "CMD_TXDATA",
                                            "CMD_TXEOTX",
                                            "CMD_TXNACK",
                                            "CMD_TXNNDO"};

    fprintf(stderr, "received packet cmd %s\n", cmdstr[packet->cmd]);
    
    return 0;
}

static int
txproto_send(int fd, const unsigned char *msg, size_t len, int flags)
{
    while (len) {

        ssize_t diff;

        do {
            diff = send(fd, msg, len, flags);
        } while ((diff < 0) && (errno == EINTR));

        if (diff < 0) {
            perror("send");
            return -1;
        }

        msg += diff;
        len -= diff;
    }

    return 0;
}

int
txproto_send_txchal(int fd)
{
    static const char msg[] = "TXCHAL\n";

    return txproto_send(fd, msg, strlen(msg), 0);
}

int
txproto_send_txresp(int fd)
{
    static const char msg[] = "TXRESP\n";

    return txproto_send(fd, msg, strlen(msg), 0);
}

ssize_t
txproto_send_txdata(int fd, const void *buffer, size_t length)
{
    static const char msg[] = "TXDATA ";

    if (txproto_send(fd, msg, strlen(msg), 0) < 0) {
        return -1;
    }

    size_t length64;
    unsigned char *buffer64 = encode64(buffer, length, &length64);

    if (txproto_send(fd, buffer64, length64, 0) < 0) {
        return -1;
    }

    free(buffer64);

    if (txproto_send(fd, "\n", strlen("\n"), 0) < 0) {
        return -1;
    }

    return length;
}

int
txproto_send_txeotx(int fd)
{
    static const char msg[] = "TXEOTX\n";

    return txproto_send(fd, msg, strlen(msg), 0);
}

int
txproto_send_txcmmt(int fd)
{
    static const char msg[] = "TXCMMT\n";

    return txproto_send(fd, msg, strlen(msg), 0);
}

int
txproto_send_txabrt(int fd)
{
    static const char msg[] = "TXABRT\n";

    return txproto_send(fd, msg, strlen(msg), 0);
}

int
txproto_send_txnndo(int fd)
{
    static const char msg[] = "TXNNDO\n";

    return txproto_send(fd, msg, strlen(msg), 0);
}

