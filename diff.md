## diff
* srtp_ctx and srtp_ctx_sub have the same policy setting
```c
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
```

* one.srtp.context  
``` c
if(flag){
    if(random() % 2 == 0){
        status = srtp_protect(srtp_ctx, buf, &length);
    }else{
        status = srtp_protect(srtp_ctx, buf, &length);
    }
    
}else{
    if(random()% 2 == 0){
        status = srtp_protect_rtcp(srtp_ctx, buf, &length);
    }else{
        status = srtp_protect_rtcp(srtp_ctx, buf, &length);
    }  
}
```

* two.srtp.context
```c
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
```

