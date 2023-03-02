#include "libzyutil_encrypt_MD5.h"
#include "stdlib.h"
#include "string.h"
#include <openssl/md5.h>

/*!
 *  get a MD5 encrypted string
 *
 *  @param [in] input               input of MD5 encryption
 *  @param [out] expected           output of MD5 encryption
 *
 */
void getExpectedContext_Openssl_MD5(char *input, unsigned char *expected)
{
    if(expected == NULL || input == NULL)
    {
        return;
    }

    MD5((unsigned char*)input, strlen(input), expected);
}

