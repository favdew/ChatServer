#include "message_proc.h"

message_proc::message_proc(boost::asio::io_context& io_context, tcp::resolver::results_type& endpoints)
	: chat_client(io_context, endpoints), cproc_(nullptr)
{

}

message_proc::~message_proc()
{

}

void message_proc::set_cproc(crypto_proc* cproc)
{
	cproc_ = cproc;
}

void message_proc::write(const LPCWSTR str)
{
	char* temp = new char[lstrlenW(str)];
	TcharToChar(temp, str);
	do_write(temp, lstrlenW(str));
}

char* message_proc::read_chatting_message()
{
	wait_for_read_header();
	char rh_buf[header_size + 1];
	get_read_header_buf(rh_buf);
	chat_message_header header(rh_buf);
	if (!header.is_valid())
	{
		return nullptr;
	}

	wait_for_read();
	char* r_buf = new char[max_buf_size];
	get_read_buf(r_buf);

	return r_buf;
}

int message_proc::request_create_room(const char* user_id, const char* room_name)
{
	cm_room_message message(user_id, room_name);
	chat_message_header header(message.size(),
		chat_message_header::MT_CREATEROOM);
	auto hbuf = header.serialization();
	
	auto buf = message.serialization();
	
	do_write_header(hbuf);
	do_write(buf, (std::size_t)(header.body_size));

	wait_for_read_header();
	char rhbuf[header_size + 1];
	get_read_header_buf(rhbuf);
	chat_message_header res_header(rhbuf);
	if (!res_header.is_valid() || res_header.message_type != chat_message_header::MT_CREATEROOM)
		return 0;
	auto length = wait_for_read(res_header.body_size);
	char rbuf[max_buf_size];
	get_read_buf(rbuf);
	cm_room_message msg(rbuf);

	delete[] hbuf;
	delete[] buf;

	if (length <= 0)
		return 0;

	return length;
}

int message_proc::request_exit_room(const char* id, const char* roomname)
{
	cm_exit_room_message message(id, roomname);
	chat_message_header header(message.size(), chat_message_header::MT_EXITROOM);

	auto hbuf = header.serialization();
	auto buf = message.serialization();

	do_write_header(hbuf);
	do_write(buf, header.body_size);

	delete[] hbuf;
	delete[] buf;

	wait_for_read_header();
	char* rhbuf = new char[header_size + 1];
	get_read_header_buf(rhbuf);
	chat_message_header res_header(rhbuf);
	if (!res_header.is_valid() || res_header.message_type != chat_message_header::MT_EXITROOM)
	{
		delete[] rhbuf;
		return 0;
	}
	wait_for_read(res_header.body_size);

	char* rbuf = new char[max_buf_size];
	get_read_buf(rbuf);
	cm_exit_room_message res_message(rbuf);
	int ret = 0;
	if (!std::strcmp(res_message.id, id) && !std::strcmp(res_message.room_name, roomname))
	{
		ret = res_header.body_size;
	}

	delete[] rhbuf;
	delete[] rbuf;

	return ret;
}

int message_proc::request_join_room(const char* id, long long room_number, LPWSTR room_name_buf)
{
	cm_join_room_message message(id, room_number, "");
	chat_message_header header(message.size(), chat_message_header::MT_JOINROOM);
	auto hbuf = header.serialization();
	auto buf = message.serialization();

	do_write_header(hbuf);
	do_write(buf, header.body_size);

	delete[] hbuf;
	delete[] buf;

	wait_for_read_header();
	char rhbuf[header_size + 1];
	get_read_header_buf(rhbuf);
	chat_message_header res_header(rhbuf);

	if (!res_header.is_valid())
		return 0;

	auto length = wait_for_read(res_header.body_size);
	char* rbuf = new char[max_buf_size];
	get_read_buf(rbuf);
	cm_join_room_message res_message(rbuf);
	delete[] rbuf;

	if (!std::strlen(res_message.id) && !std::strlen(res_message.room_name) &&
		res_message.room_number == 0)
	{
		return 0;
	}
	TCHAR roomname_buf[MAX_ROOM_NAME_LENGTH];
	std::memset(roomname_buf, 0, MAX_ROOM_NAME_LENGTH);
	CharToTchar(roomname_buf, MAX_ROOM_NAME_LENGTH, res_message.room_name, MAX_ROOM_NAME_LENGTH);
	std::memcpy(room_name_buf, roomname_buf, MAX_ROOM_NAME_LENGTH);

	
	return length;
}

int message_proc::request_login(const char* user_id, const char* user_password)
{
	cm_login_message message(user_id, user_password);
	
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution<int> dis(0, 255);
	unsigned char iv[CryptoPP::AES::DEFAULT_BLOCKSIZE];

	for (int i = 0; i < sizeof(iv); i++)
	{
		iv[i] = dis(gen);
	}

	message.set_iv(iv);

	auto buf = message.serialization_except_iv();

	std::string str_buf(buf, message.size() - message.iv_size());
	std::string buf_output;

	cproc_->EncryptWithAES(str_buf, buf_output, message.iv);

	std::size_t final_size = buf_output.size() + message.iv_size();
	char* wbuf = new char[final_size];
	std::memcpy(wbuf, buf_output.c_str(), buf_output.size());
	std::memcpy(wbuf + buf_output.size(), message.iv, message.iv_size());

	chat_message_header header(final_size,
		chat_message_header::MT_LOGIN);

	auto hbuf = header.serialization();

	do_write_header(hbuf);
	do_write(wbuf, header.body_size);

	delete[] hbuf;
	delete[] buf;
	delete[] wbuf;

	wait_for_read_header();
	char res_hbuf[header_size + 1];
	get_read_header_buf(res_hbuf);
	chat_message_header res_header(res_hbuf);

	char res_buf[max_buf_size];
	wait_for_read(res_header.body_size);
	get_read_buf(res_buf);

	unsigned char res_iv[CryptoPP::AES::DEFAULT_BLOCKSIZE];

	std::memcpy(res_iv, res_buf + (res_header.body_size
		- CryptoPP::AES::DEFAULT_BLOCKSIZE),
		CryptoPP::AES::DEFAULT_BLOCKSIZE);

	char* pure_body =
		new char[res_header.body_size - CryptoPP::AES::DEFAULT_BLOCKSIZE];
	std::memcpy(pure_body, res_buf, res_header.body_size - CryptoPP::AES::DEFAULT_BLOCKSIZE);

	std::string enc_body_data(pure_body,
		res_header.body_size - CryptoPP::AES::DEFAULT_BLOCKSIZE);
	std::string body_data;

	cproc_->DecryptWithAES(enc_body_data, body_data, res_iv);

	cm_login_message res_message(body_data.c_str());

	delete[] pure_body;

	if (!std::strcmp(user_id, res_message.id))
		return 0;
	else if (!std::strcmp(res_message.id, CODE_ALREADY_LOGINED))
		return -2;
	else
		return -1;
}

int message_proc::send_talk_message(const talk_data& td)
{
	cm_send_room_message message(td);
	chat_message_header header(message.size(), chat_message_header::MT_SENDMESSAGE);

	auto hbuf = header.serialization();
	auto buf = message.serialization();

	do_write_header(hbuf);
	do_write(buf, header.body_size);

	delete[] hbuf;
	delete[] buf;

	return 0;
}

int message_proc::request_create_key(const char* public_key, std::size_t key_size)
{
	cm_create_key_message message(public_key, key_size);
	chat_message_header header(message.size(), chat_message_header::MT_CREATEKEY);

	auto len = std::strlen(public_key);

	auto hbuf = header.serialization();
	auto buf = message.serialization();

	do_write_header(hbuf);
	do_write(buf, header.body_size);

	delete[] hbuf;
	delete[] buf;

	wait_for_read_header();
	char res_hbuf[header_size + 1];
	get_read_header_buf(res_hbuf);
	chat_message_header res_header(res_hbuf);
	char res_buf[max_buf_size];
	wait_for_read(res_header.body_size);
	get_read_buf(res_buf);
	cm_create_key_message res_message(res_buf);
	
	std::string key_data(res_message.data, res_message.key_length);
	std::string dec_key_data;
	cproc_->DecryptWithRSA(key_data, dec_key_data);
	unsigned char* aes_key = new unsigned char[dec_key_data.size()];
	std::memcpy(aes_key, dec_key_data.c_str(), dec_key_data.size());
	cproc_->SetAESKey(aes_key);

	delete[] aes_key;

	return 0;
}

int message_proc::request_register(const char* id, const char* password, const char* name, const char* email)
{
	cm_register_message message(id, password, name, email);

	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution<int> dis(0, 255);
	unsigned char iv[CryptoPP::AES::DEFAULT_BLOCKSIZE];

	for (int i = 0; i < sizeof(iv); i++)
	{
		iv[i] = dis(gen);
	}

	message.set_iv(iv);

	auto buf = message.serialization_except_iv();

	std::string str_buf(buf, message.size() - message.iv_size());
	std::string buf_output;

	cproc_->EncryptWithAES(str_buf, buf_output, message.iv);

	std::size_t final_size = buf_output.size() + message.iv_size();
	char* wbuf = new char[final_size];
	std::memcpy(wbuf, buf_output.c_str(), buf_output.size());
	std::memcpy(wbuf + buf_output.size(), message.iv, message.iv_size());

	chat_message_header header(final_size,
		chat_message_header::MT_REGISTER);

	auto hbuf = header.serialization();

	do_write_header(hbuf);
	do_write(wbuf, header.body_size);

	delete[] hbuf;
	delete[] buf;
	delete[] wbuf;

	wait_for_read_header();
	char res_hbuf[header_size + 1];
	get_read_header_buf(res_hbuf);
	chat_message_header res_header(res_hbuf);
	char res_buf[max_buf_size];
	wait_for_read(res_header.body_size);
	get_read_buf(res_buf);

	unsigned char res_iv[CryptoPP::AES::DEFAULT_BLOCKSIZE];

	std::memcpy(res_iv, res_buf + (res_header.body_size
		- CryptoPP::AES::DEFAULT_BLOCKSIZE),
		CryptoPP::AES::DEFAULT_BLOCKSIZE);

	char *pure_body =
		new char[res_header.body_size - CryptoPP::AES::DEFAULT_BLOCKSIZE];
	std::memcpy(pure_body, res_buf, res_header.body_size - CryptoPP::AES::DEFAULT_BLOCKSIZE);

	std::string enc_body_data(pure_body,
		res_header.body_size - CryptoPP::AES::DEFAULT_BLOCKSIZE);
	std::string body_data;

	cproc_->DecryptWithAES(enc_body_data, body_data, res_iv);

	cm_register_message res_message(body_data.c_str());

	delete[] pure_body;

	if (!std::strcmp(message.id, res_message.id) && !std::strcmp(message.name, res_message.name)
		&& !std::strcmp(message.email, res_message.email))
	{
		return 0;
	}
	else
	{
		return -1;
	}
}

int message_proc::request_confirm_uinfo(const char* id, const char* name, const char* email)
{
	cm_confirm_uinfo_message message(id, name, email);

	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution<int> dis(0, 255);
	unsigned char iv[CryptoPP::AES::DEFAULT_BLOCKSIZE];

	for (int i = 0; i < sizeof(iv); i++)
	{
		iv[i] = dis(gen);
	}

	message.set_iv(iv);

	auto buf = message.serialization_except_iv();

	std::string str_buf(buf, message.size() - message.iv_size());
	std::string buf_output;

	cproc_->EncryptWithAES(str_buf, buf_output, message.iv);

	std::size_t final_size = buf_output.size() + message.iv_size();
	char* wbuf = new char[final_size];
	std::memcpy(wbuf, buf_output.c_str(), buf_output.size());
	std::memcpy(wbuf + buf_output.size(), message.iv, message.iv_size());

	chat_message_header header(final_size,
		chat_message_header::MT_CONFIRMUINFO);

	auto hbuf = header.serialization();

	do_write_header(hbuf);
	do_write(wbuf, header.body_size);

	delete[] hbuf;
	delete[] buf;
	delete[] wbuf;

	wait_for_read_header();
	char res_hbuf[header_size + 1];
	get_read_header_buf(res_hbuf);
	chat_message_header res_header(res_hbuf);
	char res_buf[max_buf_size];
	wait_for_read(res_header.body_size);
	get_read_buf(res_buf);

	unsigned char res_iv[CryptoPP::AES::DEFAULT_BLOCKSIZE];

	std::memcpy(res_iv, res_buf + (res_header.body_size
		- CryptoPP::AES::DEFAULT_BLOCKSIZE),
		CryptoPP::AES::DEFAULT_BLOCKSIZE);

	std::memcpy(res_iv, res_buf + (res_header.body_size
		- CryptoPP::AES::DEFAULT_BLOCKSIZE),
		CryptoPP::AES::DEFAULT_BLOCKSIZE);

	char* pure_body =
		new char[res_header.body_size - CryptoPP::AES::DEFAULT_BLOCKSIZE];
	std::memcpy(pure_body, res_buf, res_header.body_size - CryptoPP::AES::DEFAULT_BLOCKSIZE);

	std::string enc_body_data(pure_body,
		res_header.body_size - CryptoPP::AES::DEFAULT_BLOCKSIZE);
	std::string body_data;

	cproc_->DecryptWithAES(enc_body_data, body_data, res_iv);

	cm_confirm_uinfo_message res_message(body_data.c_str());

	delete[] pure_body;

	if (!std::strcmp(message.id, res_message.id) && !std::strcmp(message.name, res_message.name)
		&& !std::strcmp(message.email, res_message.email))
	{
		return 0;
	}
	else
	{
		return -1;
	}
}

int message_proc::request_change_password(const char* id, const char* password)
{
	cm_change_password_message message(id, password);

	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution<int> dis(0, 255);
	unsigned char iv[CryptoPP::AES::DEFAULT_BLOCKSIZE];

	for (int i = 0; i < sizeof(iv); i++)
	{
		iv[i] = dis(gen);
	}

	message.set_iv(iv);

	auto buf = message.serialization_except_iv();

	std::string str_buf(buf, message.size() - message.iv_size());
	std::string buf_output;

	cproc_->EncryptWithAES(str_buf, buf_output, message.iv);

	std::size_t final_size = buf_output.size() + message.iv_size();
	char* wbuf = new char[final_size];
	std::memcpy(wbuf, buf_output.c_str(), buf_output.size());
	std::memcpy(wbuf + buf_output.size(), message.iv, message.iv_size());

	chat_message_header header(final_size,
		chat_message_header::MT_CHANGEPASSWORD);

	auto hbuf = header.serialization();

	do_write_header(hbuf);
	do_write(wbuf, header.body_size);

	delete[] hbuf;
	delete[] buf;
	delete[] wbuf;

	wait_for_read_header();
	char res_hbuf[header_size + 1];
	get_read_header_buf(res_hbuf);
	chat_message_header res_header(res_hbuf);
	char res_buf[max_buf_size];
	wait_for_read(res_header.body_size);
	get_read_buf(res_buf);

	unsigned char res_iv[CryptoPP::AES::DEFAULT_BLOCKSIZE];

	std::memcpy(res_iv, res_buf + (res_header.body_size
		- CryptoPP::AES::DEFAULT_BLOCKSIZE),
		CryptoPP::AES::DEFAULT_BLOCKSIZE);

	std::memcpy(res_iv, res_buf + (res_header.body_size
		- CryptoPP::AES::DEFAULT_BLOCKSIZE),
		CryptoPP::AES::DEFAULT_BLOCKSIZE);

	char* pure_body =
		new char[res_header.body_size - CryptoPP::AES::DEFAULT_BLOCKSIZE];
	std::memcpy(pure_body, res_buf, res_header.body_size - CryptoPP::AES::DEFAULT_BLOCKSIZE);

	std::string enc_body_data(pure_body,
		res_header.body_size - CryptoPP::AES::DEFAULT_BLOCKSIZE);
	std::string body_data;

	cproc_->DecryptWithAES(enc_body_data, body_data, res_iv);

	cm_change_password_message res_message(body_data.c_str());

	delete[] pure_body;

	if (!std::strcmp(message.id, res_message.id))
	{
		return 0;
	}
	else
	{
		return -1;
	}
}

void message_proc::read_header_result(boost::system::error_code ec, std::size_t length)
{

}

void message_proc::read_result(boost::system::error_code ec, std::size_t length)
{

}
void message_proc::write_result( boost::system::error_code ec, std::size_t length)
{
	if (!ec)
	{
		std::ofstream log_file;
		log_file.open("write_log.log");

		std::string log_value = std::to_string(length) + 
			" bytes transferred, content: ";
		char write_buffer[max_buf_size];
		get_write_buf(write_buffer);
		log_value.append(write_buffer);
		log_file.write(log_value.c_str(), max_buf_size);

		log_file.close();
	}

	else
	{
		std::ofstream log_file;
		log_file.open("write_error_log.log");
		log_file.close();
	}
}