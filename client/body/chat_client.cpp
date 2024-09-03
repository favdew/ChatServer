#include "chat_client.h"

chat_client::chat_client(boost::asio::io_context& io_context,
	tcp::resolver::results_type& endpoints)
: io_context_(io_context), socket_(io_context)
{
	do_connect(endpoints);
}

chat_client::~chat_client()
{
	do_close();
}

bool chat_client::is_connected()
{
	return socket_.is_open();
}

void chat_client::get_read_header_buf(char* dest)
{
	std::memset(dest, 0, header_size + 1);
	std::memcpy(dest, read_header_buffer_, header_size + 1);
}

void chat_client::get_read_buf(char* dest)
{
	std::memset(dest, 0, header_size + 1);
	std::memcpy(dest, read_buffer_, max_buf_size);
}

void chat_client::get_write_buf(char* dest)
{
	std::memset(dest, 0, max_buf_size);
	std::memcpy(dest, write_buffer_, max_buf_size);
}

void chat_client::do_write_header(const char* msg)
{
	std::memset(write_header_buffer_, 0, header_size + 1);
	std::memcpy(write_header_buffer_, msg, header_size);
	socket_.wait(tcp::socket::wait_write);
	boost::system::error_code ec;
	socket_.write_some(boost::asio::buffer(write_header_buffer_, header_size), ec);
	if (ec)
	{
		do_close();
	}
}

void chat_client::do_write(const char* msg, std::size_t length)
{
	auto len = std::min(max_buf_size, length);
	std::memset(write_buffer_, 0, max_buf_size);
	write_buffer_[max_buf_size-1] = '\0';
	std::memcpy(write_buffer_, msg, len);
	
	boost::system::error_code ec;
	socket_.wait(tcp::socket::wait_write);
	socket_.write_some(boost::asio::buffer(write_buffer_, len), ec);

	if (ec)
	{
		do_close();
	}

	socket_.wait(tcp::socket::wait_write);
}

void chat_client::do_read_header()
{
	std::memset(read_header_buffer_, 0, header_size + 1);
	boost::asio::async_read(socket_,
		boost::asio::buffer(read_header_buffer_, header_size),
		boost::bind(&chat_client::read_header_result, this,
			boost::asio::placeholders::error,
			boost::asio::placeholders::bytes_transferred)
	);
}

void chat_client::do_read()
{
	std::memset(read_buffer_, 0, max_buf_size);
	boost::asio::async_read(socket_,
		boost::asio::buffer(read_buffer_, max_buf_size),
		boost::bind(&chat_client::read_result, this,
			boost::asio::placeholders::error,
			boost::asio::placeholders::bytes_transferred)
	);
}

std::size_t chat_client::wait_for_read_header()
{
	std::memset(read_header_buffer_, 0, header_size + 1);
	boost::system::error_code ec;
	std::size_t length = 
		socket_.read_some(boost::asio::buffer(read_header_buffer_, header_size), ec);
	if (ec)
	{
		do_close();
		return 0;
	}
	return length;
}

std::size_t chat_client::wait_for_read()
{
	std::memset(read_buffer_, 0, max_buf_size);
	boost::system::error_code ec;
	std::size_t length = socket_.read_some(boost::asio::buffer(read_buffer_, max_buf_size), ec);
	if (ec)
	{
		do_close();
		return 0;
	}
	return length;
}

std::size_t chat_client::wait_for_read(unsigned long long len)
{
	std::memset(read_buffer_, 0, max_buf_size);
	boost::system::error_code ec;
	std::size_t length = socket_.read_some(boost::asio::buffer(read_buffer_, len), ec);
	if (ec)
	{
		do_close();
		return 0;
	}
	return length;
}

void chat_client::do_close()
{
	boost::asio::post(io_context_,
		[this]()
		{
			socket_.close();
		});
}

void chat_client::write_result(boost::system::error_code ec, std::size_t length)
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

void chat_client::write_result2(const boost::system::error_code& ec, std::size_t length)
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

void chat_client::do_connect(tcp::resolver::results_type endpoints)
{
	socket_.connect(*endpoints);
}