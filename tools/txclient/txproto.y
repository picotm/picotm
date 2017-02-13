
%{

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "base64.h"
#include "txproto.h"

static unsigned char *
alloc_block(unsigned char c0,
            unsigned char c1,
            unsigned char c2,
            unsigned char c3)
{
    unsigned char *block = malloc(4);
    if (!block) {
        return NULL;
    }

    block[0] = c0;
    block[1] = c1;
    block[2] = c2;
    block[3] = c3;

    return block;
}

static int
append_block(struct block_sequence *seq, unsigned char *block)
{
    assert(seq);
    assert(block);

    void *tmp = realloc(seq->msg, seq->len+4);
    if (!tmp) {
        perror("realloc");
        return -1;
    }
    seq->msg = tmp;

    size_t i;

    for (i = 0; i < 4; ++i) {
        seq->msg[seq->len++] = block[i];
    }

    return 0;
}

static void
txprotoerror(struct packet *p, void *scanner, const char *s)
{
    fprintf(stderr, "%s\n", s);
}

%}

%pure-parser
%parse-param {struct packet *packet}
%parse-param {void *scanner}
%lex-param {void *scanner}

%union {
    struct packet         packet;
    struct block_sequence seq;
    unsigned char        *ucp;
    unsigned char         uc;
};

%token      TOK_TXCHAL;
%token      TOK_TXRESP;
%token      TOK_TXCMMT;
%token      TOK_TXABRT;
%token      TOK_TXDATA;
%token      TOK_TXEOTX;
%token      TOK_TXNACK;
%token      TOK_TXNNDO;
%token <uc> TOK_B64CHR;

%type <packet> command;
%type <packet> command_challenge;
%type <packet> command_response;
%type <packet> command_commit;
%type <packet> command_abort;
%type <packet> command_data;
%type <packet> command_eotx;
%type <packet> command_nack;
%type <packet> command_nndo;
%type <seq>    data;
%type <seq>    base64_data;
%type <seq>    base64_block_sequence;
%type <ucp>    base64_block;
%type <ucp>    base64_endblock;

%%

command
    : command_challenge {*packet = $1; YYACCEPT;}
    | command_response  {*packet = $1; YYACCEPT;}
    | command_commit    {*packet = $1; YYACCEPT;}
    | command_abort     {*packet = $1; YYACCEPT;}
    | command_data      {*packet = $1; YYACCEPT;}
    | command_eotx      {*packet = $1; YYACCEPT;}
    | command_nack      {*packet = $1; YYACCEPT;}
    | command_nndo      {*packet = $1; YYACCEPT;}
    ;

command_challenge
    : TOK_TXCHAL    {$$.cmd = CMD_TXCHAL;}
    ;

command_response
    : TOK_TXRESP    {$$.cmd = CMD_TXRESP;}
    ;

command_commit
    : TOK_TXCMMT    {$$.cmd = CMD_TXCMMT;}
    ;

command_abort
    : TOK_TXABRT    {$$.cmd = CMD_TXABRT;}
    ;

command_data
    : TOK_TXDATA data   {$$.cmd = CMD_TXDATA;
                         $$.data = $2;}
    ;

command_eotx
    : TOK_TXEOTX    {$$.cmd = CMD_TXEOTX;}
    ;

command_nack
    : TOK_TXNACK    {$$.cmd = CMD_TXNACK;}
    ;

command_nndo
    : TOK_TXNNDO    {$$.cmd = CMD_TXNNDO;}
    ;

data
    : base64_data   {size_t len;
                     unsigned char *msg = decode64($1.msg, $1.len, &len);
                     $$.msg = msg;
                     $$.len = len;}

base64_data
    : base64_block_sequence                    {$$ = $1}
    | base64_block_sequence base64_endblock    {$$ = $1;
                                                if (append_block(&$$, $2) < 0) {
                                                    YYABORT;
                                                } }
    ;

base64_block_sequence
    : base64_block                         {memset(&$$, 0, sizeof($$));
                                            if (append_block(&$$, $1) < 0) {
                                                YYABORT;
                                            } }
    | base64_block_sequence base64_block   {$$ = $1;
                                            if (append_block(&$$, $2) < 0) {
                                                YYABORT;
                                            } }
    ;

base64_block
    : TOK_B64CHR TOK_B64CHR TOK_B64CHR TOK_B64CHR   {if (!($$ = alloc_block($1,$2,$3,$4))) {
                                                         YYABORT;
                                                     } }
    ;

base64_endblock
    : TOK_B64CHR TOK_B64CHR TOK_B64CHR '='          {if (!($$ = alloc_block($1,$2,$3,'='))) {
                                                         YYABORT;
                                                     } }
    | TOK_B64CHR TOK_B64CHR '='        '='          {if (!($$ = alloc_block($1,$2,'=','='))) {
                                                         YYABORT;
                                                     } }
    ;

%%

