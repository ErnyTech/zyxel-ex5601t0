#include "libzyutil_encrypt.h"
#include "libzyutil_encrypt_SHA256.h"
#include "libzyutil_encrypt_MD5.h"
#include "stdio.h"


/*!
 *  get a encrypted string
 *
 *  @param [in] encrypt          encryption algorithm
 *  @param [in] input            encryption input
 *  @param [out] output          encryption output
 *
 *  @return      0               successfully
 *               -1              fail
 */
int getExpectedContext(EncryptAlgo_t encrypt, char *input, unsigned char *output)
{
    int ret = 0;

    switch( encrypt )
    {
        case Encrypt_SHA256:
            getExpectedContext_SHA256(input, output);
            break;

        case Encrypt_OPENSSL_MD5:
            getExpectedContext_Openssl_MD5(input, output);
            break;

        default:
            printf("invalid encrypt = %d\n", encrypt);
            ret = -1;
            break;
    }

    return ret;
}

