#include <memory>
#include <chrono>
#include <boost/asio.hpp>
#include <thread>
#include <mutex>
#include <queue>
#include <random>
#include "crypto_proc.h"
#include "chat_message.hpp"

using boost::asio::ip::tcp;
using namespace std::chrono_literals;

const int max_buf_size = 10240 - 16;
const int header_size = 16;

class client : public std::enable_shared_from_this<client>
{
public:
	client(boost::asio::io_context& io_context, tcp::resolver::results_type& endpoints)
		: io_context_(io_context), socket_(io_context_), ep_(endpoints)
	{
		int option;

		option = 1;

		if (setsockopt(socket_.native_handle(), SOL_SOCKET, SO_REUSEADDR, (const char*)&option, sizeof(option)) < 0)
		{
			std::cout << "client Constructor: faild to setsockopt\n";
			exit(2);
		}
		do_connect(endpoints);
	}

	client(boost::asio::io_context& io_context, tcp::resolver::results_type& endpoints,
		std::string& private_key, std::string& public_key)
		: io_context_(io_context), socket_(io_context_), ep_(endpoints),
		cproc_(private_key, public_key)
	{
		do_connect(endpoints);
	}

	~client()
	{
		if (socket_.is_open()) {
			do_close();
		}
	}

	void set_packet_num(int num)
	{
		first_packet_count_ = num;
		packet_count_ = num;
		read_packet_count_ = num * 2;
	}
	int get_packet_num() { return packet_count_; }
	int get_read_packet_num() { return packet_count_; }
	int get_tcount() { return tcount_; }
	int get_wcount() { return w_count_; }
	int get_whcount() { return wh_count_; }
	int get_rcount() { return r_count_; }

	void set_tcount(int tcount)
	{
		tcount_ = tcount;
	}

	long long get_room_number()
	{
		return room_number_;
	}

	std::string get_store_id()
	{
		return id_;
	}

	double calc_res_avg()
	{
		double sum = 0.0f;
		std::queue<double> temp_queue = res_sec_queue_;
		int len = temp_queue.size();
		if (len <= 0) return 0.0f;
		while (!temp_queue.empty())
		{
			sum += temp_queue.front();
			temp_queue.pop();
		}

		return (sum / len);
	}

	void request_create_key()
	{
		std::string public_key;
		cproc_.SavePublicKey(public_key);
		cm_create_key_message message(public_key.c_str(), public_key.size());
		chat_message_header header(message.size(), chat_message_header::MT_CREATEKEY);

		auto len = public_key.size();

		auto hbuf = header.serialization();
		auto buf = message.serialization();

		write_header(hbuf);
		write(buf, header.body_size);

		delete[] hbuf;
		delete[] buf;

		read_header();
	}

	void request_login(const char* id, const char* password)
	{
		cm_login_message message(id, password);

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

		cproc_.EncryptWithAES(str_buf, buf_output, message.iv);

		std::size_t final_size = buf_output.size() + message.iv_size();
		char* wbuf = new char[final_size];
		std::memcpy(wbuf, buf_output.c_str(), buf_output.size());
		std::memcpy(wbuf + buf_output.size(), message.iv, message.iv_size());

		chat_message_header header(final_size,
			chat_message_header::MT_LOGIN);

		auto hbuf = header.serialization();

		write_header(hbuf);
		write(wbuf, header.body_size);

		delete[] hbuf;
		delete[] buf;
		delete[] wbuf;

		read_header();
	}

	void request_create_room(const char* room_name)
	{
		cm_room_message message(id_.c_str(), room_name);
		chat_message_header header(message.size(), chat_message_header::MT_CREATEROOM);
		auto hbuf = header.serialization();
		auto buf = message.serialization();

		write_header(hbuf);
		write(buf, header.body_size);

		front_header_.message_type = 0;
		while (front_header_.message_type != chat_message_header::MT_CREATEROOM)
			read_header();

		if (room_number_ != -1)
		{
			front_send_id_ = "";
			while (front_send_id_ != id_)
				read_header();

			front_send_id_ = "";
			while (front_send_id_ != id_)
				read_header();
		}
		else
		{
			tp_queue_mutex_.lock();
			tp_queue_.pop();
			tp_queue_.pop();
			tp_queue_mutex_.unlock();
		}

		delete[] hbuf;
		delete[] buf;
	}

	void request_join_room(long long room_number)
	{
		request_room_number_ = room_number;
		cm_join_room_message message(id_.c_str(), room_number, "");
		chat_message_header header(message.size(), chat_message_header::MT_JOINROOM);
		auto hbuf = header.serialization();
		auto buf = message.serialization();

		write_header(hbuf);
		write(buf, header.body_size);

		front_header_.message_type = 0;
		while(front_header_.message_type != chat_message_header::MT_JOINROOM)
			read_header();

		if (room_number_ != 0 && room_name_ != "")
		{
			front_send_id_ = "";
			while (front_send_id_ != id_)
				read_header();
		}
		else
		{
			tp_queue_mutex_.lock();
			tp_queue_.pop();
			tp_queue_mutex_.unlock();
		}

		delete[] hbuf;
		delete[] buf;
	}

	void reqeust_exit_room()
	{
		cm_exit_room_message message(id_.c_str(), room_name_.c_str());
		chat_message_header header(message.size(), chat_message_header::MT_EXITROOM);
		auto hbuf = header.serialization();
		auto buf = message.serialization();

		write_header(hbuf);
		write(buf, header.body_size);

		front_header_.message_type = 0;
		while(front_header_.message_type != chat_message_header::MT_EXITROOM)
			read_header();
	}

	void reqeust_send_message(const char* content)
	{
		talk_data td(id_, nickname_, talk_data::TDF_MESSAGE, content);
		cm_send_room_message message(td);
		chat_message_header header(message.size(), chat_message_header::MT_SENDMESSAGE);
		auto hbuf = header.serialization();
		auto buf = message.serialization();

		write_header(hbuf);
		write(buf, header.body_size);

		do
		{
			front_send_id_ = "";
			read_header();
		} while (front_send_id_ != id_);

		delete[] hbuf;
		delete[] buf;
	}

	void request_register(const char *id, const char *password, const char *name, const char *email)
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

		cproc_.EncryptWithAES(str_buf, buf_output, message.iv);

		std::size_t final_size = buf_output.size() + message.iv_size();
		char* wbuf = new char[final_size];
		std::memcpy(wbuf, buf_output.c_str(), buf_output.size());
		std::memcpy(wbuf + buf_output.size(), message.iv, message.iv_size());

		chat_message_header header(final_size,
			chat_message_header::MT_REGISTER);

		auto hbuf = header.serialization();

		write_header(hbuf);
		write(wbuf, header.body_size);

		delete[] hbuf;
		delete[] buf;
		delete[] wbuf;

		read_header();
	}

	void close()
	{
		if (socket_.is_open())
		{
			socket_.close();
		}
	}
private:
	void do_close()
	{
		boost::system::error_code ec;

		if (ec)
		{
			std::cout << "local endpoint error\n";
			std::cout << ec.message() << "\n";
		}
		boost::system::error_code ec2;
		socket_.close(ec2);
		if (!ec2)
		{
			;
		}
		else
		{
			std::cout << "socket close error\n";
			std::cout << ec2.message() << "\n";
		}
	}

	void write_header(const char* msg)
	{
		if (!socket_.is_open()) return;

		std::memset(write_header_buffer_, 0, header_size + 1);
		std::memcpy(write_header_buffer_, msg, header_size);

		boost::system::error_code ec;

		socket_.write_some(boost::asio::buffer(write_header_buffer_, header_size), ec);
		if (ec)
		{
			do_close();
			std::cout << "write header error:\a" << ec.message() << "\n";
			exit(1);
		}
		else
		{
			wh_count_++;
			chat_message_header header(write_header_buffer_);
			front_write_header_ = header;
		}
	}

	void write(const char* msg, std::size_t length)
	{
		if (!socket_.is_open()) return;
		std::memset(write_buffer_, 0, length);
		write_buffer_[max_buf_size - 1] = '\0';
		std::memcpy(write_buffer_, msg, length);

		boost::system::error_code ec;
		socket_.write_some(boost::asio::buffer(write_buffer_, length), ec);

		if (ec)
		{
			do_close();
			std::cout << "write error:\a" << ec.message() << "\n";
			exit(1);
		}
		else
		{
			w_count_++;
			tp_queue_mutex_.lock();
			tp_queue_.push(std::chrono::system_clock::now());
			tp_queue_mutex_.unlock();

			if (front_write_header_.message_type == chat_message_header::MT_CREATEROOM)
			{
				tp_queue_mutex_.lock();
				tp_queue_.push(std::chrono::system_clock::now());
				tp_queue_.push(std::chrono::system_clock::now());
				tp_queue_mutex_.unlock();
			}
			else if (front_write_header_.message_type == chat_message_header::MT_LOGIN)
			{
				;
			}
			else if (front_write_header_.message_type == chat_message_header::MT_SENDMESSAGE)
			{
				packet_count_--;
			}
			else if (front_write_header_.message_type == chat_message_header::MT_JOINROOM)
			{
				tp_queue_mutex_.lock();
				tp_queue_.push(std::chrono::system_clock::now());
				tp_queue_mutex_.unlock();
			}
			else if (front_write_header_.message_type == chat_message_header::MT_EXITROOM)
			{
				;
			}
		}
	}

	void read_header()
	{
		if (!socket_.is_open()) return;
		std::memset(read_header_buffer_, 0, header_size + 1);
		boost::system::error_code ec;

		auto result_len
			= socket_.read_some(boost::asio::buffer(read_header_buffer_, header_size), ec);
		while (result_len < header_size)
		{
			boost::system::error_code sub_ec;
			auto received
				= socket_.read_some(
					boost::asio::buffer(read_header_buffer_ + result_len,
						header_size - result_len), sub_ec);
			if (!ec)
			{
				result_len += received;
			}
			else
			{
				std::cout << socket_.local_endpoint().port()
					<< " read header error:" << ec.message() << "\a\n";
				do_close();
				exit(1);
				return;
			}
		}
		if (!ec)
		{
			rh_count_++;
			chat_message_header header(read_header_buffer_);

			if (header.message_type == 0)
			{
				boost::system::error_code ec, ec2;
				auto port = socket_.local_endpoint(ec).port();
				auto port2 = socket_.remote_endpoint(ec2).port();
				if (!ec && !ec2)
				{
					std::cout << "port=" << port << ", remote=" << port2
						<< ", message_type=" << header.message_type
						<< ", body_size=" << header.body_size
						<< ", rh_conut=" << rh_count_
						<< ", r_count=" << r_count_ << "\n";
					front_header_ = header;

					exit(1);
					return;
				}
				else
				{
					std::cout << "error:" << ec.message() << "\n";
					std::cout << "error:" << ec2.message() << "\n";
					exit(1);
				}
			}
			else
			{
				front_header_ = header;
				read(header.body_size);
			}
		}
		else
		{
			std::cout << "[" << socket_.local_endpoint().port() << "] [" <<
				front_write_header_.message_type << "]read header error\a\n";
			if (ec == boost::asio::error::connection_reset)
			{
				do_close();
			}
			else if (ec == boost::asio::error::timed_out)
			{
				do_close();
				std::cout << "#############################timed out sockettt close pn="
					<< packet_count_ << ", rpn=" << read_packet_count_ << "\n";
			}
			else
			{
				std::cout << ec << " " << ec.message() << "\a\n";
				do_close();
			}
			exit(1);
		}

	}

	void read(unsigned long long body_size)
	{
		if (!socket_.is_open()) return;
		std::memset(read_buffer_, 0, max_buf_size);
		boost::system::error_code ec;
		
		auto result_len
			= socket_.read_some(boost::asio::buffer(read_buffer_, body_size), ec);
		if (!ec)
		{
			r_count_++;
			queue_delete_flag = true;
			while (result_len < body_size)
			{
				boost::system::error_code sub_ec;
				auto received
					= socket_.read_some(
						boost::asio::buffer(read_buffer_ + result_len, body_size - result_len), sub_ec);
				if (!ec)
				{
					result_len += received;
				}
				else
				{
					std::cout << "read error\n";
					do_close();
					return;
				}
			}
			if (tp_queue_.size() <= 0)
			{
				std::cout << id_ << " size=" << tp_queue_.size() << ", mtype="
					<< front_header_.message_type << "\n";
				exit(1);
			}
			switch (front_header_.message_type)
			{
			case chat_message_header::MT_SENDMESSAGE:
			{
				cm_send_room_message message(read_buffer_);
				front_send_id_ = message.td.id();
				
				if (message.td.id() != id_)
				{
					queue_delete_flag = false;
					r_count_--;
				}

				if (message.td.flag() == talk_data::TDF_MESSAGE)
				{
					if (message.td.id() == id_)
						read_packet_count_--;
					else
						queue_delete_flag = false;
				}
				else if (message.td.flag() == talk_data::TDF_CREATE)
				{
					;
				}
				else if (message.td.flag() == talk_data::TDF_JOIN)
				{
					;
				}
				break;
			}
			case chat_message_header::MT_CREATEROOM:
			{
				cm_room_message message(read_buffer_);
				room_number_ = message.room_number;
				room_name_ = message.room_name;

				if (room_number_ == 0) do_close();
				if (message.room_name == "") return;
				break;
			}
			case chat_message_header::MT_JOINROOM:
			{
				cm_join_room_message message(read_buffer_);
				room_number_ = message.room_number;
				room_name_ = message.room_name;

				if (message.room_name == "") return;
				break;
			}
			case chat_message_header::MT_CREATEKEY:
			{
				cm_create_key_message message(read_buffer_);

				std::string key_data(message.data, message.key_length);
				std::string dec_key_data;
				cproc_.DecryptWithRSA(key_data, dec_key_data);
				unsigned char* aes_key = new unsigned char[dec_key_data.size()];
				std::memcpy(aes_key, dec_key_data.c_str(), dec_key_data.size());
				cproc_.SetAESKey(aes_key);

				delete[] aes_key;
				break;
			}
			case chat_message_header::MT_LOGIN:
			{
				unsigned char res_iv[CryptoPP::AES::DEFAULT_BLOCKSIZE];

				std::memcpy(res_iv, read_buffer_ + (front_header_.body_size
					- CryptoPP::AES::DEFAULT_BLOCKSIZE),
					CryptoPP::AES::DEFAULT_BLOCKSIZE);

				char* pure_body =
					new char[front_header_.body_size - CryptoPP::AES::DEFAULT_BLOCKSIZE];
				std::memcpy(pure_body, read_buffer_, front_header_.body_size - CryptoPP::AES::DEFAULT_BLOCKSIZE);

				std::string enc_body_data(pure_body,
					front_header_.body_size - CryptoPP::AES::DEFAULT_BLOCKSIZE);
				std::string body_data;

				cproc_.DecryptWithAES(enc_body_data, body_data, res_iv);
				cm_login_message res_message(body_data.c_str());

				delete[] pure_body;
				if (res_message.id != "") id_ = res_message.id;
				break;
			}
			case chat_message_header::MT_EXITROOM:
			{
				break;
			}
			case chat_message_header::MT_REGISTER:
			{
				break;
			}
			default:
			{
				std::cout << "invalid read body\n";
				std::cout << "front header body_size=" << front_header_.body_size
					<< ", message_type=" << front_header_.message_type << "\n";
				break;
			}
			}


			if (queue_delete_flag == true)
			{
				std::chrono::duration<double> sec;
				std::size_t qsize;
				tp_queue_mutex_.lock();
				sec = std::chrono::system_clock::now() - tp_queue_.front();
				tp_queue_.pop();
				qsize = tp_queue_.size();
				tp_queue_mutex_.unlock();
				res_sec_queue_.push(sec.count());
			}

		}
		else
		{
			do_close();
			std::cout << "read error:\a" << ec.message() << "\n";
			exit(1);
		}
	}

	void do_connect(tcp::resolver::results_type& endpoints)
	{
		boost::system::error_code ec;
		do
		{
			socket_.connect(*endpoints, ec);
			if (ec)
			{
				std::cout << "[" << socket_.local_endpoint().port() << "] connect error:" << ec.message() << "\a\n";
				exit(1);
			}
			else
			{
				stored_ip = socket_.remote_endpoint().address().to_string();
				stored_port = socket_.remote_endpoint().port();

				int option;

				option = 1;
				int result;

				if ((result = setsockopt(socket_.native_handle(), SOL_SOCKET, SO_LINGER, (const char*)&option, sizeof(option))) < 0)
				{
					std::cout << result << "\n";
					exit(2);
				}
			}
		} while (ec);
	}




public:
	int re_send_count_ = 0;

	std::string stored_ip;
	unsigned short stored_port;

private:
	boost::asio::io_context& io_context_;
	tcp::socket socket_;
	boost::asio::ip::basic_resolver_results<boost::asio::ip::tcp>& ep_;

	char read_header_buffer_[header_size + 1];
	char read_buffer_[max_buf_size];

	char write_header_buffer_[header_size + 1];
	char write_buffer_[max_buf_size];

	chat_message_header front_header_;
	chat_message_header front_write_header_;

	std::queue<std::chrono::system_clock::time_point> tp_queue_;
	std::mutex tp_queue_mutex_;
	std::queue<double> res_sec_queue_;

	std::string id_ = "";
	std::string nickname_;
	std::string room_name_;
	std::string front_send_id_ = "";

	int first_packet_count_ = 0;
	int packet_count_ = 0;
	int read_packet_count_ = 0;
	int tcount_ = -1;
	long long room_number_ = 0;
	bool queue_delete_flag = true;

	int rh_count_ = 0;
	int r_count_ = 0;
	int wh_count_ = 0;
	int w_count_ = 0;
	long long request_room_number_ = 0;

	crypto_proc cproc_;
};