
%option reentrant
%option debug
%option noyywrap

%{
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <tanger-stm-internal-errcode.h>
#include "txproto.h"
#include "txproto.tab.h"

%}

b64char     [A-Za-z0-9+/]

%%

[[:space:]]

TXCHAL      {return TOK_TXCHAL;}
TXCMMT      {return TOK_TXCMMT;}
TXABRT      {return TOK_TXABRT;}
TXDATA      {return TOK_TXDATA;}
TXEOTX      {return TOK_TXEOTX;}
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

    int numfds;

    if (s >= 0) {

        struct timeval timeout;
        timeout.tv_sec = s;
        timeout.tv_usec = 0;

        numfds = TEMP_FAILURE_RETRY(select(FD_SETSIZE, &rdset, NULL, NULL, &timeout));

    } else {
        numfds = TEMP_FAILURE_RETRY(select(FD_SETSIZE, &rdset, NULL, NULL, NULL));
    }

    if (numfds < 0) {
        perror("select");
    }

    int isset = FD_ISSET(fd, &rdset);

    FD_ZERO(&rdset);

    return isset;
}

/** \brief Consume specified number of bytes
 */
static ssize_t
txproto_consume(int fd, size_t len)
{
    unsigned char mem[256];

    do {
        size_t recvlen = TEMP_FAILURE_RETRY(recv(fd,
                                                 mem,
                                                 len < sizeof(mem) ?
                                                 len : sizeof(mem),
                                                 0));

        if (!(recvlen > 0)) {
            return ERR_SYSTEM;
        }
        len -= recvlen;

    } while (len);

    return len;
}

/** \brief Recv available bytes until delimiter
 */
static ssize_t
txproto_recvdelim(int fd, char **lineptr, size_t *n, int delim, int peek, int wait)
{
    assert(lineptr);
    assert(n);

    if (!*lineptr) {
        *n = 0;
    } else if (!*n) {
        *lineptr = NULL;
    }

    /* Read available bytes until delimiter */

    const char *bufnl = *lineptr;
    ssize_t len = 0;

    do {
        if (!txproto_wait_data(fd, -1)) {
            return ERR_SYSTEM;
        }

        int avail;

        if (ioctl(fd, FIONREAD, &avail) < 0) {
            perror("ioctl");
            abort();
        }

        /* Nothing to read */
        if (!avail) {
            return ERR_SYSTEM;
        }

        if (avail > *n) {

            void *tmp = realloc(*lineptr, avail);

            if (!tmp) {
                perror("realloc");
                abort();
            }

            *lineptr = tmp;
            *n = avail;
        }

        len = TEMP_FAILURE_RETRY(recv(fd, *lineptr, avail, MSG_PEEK));

        if (len < 0) {
            /* Error occured */
            perror("recv");
            return ERR_SYSTEM;
        } else if (!len) {
            /* Peer closed connection */
            return 0;
        }

        bufnl = memchr(*lineptr, delim, len);

    } while (wait && !bufnl);

    if (bufnl) {
        len = bufnl-*lineptr+1;
    }

    if (!peek) {

        /* Consume peeked data until delimiter */

        if (txproto_consume(fd, len) < 0) {
            return ERR_SYSTEM;
        }
    }

    return len;
}

/** \brief Parse buffer to packet
 */
static int
txproto_parsebuf(struct packet *p, char *buf, ssize_t len)
{
    assert(p);
    assert(buf);
    assert(len);

    yyscan_t scanner;
    yylex_init(&scanner);

    YY_BUFFER_STATE buffer = yy_scan_buffer(buf, len, scanner);

    if (!buffer) {
        abort();
        return -1;
    }

    yy_switch_to_buffer(buffer, scanner);

    int res = txprotoparse(p);

    yy_delete_buffer(buffer, scanner);
    yylex_destroy(scanner);

    return res;
}

int
txproto_recv_packet(int fd, struct packet *packet, int flags, int usedelim)
{
    assert(packet);

    char *buf = NULL;
    size_t buflen = 0;

    /* Recv available bytes until newline */

    ssize_t len;

    len = txproto_recvdelim(fd, &buf, &buflen, '\n', 1, !!usedelim);

    if (!(len > 0)) {
        return len;
    }

    if (!len) {
        abort();
        packet->cmd = CMD_EMPTY;
    }

    /* Parse line to packet */

    void *tmp = realloc(buf, len+1);

    if (!tmp) {
        perror("realloc");
        abort();
    }
    buf = tmp;

    buf[len-1] = YY_END_OF_BUFFER_CHAR;
    buf[len]   = YY_END_OF_BUFFER_CHAR;

    int res = txproto_parsebuf(packet, buf, len+1);

    if (res < 0) {
        return res;
    }

    /* Handle packet */

    switch (res) {
        case 0:
            /* Packet received, consume bytes from socket */
            if (txproto_consume(fd, len) < 0) {
                return ERR_SYSTEM;
            }
            break;
        case 1:
            /* Invalid input */
            packet->cmd = CMD_PLAIN;
            packet->data.msg = malloc(buflen);
            memcpy(packet->data.msg, buf, buflen);
            packet->data.len = buflen;
            break;
        case 2:
            /* Memory exhaused */
            errno = ENOMEM;
            return ERR_SYSTEM;
        default:
            /* Unknown error code */
            abort();
    }

    static const char *cmdstr[] = {"CMD_EMPTY",
                                   "CMD_PLAIN",
                                   "CMD_TXCHAL",
                                   "CMD_TXCMMT",
                                   "CMD_TXABRT",
                                   "CMD_TXDATA",
                                   "CMD_TXEOTX",
                                   "CMD_TXNNDO"};

    fprintf(stderr, "received packet cmd %s\n", cmdstr[packet->cmd]);

    return 0;
}

static int
txproto_send(int fd, const void *msg, size_t len, int flags)
{
    const unsigned char *cmsg = msg;

    while (len) {

        ssize_t diff;

        do {
            diff = send(fd, cmsg, len, flags);
        } while ((diff < 0) && (errno == EINTR));

        if (diff < 0) {
            perror("send");
            return -1;
        }

        cmsg += diff;
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

