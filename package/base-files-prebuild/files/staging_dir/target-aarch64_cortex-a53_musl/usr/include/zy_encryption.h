#ifndef ZYENCRYPTION_H
#define ZYENCRYPTION_H

typedef enum
{
    ZY_SHIFT,
    ZY_BASE64,
    ZY_AES256_BASE64,
    NONE
} zy_algo_t;


// ===== AES =====
#define ZY_AES_STR_MAX 16
#define ZY_AES_KEY_MAX 32
#define ZY_AES_IV_MAX  16

typedef struct zyencrypt_s{
        unsigned char zy_key[ZY_AES_KEY_MAX*2 + 1];
        unsigned char zy_iv[ZY_AES_IV_MAX*2 + 1];
}zyencrypt_t;
// ===== AES =====

#if defined(ZYXEL_SUPPORT_ATCK_ENCRYPTION)
#define ZY_DEFAULT_ALGO ZY_AES256_BASE64
#else
#define ZY_DEFAULT_ALGO NONE
#endif

#define ZY_SHIFT_NUM 3

int zy_enc(zy_algo_t algo_type, const unsigned char *src, size_t src_len, unsigned char *out, size_t *out_len, zyencrypt_t *parm);
int zy_dec(zy_algo_t algo_type, const unsigned char *src, size_t src_len, unsigned char *out, size_t *out_len, zyencrypt_t *parm);
#endif
