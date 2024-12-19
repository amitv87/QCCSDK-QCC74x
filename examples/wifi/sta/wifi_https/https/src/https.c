#include <stdio.h>
#include <string.h>
#include <FreeRTOS.h>
#include <task.h>
#include <queue.h>
#include <timers.h>

#include <lwip/sockets.h>
#include <lwip/tcp.h>
#include <lwip/err.h>
#include <lwip/dns.h>
#include <lwip/netdb.h>

#include <mbedtls/net.h>
#include <mbedtls/debug.h>
#include <mbedtls/ssl.h>
#include <mbedtls/entropy.h>
#include <mbedtls/ctr_drbg.h>
#include <mbedtls/error.h>
#include <mbedtls/certs.h>

#include "qcc74x_error.h"
#include "https.h"

#define DBG_TAG "HTTP/S"
#include "log.h"

//#define QCC74x_VERIFY

#if 0
#if defined(QCC74x_VERIFY)
const char qcc74x_test_cli_key_rsa[] =
"-----BEGIN CERTIFICATE-----\r\n"
"MIIDjTCCAnUCFBjPxIX+hViDcOzz3QE1u250j+a5MA0GCSqGSIb3DQEBCwUAMIGC\r\n"
"MQswCQYDVQQGEwJDTjEQMA4GA1UECAwHamlhbmdzdTEQMA4GA1UEBwwHbmFuamlu\r\n"
"ZzEPMA0GA1UECgwGbmp0ZWNoMQwwCgYDVQQLDANDU1QxDzANBgNVBAMMBnlpbWlu\r\n"
"ZzEfMB0GCSqGSIb3DQEJARYQODc2NzExMDIyQHFxLmNvbTAeFw0xOTA3MzAwODMx\r\n"
"NDNaFw0yMDA3MjkwODMxNDNaMIGCMQswCQYDVQQGEwJDTjEQMA4GA1UECAwHamlh\r\n"
"bmdzdTEQMA4GA1UEBwwHbmFuamluZzEPMA0GA1UECgwGbmp0ZWNoMQwwCgYDVQQL\r\n"
"DANDU1QxDzANBgNVBAMMBnlpbWluZzEfMB0GCSqGSIb3DQEJARYQODc2NzExMDIy\r\n"
"QHFxLmNvbTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBAKLcS3nEivyQ\r\n"
"rU1ehmr2uDSEf/PvRxmEjlff+S39nbtvxc/Ed1wUGSVmAee9bqjj9bEej/UGnZLN\r\n"
"4onfW6MC6WoDUmvVyq00Tcyq1hzpzRPhl8nCz9BI7hizsmlh3L10Mff+Jd6xzCk2\r\n"
"StKQXWbmpLOdqsZDIdumHDb8EGSQrv/J2o8JKCxyQ4gP9td48Z9bjeLIZPmhK8Lm\r\n"
"LVgGR2s87MaVfjCde4mefgJA+mkavB2/weNi5s+LxaYQ2T53NO39t/2yZ32VQyb7\r\n"
"6/NKf2jWniaKd01BFPLYpuVj8uR1eCn8SzVc84CP4TG96NdPgTT8fnHJHPxUQFMt\r\n"
"jNuJCWpxKOsCAwEAATANBgkqhkiG9w0BAQsFAAOCAQEAhvDTyNz0rZrgVSrtVE3B\r\n"
"zTo1t+8xFgWgsHJ5VT2s4O9W5RX+RpjaOp33cLFjSCM2jhxUZQSJTbZEnV3vlMOI\r\n"
"LLhCl7kWHaZt71oMgNeb32qRgTW197ogZnvkp7f+7RObxGAWJ1J3F11+sBT711HB\r\n"
"XARFQSdYfq8j1hxnlLR3rMntW8BIEk/1iz+qhYMXiwkTHQhTgX1308TLID2XtaEh\r\n"
"drTFzUuEJ0OGv3BWVyEtBM2xwX/5Xsjy2snAus66gI4OJEaBcJoDogNiFQm/PwvU\r\n"
"siFwHDQaP4athzbf2NvR0RVzWWyXP3Pj8puH0CJRjFddexdcd4NgrZojU3f7aZfs\r\n"
"eA==\r\n"
"-----END CERTIFICATE-----\r\n";

const int32_t qcc74x_test_cas_pem_len = sizeof(qcc74x_test_cli_key_rsa);
#endif
#endif

#if defined(QCC74x_VERIFY)
// ipinfo.io root key
const char qcc74x_test_cli_key_rsa[] =
"-----BEGIN CERTIFICATE-----\r\n"
"MIIFBjCCAu6gAwIBAgIRAIp9PhPWLzDvI4a9KQdrNPgwDQYJKoZIhvcNAQELBQAw\r\n"
"TzELMAkGA1UEBhMCVVMxKTAnBgNVBAoTIEludGVybmV0IFNlY3VyaXR5IFJlc2Vh\r\n"
"cmNoIEdyb3VwMRUwEwYDVQQDEwxJU1JHIFJvb3QgWDEwHhcNMjQwMzEzMDAwMDAw\r\n"
"WhcNMjcwMzEyMjM1OTU5WjAzMQswCQYDVQQGEwJVUzEWMBQGA1UEChMNTGV0J3Mg\r\n"
"RW5jcnlwdDEMMAoGA1UEAxMDUjExMIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIB\r\n"
"CgKCAQEAuoe8XBsAOcvKCs3UZxD5ATylTqVhyybKUvsVAbe5KPUoHu0nsyQYOWcJ\r\n"
"DAjs4DqwO3cOvfPlOVRBDE6uQdaZdN5R2+97/1i9qLcT9t4x1fJyyXJqC4N0lZxG\r\n"
"AGQUmfOx2SLZzaiSqhwmej/+71gFewiVgdtxD4774zEJuwm+UE1fj5F2PVqdnoPy\r\n"
"6cRms+EGZkNIGIBloDcYmpuEMpexsr3E+BUAnSeI++JjF5ZsmydnS8TbKF5pwnnw\r\n"
"SVzgJFDhxLyhBax7QG0AtMJBP6dYuC/FXJuluwme8f7rsIU5/agK70XEeOtlKsLP\r\n"
"Xzze41xNG/cLJyuqC0J3U095ah2H2QIDAQABo4H4MIH1MA4GA1UdDwEB/wQEAwIB\r\n"
"hjAdBgNVHSUEFjAUBggrBgEFBQcDAgYIKwYBBQUHAwEwEgYDVR0TAQH/BAgwBgEB\r\n"
"/wIBADAdBgNVHQ4EFgQUxc9GpOr0w8B6bJXELbBeki8m47kwHwYDVR0jBBgwFoAU\r\n"
"ebRZ5nu25eQBc4AIiMgaWPbpm24wMgYIKwYBBQUHAQEEJjAkMCIGCCsGAQUFBzAC\r\n"
"hhZodHRwOi8veDEuaS5sZW5jci5vcmcvMBMGA1UdIAQMMAowCAYGZ4EMAQIBMCcG\r\n"
"A1UdHwQgMB4wHKAaoBiGFmh0dHA6Ly94MS5jLmxlbmNyLm9yZy8wDQYJKoZIhvcN\r\n"
"AQELBQADggIBAE7iiV0KAxyQOND1H/lxXPjDj7I3iHpvsCUf7b632IYGjukJhM1y\r\n"
"v4Hz/MrPU0jtvfZpQtSlET41yBOykh0FX+ou1Nj4ScOt9ZmWnO8m2OG0JAtIIE38\r\n"
"01S0qcYhyOE2G/93ZCkXufBL713qzXnQv5C/viOykNpKqUgxdKlEC+Hi9i2DcaR1\r\n"
"e9KUwQUZRhy5j/PEdEglKg3l9dtD4tuTm7kZtB8v32oOjzHTYw+7KdzdZiw/sBtn\r\n"
"UfhBPORNuay4pJxmY/WrhSMdzFO2q3Gu3MUBcdo27goYKjL9CTF8j/Zz55yctUoV\r\n"
"aneCWs/ajUX+HypkBTA+c8LGDLnWO2NKq0YD/pnARkAnYGPfUDoHR9gVSp/qRx+Z\r\n"
"WghiDLZsMwhN1zjtSC0uBWiugF3vTNzYIEFfaPG7Ws3jDrAMMYebQ95JQ+HIBD/R\r\n"
"PBuHRTBpqKlyDnkSHDHYPiNX3adPoPAcgdF3H2/W0rmoswMWgTlLn1Wu0mrks7/q\r\n"
"pdWfS6PJ1jty80r2VKsM/Dj3YIDfbjXKdaFU5C+8bhfJGqU3taKauuz0wHVGT3eo\r\n"
"6FlWkWYtbt4pgdamlwVeZEW+LM7qZEJEsMNPrfC03APKmZsJgpWCDWOKZvkZcvjV\r\n"
"uYkQ4omYCTX5ohy+knMjdOmdH9c7SpqEWBDC86fiNex+O0XOMEZSa8DA\r\n"
"-----END CERTIFICATE-----\r\n";
const int32_t qcc74x_test_cas_pem_len = sizeof(qcc74x_test_cli_key_rsa);
#endif

typedef struct _https_context {
    mbedtls_ssl_context ssl;
    mbedtls_ssl_config conf;
    mbedtls_entropy_context entropy;
    mbedtls_ctr_drbg_context ctr_drbg;
    mbedtls_net_context server_fd;
#if defined(QCC74x_VERIFY)
    mbedtls_x509_crt cacert;
#endif
} https_context_t;

https_context_t *qcc74x_hsbuf = NULL;

static void qcc74x_debug( void *ctx, int level,
                      const char *file, int line,
                      const char *str )
{
    ((void) level);

    LOG_I("%s:%04d: %s\r\n", file, line, str);
}

static int is_valid_ip_address(const char *ipAddress)
{
    struct sockaddr_in sa;
    int result = inet_pton(AF_INET, ipAddress, &(sa.sin_addr));

    return result != 0;
}

int32_t qcc74x_TcpSslConnect(const char *dst, uint16_t port)
{
    in_addr_t dst_addr;
    int ret;

    struct sockaddr_in servaddr;
    int flags;
    int reuse = 1;

    if (NULL != qcc74x_hsbuf) {
        return -1;
    }
    qcc74x_hsbuf = malloc(sizeof(https_context_t));
    if (NULL == qcc74x_hsbuf) {
        return -1;
    }
    if (NULL == dst) {
        return QCC74x_TCP_ARG_INVALID;
    }

    if (is_valid_ip_address(dst)) {
        dst_addr = inet_addr(dst);
    } else {
        struct hostent *hostinfo = gethostbyname(dst);
        if (!hostinfo) {
            return -1;
        }
        dst_addr = ((struct in_addr *) hostinfo->h_addr)->s_addr;
        printf("dst_addr is %08lX\n", *(uint32_t *)&dst_addr);
    }

    mbedtls_ssl_init(&qcc74x_hsbuf->ssl);

#if defined(QCC74x_VERIFY)
    mbedtls_x509_crt_init( &qcc74x_hsbuf->cacert );
#endif

    mbedtls_ctr_drbg_init(&qcc74x_hsbuf->ctr_drbg);
    mbedtls_ssl_config_init(&qcc74x_hsbuf->conf);
    mbedtls_entropy_init(&qcc74x_hsbuf->entropy);

    if ((ret = mbedtls_ctr_drbg_seed(&qcc74x_hsbuf->ctr_drbg, mbedtls_entropy_func, &qcc74x_hsbuf->entropy,
                                     NULL, 0)) != 0) {
        return QCC74x_TCP_CREATE_CONNECT_ERR;
    }

#if defined(QCC74x_VERIFY)
    ret = mbedtls_x509_crt_parse( &qcc74x_hsbuf->cacert, (const unsigned char *)qcc74x_test_cli_key_rsa,
                                   qcc74x_test_cas_pem_len );

    if (ret < 0) {
        printf("\r\n mbedtls_x509_crt_parse returned -0x%x\r\n", -ret);
    }
#endif

    if ((ret = mbedtls_ssl_config_defaults(&qcc74x_hsbuf->conf,
                                           MBEDTLS_SSL_IS_CLIENT,
                                           MBEDTLS_SSL_TRANSPORT_STREAM,
                                           MBEDTLS_SSL_PRESET_DEFAULT)) != 0) {
        printf("mbedtls_ssl_config_defaults returned %d", ret);
        return QCC74x_TCP_CREATE_CONNECT_ERR;
    }

#if defined(QCC74x_VERIFY)
    mbedtls_ssl_conf_authmode(&qcc74x_hsbuf->conf, MBEDTLS_SSL_VERIFY_REQUIRED/*MBEDTLS_SSL_VERIFY_OPTIONAL*/);
    mbedtls_ssl_conf_ca_chain( &qcc74x_hsbuf->conf, &qcc74x_hsbuf->cacert, NULL );
#else
    mbedtls_ssl_conf_authmode(&qcc74x_hsbuf->conf, MBEDTLS_SSL_VERIFY_NONE);
#endif

    mbedtls_ssl_conf_rng(&qcc74x_hsbuf->conf, mbedtls_ctr_drbg_random, &qcc74x_hsbuf->ctr_drbg);

    //todo
    mbedtls_ssl_conf_read_timeout(&qcc74x_hsbuf->conf, 0);
    //mbedtls_ssl_set_timer_cb(&ssl, &ssl_timer, f_set_timer, f_get_timer);

    mbedtls_ssl_conf_dbg( &qcc74x_hsbuf->conf, qcc74x_debug, stdout );

#if defined(MBEDTLS_DEBUG_C)
    // mbedtls_debug_set_threshold(4);
#endif

    if ((ret = mbedtls_ssl_setup(&qcc74x_hsbuf->ssl, &qcc74x_hsbuf->conf)) != 0) {
        printf("mbedtls_ssl_setup returned -0x%x\r\n", -ret);
        return QCC74x_TCP_CREATE_CONNECT_ERR;
    }

    mbedtls_net_init(&qcc74x_hsbuf->server_fd);

    qcc74x_hsbuf->server_fd.fd = socket(AF_INET, SOCK_STREAM, 0);

    if (qcc74x_hsbuf->server_fd.fd < 0) {
        printf("ssl creat socket fd failed\r\n");
        return QCC74x_TCP_CREATE_CONNECT_ERR;
    }

    flags = fcntl(qcc74x_hsbuf->server_fd.fd, F_GETFL, 0);
    if (flags < 0 || fcntl(qcc74x_hsbuf->server_fd.fd, F_SETFL, flags | O_NONBLOCK) < 0) {
        printf("ssl fcntl: %s\r\n", strerror(errno));
        close(qcc74x_hsbuf->server_fd.fd);
        return QCC74x_TCP_CREATE_CONNECT_ERR;
    }

    if (setsockopt(qcc74x_hsbuf->server_fd.fd, SOL_SOCKET, SO_REUSEADDR,
                   (const char*) &reuse, sizeof(reuse)) != 0) {
        close(qcc74x_hsbuf->server_fd.fd);
        printf("ssl set SO_REUSEADDR failed\r\n");
        return QCC74x_TCP_CREATE_CONNECT_ERR;
    }

    memset(&servaddr, 0, sizeof(struct sockaddr_in));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = dst_addr;
    servaddr.sin_port = htons(port);

    if (connect(qcc74x_hsbuf->server_fd.fd, (struct sockaddr*)&servaddr, sizeof(struct sockaddr_in)) == 0) {
        //printf("ssl dst %s errno %d\r\n", dst, errno);
    } else {
        //printf("ssl dst %s errno %d\r\n", dst, errno);
        if (errno == EINPROGRESS) {
            //printf("ssl tcp conncet noblock\r\n");
        } else {
            close(qcc74x_hsbuf->server_fd.fd);
            return QCC74x_TCP_CREATE_CONNECT_ERR;
        }
    }

    //todo
    //mbedtls_ssl_set_bio(&qcc74x_hsbuf->ssl, &qcc74x_hsbuf->server_fd, mbedtls_net_send, mbedtls_net_recv, mbedtls_net_recv_timeout); //noblock
    mbedtls_ssl_set_bio(&qcc74x_hsbuf->ssl, &qcc74x_hsbuf->server_fd, mbedtls_net_send, mbedtls_net_recv, NULL);
    return (int32_t)&qcc74x_hsbuf->ssl;
}

void qcc74x_TcpSslDisconnect(int32_t fd)
{
    mbedtls_ssl_context *pssl = (mbedtls_ssl_context *)fd;

    if (NULL == pssl) {
        printf("qcc74x_TcpSslDisconnect\r\n");
    }

    mbedtls_ssl_close_notify(pssl);

    mbedtls_net_free((mbedtls_net_context*)(pssl->p_bio));

#if defined(QCC74x_VERIFY)
    mbedtls_x509_crt_free( &qcc74x_hsbuf->cacert );
#endif

    mbedtls_ssl_free( pssl );
    mbedtls_ssl_config_free( &qcc74x_hsbuf->conf );
    mbedtls_ctr_drbg_free( &qcc74x_hsbuf->ctr_drbg );
    mbedtls_entropy_free( &qcc74x_hsbuf->entropy );

    free(qcc74x_hsbuf);

    qcc74x_hsbuf = NULL;
    printf("qcc74x_TcpSslDisconnect end\r\n");
}

int32_t qcc74x_TcpSslState(int32_t fd)
{
    //printf("qcc74x_TcpSslState start\r\n");
    int errcode = QCC74x_TCP_NO_ERROR;
    mbedtls_ssl_context *pssl = (mbedtls_ssl_context *)fd;
    int tcp_fd = ((mbedtls_net_context*)(pssl->p_bio))->fd;
    int ret;

    fd_set rset, wset;
    int ready_n;

    struct timeval timeout;

    socklen_t len =  sizeof(int);

    if (tcp_fd < 0 || NULL == pssl) {
        return QCC74x_TCP_ARG_INVALID;
    }

    FD_ZERO(&rset);
    FD_ZERO(&wset);
    FD_CLR(tcp_fd, &rset);
    FD_CLR(tcp_fd, &wset);
    FD_SET(tcp_fd, &rset);
    FD_SET(tcp_fd, &wset);

    timeout.tv_sec = 2;
    timeout.tv_usec = 0;

    ready_n = select(tcp_fd + 1, &rset, &wset, NULL, &timeout);

    if (0 == ready_n) {
        errcode = QCC74x_TCP_CONNECTING;
    } else if (ready_n < 0) {
        errcode = QCC74x_TCP_CONNECT_ERR;
    } else {
        if (FD_ISSET(tcp_fd, &wset) != 0) {
            errcode = QCC74x_TCP_CONNECTING;

            if(pssl->state != MBEDTLS_SSL_HANDSHAKE_OVER) {
                ret = mbedtls_ssl_handshake_step( pssl );
                // printf("mbedtls_ssl_handshake_step return = 0X%X\r\n", -ret);

                if ((0 != ret) && (MBEDTLS_ERR_SSL_WANT_READ != ret)) {
                    errcode = QCC74x_TCP_CONNECT_ERR;
                }
            } else {
                errcode = QCC74x_TCP_NO_ERROR;
            }
        } else {
            if (0 != getsockopt(tcp_fd, SOL_SOCKET, SO_ERROR, &ret, &len)) {
                errcode = QCC74x_TCP_CONNECT_ERR;
            }
            if (0 != ret) {
                errcode = QCC74x_TCP_CONNECT_ERR;
            }
            errcode = QCC74x_TCP_CONNECT_ERR;
        }
    }

    return errcode;
}

int32_t qcc74x_TcpSslSend(int32_t fd, const uint8_t* buf, uint16_t len)
{
    //puts("qcc74x_TcpSslSend start\r\n");
    int ret = 0;
    mbedtls_ssl_context *pssl = (mbedtls_ssl_context *)fd;

    if (NULL == buf || NULL == pssl) {
        return QCC74x_TCP_ARG_INVALID;
    }

    ret = mbedtls_ssl_write(pssl, buf, len);

    if(ret > 0) {
        return ret;
    } else {
        printf("qcc74x_TcpSslsend error ret = 0X%X\r\n", -ret);
        return QCC74x_TCP_SEND_ERR;
    }
    return ret;
}

int32_t qcc74x_TcpSslRead(int32_t fd, uint8_t* buf, uint16_t len)
{
    int ret = 0;
    mbedtls_ssl_context *pssl = (mbedtls_ssl_context *)fd;

    if (NULL == buf || NULL == pssl) {
        return QCC74x_TCP_ARG_INVALID;
    }

    ret = mbedtls_ssl_read(pssl, buf, len);

    if(ret > 0) {
        return ret;
    } else if(MBEDTLS_ERR_SSL_WANT_READ == ret) {
        return 0;
    } else {
        printf("qcc74x_TcpSslRead ret = 0X%X\r\n", ret);
        return QCC74x_TCP_READ_ERR;
    }
}
