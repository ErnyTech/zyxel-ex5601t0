/* wolfssl options.h
 * generated from configure options
 *
 * Copyright (C) 2006-2020 wolfSSL Inc.
 *
 * This file is part of wolfSSL. (formerly known as CyaSSL)
 *
 */

#ifndef WOLFSSL_OPTIONS_H
#define WOLFSSL_OPTIONS_H


#ifdef __cplusplus
extern "C" {
#endif

#ifndef WOLFSSL_OPTIONS_IGNORE_SYS
#undef  _FORTIFY_SOURCE
#define _FORTIFY_SOURCE 1
#endif

#undef  PIC
#define PIC

#undef  FP_MAX_BITS
#define FP_MAX_BITS 8192

#undef  WOLFSSL_ALT_CERT_CHAINS
#define WOLFSSL_ALT_CERT_CHAINS

#undef  WOLFSSL_TLS13
#define WOLFSSL_TLS13

#undef  HAVE_TLS_EXTENSIONS
#define HAVE_TLS_EXTENSIONS

#undef  HAVE_SUPPORTED_CURVES
#define HAVE_SUPPORTED_CURVES

#undef  HAVE_FFDHE_2048
#define HAVE_FFDHE_2048

#undef  HAVE_EXT_CACHE
#define HAVE_EXT_CACHE

#undef  WOLFSSL_VERIFY_CB_ALL_CERTS
#define WOLFSSL_VERIFY_CB_ALL_CERTS

#undef  WOLFSSL_EXTRA_ALERTS
#define WOLFSSL_EXTRA_ALERTS

#undef  OPENSSL_EXTRA
#define OPENSSL_EXTRA

#undef  WOLFSSL_ALWAYS_VERIFY_CB
#define WOLFSSL_ALWAYS_VERIFY_CB

#undef  OPENSSL_ALL
#define OPENSSL_ALL

#undef  WOLFSSL_EITHER_SIDE
#define WOLFSSL_EITHER_SIDE

#undef  WC_RSA_NO_PADDING
#define WC_RSA_NO_PADDING

#undef  WC_RSA_PSS
#define WC_RSA_PSS

#undef  WOLFSSL_PSS_LONG_SALT
#define WOLFSSL_PSS_LONG_SALT

#ifndef WOLFSSL_OPTIONS_IGNORE_SYS
#undef  _POSIX_THREADS
#define _POSIX_THREADS
#endif

#undef  HAVE_THREAD_LS
#define HAVE_THREAD_LS

#undef  TFM_TIMING_RESISTANT
#define TFM_TIMING_RESISTANT

#undef  ECC_TIMING_RESISTANT
#define ECC_TIMING_RESISTANT

#undef  WC_RSA_BLINDING
#define WC_RSA_BLINDING

#undef  TEST_IPV6
#define TEST_IPV6

#undef  WOLFSSL_IPV6
#define WOLFSSL_IPV6

#undef  WOLFSSL_WPAS
#define WOLFSSL_WPAS

#undef  HAVE_SECRET_CALLBACK
#define HAVE_SECRET_CALLBACK

#undef  WOLFSSL_PUBLIC_ECC_ADD_DBL
#define WOLFSSL_PUBLIC_ECC_ADD_DBL

#undef  WOLFSSL_ALWAYS_VERIFY_CB
#define WOLFSSL_ALWAYS_VERIFY_CB

#undef  WOLFSSL_ALWAYS_KEEP_SNI
#define WOLFSSL_ALWAYS_KEEP_SNI

#undef  HAVE_EX_DATA
#define HAVE_EX_DATA

#undef  HAVE_EXT_CACHE
#define HAVE_EXT_CACHE

#undef  WOLFSSL_EITHER_SIDE
#define WOLFSSL_EITHER_SIDE

#undef  OPENSSL_EXTRA_X509_SMALL
#define OPENSSL_EXTRA_X509_SMALL

#undef  WOLFSSL_PUBLIC_MP
#define WOLFSSL_PUBLIC_MP

#undef  WOLFSSL_DER_LOAD
#define WOLFSSL_DER_LOAD

#undef  ATOMIC_USER
#define ATOMIC_USER

#undef  WOLFSSL_KEY_GEN
#define WOLFSSL_KEY_GEN

#undef  WOLFSSL_DES_ECB
#define WOLFSSL_DES_ECB

#undef  FORTRESS
#define FORTRESS

#undef  WOLFSSL_ALWAYS_VERIFY_CB
#define WOLFSSL_ALWAYS_VERIFY_CB

#undef  WOLFSSL_AES_COUNTER
#define WOLFSSL_AES_COUNTER

#undef  WOLFSSL_AES_DIRECT
#define WOLFSSL_AES_DIRECT

#undef  WOLFSSL_DER_LOAD
#define WOLFSSL_DER_LOAD

#undef  WOLFSSL_KEY_GEN
#define WOLFSSL_KEY_GEN

#undef  HAVE_AESCCM
#define HAVE_AESCCM

#undef  WOLFSSL_USE_ALIGN
#define WOLFSSL_USE_ALIGN

#undef  WOLFSSL_SHA224
#define WOLFSSL_SHA224

#undef  WOLFSSL_SHA512
#define WOLFSSL_SHA512

#undef  WOLFSSL_SHA384
#define WOLFSSL_SHA384

#undef  SESSION_CERTS
#define SESSION_CERTS

#undef  WOLFSSL_CERT_GEN
#define WOLFSSL_CERT_GEN

#undef  HAVE_HKDF
#define HAVE_HKDF

#undef  HAVE_ECC
#define HAVE_ECC

#undef  TFM_ECC256
#define TFM_ECC256

#undef  ECC_SHAMIR
#define ECC_SHAMIR

#undef  HAVE_COMP_KEY
#define HAVE_COMP_KEY

#undef  WOLFSSL_ALLOW_TLSV10
#define WOLFSSL_ALLOW_TLSV10

#undef  WC_RSA_PSS
#define WC_RSA_PSS

#undef  HAVE_ANON
#define HAVE_ANON

#undef  WOLFSSL_CMAC
#define WOLFSSL_CMAC

#undef  WOLFSSL_AES_DIRECT
#define WOLFSSL_AES_DIRECT

#undef  NO_HC128
#define NO_HC128

#undef  NO_RABBIT
#define NO_RABBIT

#undef  WOLFSSL_SHA3
#define WOLFSSL_SHA3

#undef  HAVE_POLY1305
#define HAVE_POLY1305

#undef  HAVE_ONE_TIME_AUTH
#define HAVE_ONE_TIME_AUTH

#undef  HAVE_CHACHA
#define HAVE_CHACHA

#undef  HAVE_HASHDRBG
#define HAVE_HASHDRBG

#undef  HAVE_OCSP
#define HAVE_OCSP

#undef  HAVE_OPENSSL_CMD
#define HAVE_OPENSSL_CMD

#undef  HAVE_TLS_EXTENSIONS
#define HAVE_TLS_EXTENSIONS

#undef  HAVE_CERTIFICATE_STATUS_REQUEST
#define HAVE_CERTIFICATE_STATUS_REQUEST

#undef  HAVE_TLS_EXTENSIONS
#define HAVE_TLS_EXTENSIONS

#undef  HAVE_CERTIFICATE_STATUS_REQUEST_V2
#define HAVE_CERTIFICATE_STATUS_REQUEST_V2

#undef  HAVE_CRL
#define HAVE_CRL

#undef  HAVE_TLS_EXTENSIONS
#define HAVE_TLS_EXTENSIONS

#undef  HAVE_SNI
#define HAVE_SNI

#undef  HAVE_TLS_EXTENSIONS
#define HAVE_TLS_EXTENSIONS

#undef  HAVE_SUPPORTED_CURVES
#define HAVE_SUPPORTED_CURVES

#undef  HAVE_TLS_EXTENSIONS
#define HAVE_TLS_EXTENSIONS

#undef  HAVE_SESSION_TICKET
#define HAVE_SESSION_TICKET

#undef  HAVE_EXTENDED_MASTER
#define HAVE_EXTENDED_MASTER

#undef  HAVE_TLS_EXTENSIONS
#define HAVE_TLS_EXTENSIONS

#undef  HAVE_SNI
#define HAVE_SNI

#undef  HAVE_MAX_FRAGMENT
#define HAVE_MAX_FRAGMENT

#undef  HAVE_TRUNCATED_HMAC
#define HAVE_TRUNCATED_HMAC

#undef  HAVE_ALPN
#define HAVE_ALPN

#undef  HAVE_TRUSTED_CA
#define HAVE_TRUSTED_CA

#undef  HAVE_SUPPORTED_CURVES
#define HAVE_SUPPORTED_CURVES

#undef  HAVE_LIGHTY
#define HAVE_LIGHTY

#undef  HAVE_WOLFSSL_SSL_H
#define HAVE_WOLFSSL_SSL_H 1

#undef  HAVE_EX_DATA
#define HAVE_EX_DATA

#undef  OPENSSL_ALL
#define OPENSSL_ALL

#undef  OPENSSL_NO_SSL2
#define OPENSSL_NO_SSL2

#undef  OPENSSL_NO_COMP
#define OPENSSL_NO_COMP

#undef  OPENSSL_NO_SSL3
#define OPENSSL_NO_SSL3

#undef  SINGLE_THREADED
#define SINGLE_THREADED

#undef  WOLFSSL_ALWAYS_VERIFY_CB
#define WOLFSSL_ALWAYS_VERIFY_CB

#undef  WOLFSSL_ALWAYS_KEEP_SNI
#define WOLFSSL_ALWAYS_KEEP_SNI

#undef  KEEP_OUR_CERT
#define KEEP_OUR_CERT

#undef  KEEP_PEER_CERT
#define KEEP_PEER_CERT

#undef  HAVE_EXT_CACHE
#define HAVE_EXT_CACHE

#undef  HAVE_EX_DATA
#define HAVE_EX_DATA

#undef  WOLFSSL_CERT_GEN
#define WOLFSSL_CERT_GEN

#undef  HAVE_ENCRYPT_THEN_MAC
#define HAVE_ENCRYPT_THEN_MAC

#undef  WOLFSSL_CERT_EXT
#define WOLFSSL_CERT_EXT

#undef  HAVE_STUNNEL
#define HAVE_STUNNEL

#undef  WOLFSSL_ALWAYS_VERIFY_CB
#define WOLFSSL_ALWAYS_VERIFY_CB

#undef  WOLFSSL_ALWAYS_KEEP_SNI
#define WOLFSSL_ALWAYS_KEEP_SNI

#undef  HAVE_EX_DATA
#define HAVE_EX_DATA

#undef  WOLFSSL_DES_ECB
#define WOLFSSL_DES_ECB

#undef  WOLFSSL_SIGNER_DER_CERT
#define WOLFSSL_SIGNER_DER_CERT

#undef  WOLFSSL_ENCRYPTED_KEYS
#define WOLFSSL_ENCRYPTED_KEYS

#undef  USE_FAST_MATH
#define USE_FAST_MATH

#undef  WOLFSSL_AARCH64_BUILD
#define WOLFSSL_AARCH64_BUILD

#undef  WC_NO_ASYNC_THREADING
#define WC_NO_ASYNC_THREADING

#undef  HAVE_AES_KEYWRAP
#define HAVE_AES_KEYWRAP

#undef  WOLFSSL_AES_DIRECT
#define WOLFSSL_AES_DIRECT

#undef  HAVE_DH_DEFAULT_PARAMS
#define HAVE_DH_DEFAULT_PARAMS

#undef  GCM_TABLE_4BIT
#define GCM_TABLE_4BIT

#undef  HAVE_AESGCM
#define HAVE_AESGCM

#undef  HAVE___UINT128_T
#define HAVE___UINT128_T 1

#undef  HAVE_WC_INTROSPECTION
#define HAVE_WC_INTROSPECTION


#ifdef __cplusplus
}
#endif


#endif /* WOLFSSL_OPTIONS_H */
