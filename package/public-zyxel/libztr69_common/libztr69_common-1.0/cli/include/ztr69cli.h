#ifndef _ZTR69CLI_H_
#define _ZTR69CLI_H_
typedef struct ztr69_common_clihandler_s{
    char Name[32];
    int (*handler)(int argc, char **argv);
}ztr69_common_clihandler_t;

typedef struct error_message_s
{
    int errorno;
    char error_message[256];
}error_message_t;
#endif