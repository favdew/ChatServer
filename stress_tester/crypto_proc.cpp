#include "crypto_proc.h"

crypto_proc::crypto_proc()
{
	std::memset(aes_key_, 0, sizeof(aes_key_));

	CryptoPP::InvertibleRSAFunction params;
	params.GenerateRandomWithKeySize(rng_, 3072);

	rsa_prk_ = new CryptoPP::RSA::PrivateKey(params);
	rsa_puk_ = new CryptoPP::RSA::PublicKey(params);
}

crypto_proc::crypto_proc(std::string& private_key, std::string& public_key)
{
	rsa_prk_ = new CryptoPP::RSA::PrivateKey();
	rsa_puk_ = new CryptoPP::RSA::PublicKey();

	LoadPrivateKey(private_key);
	LoadPublicKey(public_key);
}

crypto_proc::~crypto_proc()
{
	delete rsa_prk_;
	delete rsa_puk_;
	std::memset(aes_key_, 0, sizeof(aes_key_));
}

void crypto_proc::SetAESKey(const unsigned char* value)
{
	std::memcpy(aes_key_, value, sizeof(aes_key_));
}

void crypto_proc::EncryptWithRSA(const std::string& plain, std::string& output)
{
	CryptoPP::RSAES_OAEP_SHA_Encryptor encryptor(*rsa_puk_);

	CryptoPP::StringSource ss(plain, true,
		new CryptoPP::PK_EncryptorFilter(rng_, encryptor,
			new CryptoPP::StringSink(output)
		)
	);
}

void crypto_proc::EncryptWithRSA(const unsigned char* plain, unsigned char* output, std::size_t& output_len)
{
	CryptoPP::RSAES_OAEP_SHA_Encryptor encryptor(*rsa_puk_);

	size_t plainDataLen = CryptoPP::AES::DEFAULT_KEYLENGTH;
	size_t putLen = 0;
	size_t fixedLen = encryptor.FixedMaxPlaintextLength();
	for (size_t i = 0; i < plainDataLen; i += fixedLen)
	{
		size_t len = fixedLen < (plainDataLen - i) ? fixedLen : (plainDataLen - i);
		CryptoPP::ArraySink* dstArr =
			new CryptoPP::ArraySink(output + putLen, output_len - putLen);
		CryptoPP::ArraySource source(plain + i, len, true,
			new CryptoPP::PK_EncryptorFilter(rng_, encryptor, dstArr));
		putLen += dstArr->TotalPutLength();
	}
	output_len = putLen;
}

void crypto_proc::DecryptWithRSA(const std::string& encrypt, std::string& output)
{
	CryptoPP::RSAES_OAEP_SHA_Decryptor decryptor(*rsa_prk_);

	CryptoPP::StringSource ss(encrypt, true,
		new CryptoPP::PK_DecryptorFilter(rng_, decryptor,
			new CryptoPP::StringSink(output)
		)
	);
}

void crypto_proc::DecryptWithRSA(const unsigned char* encrypt, std::size_t cipherDataLen, unsigned char* output, std::size_t& output_len)
{
	CryptoPP::RSAES_OAEP_SHA_Decryptor decryptor(*rsa_prk_);

	size_t putLen = 0;
	size_t fixedLen = decryptor.FixedCiphertextLength();
	for (size_t i = 0; i < cipherDataLen; i += fixedLen)
	{
		size_t len = fixedLen < (cipherDataLen - i) ? fixedLen : (cipherDataLen - i);
		CryptoPP::ArraySink* dstArr =
			new CryptoPP::ArraySink(output + putLen, output_len - putLen);
		CryptoPP::ArraySource source(encrypt + i, len, true,
			new CryptoPP::PK_DecryptorFilter(rng_, decryptor, dstArr));
		putLen += dstArr->TotalPutLength();
	}
	output_len = putLen;
}

void crypto_proc::EncryptWithAES(const std::string& plain, std::string& output, const unsigned char* iv)
{
	CryptoPP::CBC_Mode<CryptoPP::AES>::Encryption encryptor(aes_key_,
		CryptoPP::AES::DEFAULT_KEYLENGTH, iv);

	CryptoPP::StringSource ss(plain, true,
		new CryptoPP::StreamTransformationFilter(encryptor,
			new CryptoPP::Base64Encoder(
				new CryptoPP::StringSink(output), false),
					CryptoPP::BlockPaddingSchemeDef::ZEROS_PADDING));
}

void crypto_proc::DecryptWithAES(const std::string& encrypt, std::string& output, const unsigned char* iv)
{
	CryptoPP::CBC_Mode<CryptoPP::AES>::Decryption decryptor(aes_key_,
		CryptoPP::AES::DEFAULT_KEYLENGTH, iv);

	CryptoPP::StringSource ss(encrypt, true,
		new CryptoPP::Base64Decoder(
			new CryptoPP::StreamTransformationFilter(decryptor,
				new CryptoPP::StringSink(output),
				CryptoPP::BlockPaddingSchemeDef::ZEROS_PADDING)));
}

void crypto_proc::SavePublicKey(std::string& buf)
{
	CryptoPP::StringSink as(buf);
	rsa_puk_->Save(as);
}

void crypto_proc::LoadPublicKey(std::string& buf)
{
	CryptoPP::StringSink as(buf);
	rsa_puk_->Load(CryptoPP::StringStore(
		(const CryptoPP::byte*)buf.data(), buf.size()).Ref());
}

void crypto_proc::SavePrivateKey(std::string& buf)
{
	CryptoPP::StringSink as(buf);
	rsa_prk_->Save(as);
}

void crypto_proc::LoadPrivateKey(std::string& buf)
{
	CryptoPP::StringSink as(buf);
	rsa_prk_->Load(CryptoPP::StringStore(
		(const CryptoPP::byte*)buf.data(), buf.size()).Ref());
}