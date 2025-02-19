#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/evp.h>
#include <openssl/err.h>
 
void handleErrors() {
    ERR_print_errors_fp(stderr);
    abort();
}
 
int encrypt(unsigned char *plaintext, int plaintext_len, unsigned char *key, unsigned char *iv, unsigned char *ciphertext) {
    EVP_CIPHER_CTX *ctx;
 
    int len;
    int ciphertext_len;
 
    if(!(ctx = EVP_CIPHER_CTX_new())) 
        handleErrors();
 
    if(1 != EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key, iv))
        handleErrors();
 
    int update_len = 0;
    if(1 != EVP_EncryptUpdate(ctx, ciphertext, &update_len, plaintext, plaintext_len))
        handleErrors();
    ciphertext_len = update_len;
 
    int final_len = 0;
    if(1 != EVP_EncryptFinal_ex(ctx, ciphertext + ciphertext_len, &final_len))
        handleErrors();
    ciphertext_len += final_len;
 
    EVP_CIPHER_CTX_free(ctx);
 
    return ciphertext_len;
}
 
int decrypt(unsigned char *ciphertext, int ciphertext_len, unsigned char *key, unsigned char *iv, unsigned char *plaintext) {
    EVP_CIPHER_CTX *ctx;
 
    int len;
    int plaintext_len;
 
    if(!(ctx = EVP_CIPHER_CTX_new())) 
        handleErrors();
 
    if(1 != EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key, iv))
        handleErrors();
 
    int update_len = 0;
    if(1 != EVP_DecryptUpdate(ctx, plaintext, &update_len, ciphertext, ciphertext_len))
        handleErrors();
    plaintext_len = update_len;
 
    int final_len = 0;
    if(1 != EVP_DecryptFinal_ex(ctx, plaintext + plaintext_len, &final_len))
        handleErrors();
    plaintext_len += final_len;
 
    EVP_CIPHER_CTX_free(ctx);
 
    return plaintext_len;
}
 
int main() {
    unsigned char *key = (unsigned char *)"0123456789abcdef0123456789abcdef"; // 256-bit key
    unsigned char *iv = (unsigned char *)"0123456789abcdef"; // 128-bit IV
    unsigned char *plaintext = (unsigned char *)"This is a test";
    int plaintext_len = strlen((const char *)plaintext) + 1;
    unsigned char ciphertext[128];
    unsigned char decryptedtext[128];
 
    // 加密
    int ciphertext_len = encrypt(plaintext, plaintext_len, key, iv, ciphertext);
 
    // 解密
    int decryptedtext_len = decrypt(ciphertext, ciphertext_len, key, iv, decryptedtext);
 
    // 打印结果
    printf("Decrypted text: %s\n", decryptedtext);
 
    return 0;
}
