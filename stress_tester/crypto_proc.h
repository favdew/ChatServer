#ifndef CRYPTO_PROC
#define CRYPTO_PROC

#include <string>

#include <cryptlib.h>
#include <sha3.h>
#include <filters.h>
#include <hex.h>
#include <aes.h>
#include <modes.h>
#include <base64.h>
#include <rsa.h>
#include <osrng.h>

class crypto_proc
{
public:
	crypto_proc();
	crypto_proc(std::string& private_key, std::string& public_key);
	~crypto_proc();

	void SetAESKey(const unsigned char* value);

	void EncryptWithRSA(const std::string& plain, std::string& output);
	void EncryptWithRSA(const unsigned char* plain, unsigned char* output, std::size_t& output_len);
	void DecryptWithRSA(const std::string& encrypt, std::string& output);
	void DecryptWithRSA(const unsigned char* encrypt, std::size_t cipherDataLen, unsigned char* output, std::size_t& output_len);

	void EncryptWithAES(const std::string& plain, std::string& output, const unsigned char* iv);
	void DecryptWithAES(const std::string& encrypt, std::string& output, const unsigned char* iv);

	void SavePublicKey(std::string& buf);
	void LoadPublicKey(std::string& buf);
	void SavePrivateKey(std::string& buf);
	void LoadPrivateKey(std::string& buf);
private:
	CryptoPP::AutoSeededRandomPool rng_;
	CryptoPP::RSA::PrivateKey *rsa_prk_;
	CryptoPP::RSA::PublicKey *rsa_puk_;

	unsigned char aes_key_[CryptoPP::AES::DEFAULT_KEYLENGTH];
};

#endif