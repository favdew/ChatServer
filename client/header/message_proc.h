#ifndef MESSAGE_PROC
#define MESSAGE_PROC

#include <fstream>
#include <string>
#include <cstring>
#include <random>
#include <crypto_proc.h>
#include "chat_client.h"
#include "chat_message.hpp"
#include "Win32APIWrapper.h"

class message_proc : public chat_client
{
public:
	message_proc(boost::asio::io_context& io_context, tcp::resolver::results_type& endpoints);
	~message_proc();

	void set_cproc(crypto_proc* cproc);

	void write(const LPCWSTR str);

	char* read_chatting_message();
	int request_create_room(const char* user_id, const char* room_name);
	int request_exit_room(const char* id, const char* roomname);
	int request_join_room(const char* id, long long room_number, LPWSTR room_name_buf);
	int request_login(const char* user_id, const char* user_password);
	int send_talk_message(const talk_data& td);
	int request_create_key(const char *public_key, std::size_t key_size);
	int request_register(const char* id, const char* password, const char* name, const char* email);
	int request_confirm_uinfo(const char* id, const char* name, const char* email);
	int request_change_password(const char* id, const char* password);
protected:
	void read_header_result(boost::system::error_code ec, std::size_t length);
	void read_result(boost::system::error_code ec, std::size_t length);
	void write_result(boost::system::error_code ec, std::size_t length) override;

private:
	crypto_proc* cproc_;

};

#endif