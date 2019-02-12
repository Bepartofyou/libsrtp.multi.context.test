#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "srtp2/srtp.h"

typedef int ngx_int_t;
typedef unsigned char u_char;
typedef struct {
    size_t      len;
    u_char     *data;
} ngx_str_t;

#define ngx_base64_decoded_length(len)  (((len + 3) / 4) * 3)
static ngx_int_t ngx_decode_base64_internal(ngx_str_t *dst, ngx_str_t *src, const u_char *basis);
static ngx_int_t ngx_decode_base64(ngx_str_t *dst, ngx_str_t *src);


int main(int argc, char *argv[])
{

    if(argc != 3){
        fprintf(stderr, "Usage: %s srtp.file srtp.key\n", argv[0]);
        exit(1);
    }

    fprintf(stderr, "Using %s [0x%x]\n", srtp_get_version_string(),
            srtp_get_version());

    srtp_err_status_t status;
    srtp_policy_t policy;
    srtp_ctx_t *srtp_ctx;

    srtp_policy_t policy_sub;
    srtp_ctx_t *srtp_ctx_sub;

    /* initialize srtp library */
    status = srtp_init();
    if (status) {
        fprintf(stderr,
                "error: srtp initialization failed with error code %d\n",
                status);
        exit(1);
    }

    ngx_str_t src, dst;
    src.len = strlen(argv[2]);
    src.data = malloc(src.len);
    memcpy(src.data, argv[2], src.len);
    dst.len =ngx_base64_decoded_length(src.len);
    dst.data = malloc(dst.len);

    fprintf(stderr, "key src len:%zu, dst len:%zu\n", src.len, dst.len);
    int rc = ngx_decode_base64(&dst, &src);
    if (rc != 0) {
        fprintf(stderr, "key base64 decode error\n");
        exit(1);
    }

    //////////// first srtp context ////////////////
    memset(&policy, 0, sizeof(policy));
    srtp_crypto_policy_set_aes_gcm_256_16_auth(&policy.rtp);
    srtp_crypto_policy_set_aes_gcm_256_16_auth(&policy.rtcp);
    policy.ssrc.type = ssrc_any_outbound;
    policy.ssrc.value = 0;
    policy.key = (unsigned char *)dst.data;
    policy.ekt = NULL;
    policy.next = NULL;
    policy.window_size = 1024;
    policy.allow_repeat_tx = 1;
    
    status = srtp_create(&srtp_ctx, &policy);
    if (status) {
        fprintf(stderr, "srtp create failed\n");
        exit(1);
    }

    ///////////// second srtp context ////////////////
    memset(&policy_sub, 0, sizeof(policy_sub));
    srtp_crypto_policy_set_aes_gcm_256_16_auth(&policy_sub.rtp);
    srtp_crypto_policy_set_aes_gcm_256_16_auth(&policy_sub.rtcp);
    policy_sub.ssrc.type = ssrc_any_outbound;
    policy_sub.ssrc.value = 0;
    policy_sub.key = (unsigned char *)dst.data;
    policy_sub.ekt = NULL;
    policy_sub.next = NULL;
    policy_sub.window_size = 1024;
    policy_sub.allow_repeat_tx = 1;
    
    status = srtp_create(&srtp_ctx_sub, &policy_sub);
    if (status) {
        fprintf(stderr, "srtp create failed\n");
        exit(1);
    }


    char name[100];
    sprintf(name, "./logs/%x.data.srtp", srtp_get_version());
    printf("out data:%s", name);
    FILE* file = fopen(argv[1], "rb");
    FILE* out = fopen(name, "wb");
    char buf[1500];
    int len = 0;
    int length = 0;
    uint16_t flag;
    uint16_t datalen;
    uint16_t enclen;

    for(;;)
    {
        len = fread(&flag, 2, 1, file);
        if(!len)
            break;
        flag = ntohs(flag);

        len = fread(&datalen, 2, 1, file);
        if(!len)
            break;
        datalen = ntohs(datalen);
        len = fread(buf, datalen, 1, file); 

        length = datalen;
        if(flag){
            if(random() % 2 == 0){
                status = srtp_protect(srtp_ctx, buf, &length);
            }else{
                status = srtp_protect(srtp_ctx_sub, buf, &length);
            }
            
        }else{
            if(random()% 2 == 0){
                status = srtp_protect_rtcp(srtp_ctx, buf, &length);
            }else{
                status = srtp_protect_rtcp(srtp_ctx_sub, buf, &length);
            }  
        }
        
        if (status) {
            printf("protect error:%d, srclen:%d, style:%s\n",
                    status, datalen, flag ? "rtp":"rtcp");
        }else{
            flag = htons(flag);
            fwrite(&flag, 2, 1, out);
            enclen = htons(length);
            fwrite(&enclen, 2, 1, out);
            fwrite(buf, length, 1, out);

            printf("protect succe, srclen:%d, dstlen:%d, style:%s\n",
                    datalen, length, flag ? "rtp":"rtcp");
        }
    }
    fclose(file);
    fclose(out);

    return 0;
}


ngx_int_t
ngx_decode_base64_internal(ngx_str_t *dst, ngx_str_t *src, const u_char *basis)
{
    size_t          len;
    u_char         *d, *s;

    for (len = 0; len < src->len; len++) {
        if (src->data[len] == '=') {
            break;
        }

        if (basis[src->data[len]] == 77) {
            return -1;
        }
    }

    if (len % 4 == 1) {
        return -1;
    }

    s = src->data;
    d = dst->data;

    while (len > 3) {
        *d++ = (u_char) (basis[s[0]] << 2 | basis[s[1]] >> 4);
        *d++ = (u_char) (basis[s[1]] << 4 | basis[s[2]] >> 2);
        *d++ = (u_char) (basis[s[2]] << 6 | basis[s[3]]);

        s += 4;
        len -= 4;
    }

    if (len > 1) {
        *d++ = (u_char) (basis[s[0]] << 2 | basis[s[1]] >> 4);
    }

    if (len > 2) {
        *d++ = (u_char) (basis[s[1]] << 4 | basis[s[2]] >> 2);
    }

    dst->len = d - dst->data;

    return 0;
}


ngx_int_t
ngx_decode_base64(ngx_str_t *dst, ngx_str_t *src)
{
    static u_char   basis64[] = {
        77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77,
        77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77,
        77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 62, 77, 77, 77, 63,
        52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 77, 77, 77, 77, 77, 77,
        77,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14,
        15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 77, 77, 77, 77, 77,
        77, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
        41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 77, 77, 77, 77, 77,

        77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77,
        77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77,
        77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77,
        77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77,
        77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77,
        77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77,
        77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77,
        77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77
    };

    return ngx_decode_base64_internal(dst, src, basis64);
}