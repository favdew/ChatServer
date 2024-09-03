#ifndef CHAT_CLIENT
#define CHAT_CLIENT
#include <cstring>
#include <fstream>
#define __USE_W32_SOCKETS
#define BOOST_ASIO_NO_WIN32_LEAN_AND_MEAN
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include "chat_message.hpp"

#pragma warning(disable: 4996)

using boost::asio::ip::tcp;

const std::size_t max_buf_size = 10240;
const std::size_t header_size = 16;

class chat_client
{
public:
	chat_client(boost::asio::io_context& io_context, tcp::resolver::results_type& endpoints);
	~chat_client();

	bool is_connected();
	void get_read_header_buf(char* dest);
	void get_read_buf(char* dest);
	void get_write_buf(char* dest);

	
protected:
	void do_write_header(const char* msg);
	void do_write(const char* msg, std::size_t length);
	void do_read_header();
	void do_read();
	std::size_t wait_for_read_header();
	std::size_t wait_for_read();
	std::size_t wait_for_read(unsigned long long length);
	void do_close();

	virtual void read_header_result(boost::system::error_code ec, std::size_t length) = 0;
	virtual void read_result(boost::system::error_code ec, std::size_t length) = 0;
	virtual void write_result(boost::system::error_code ec, std::size_t length);
	void write_result2(const boost::system::error_code& ec, std::size_t length);

private:
	void do_connect(tcp::resolver::results_type endpoints);

	boost::asio::io_context& io_context_;
	tcp::socket socket_;
	char read_header_buffer_[header_size + 1], write_header_buffer_[header_size + 1];
	char read_buffer_[max_buf_size], write_buffer_[max_buf_size];
};

#endif