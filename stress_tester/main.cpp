#include <fstream>
#include <iostream>
#include <queue>
#include <thread>
#include <memory>
#include <vector>
#include <mutex>
#include <boost/asio.hpp>
#include <set>
#include "chat_message.hpp"
#include "client.cpp"

using boost::asio::ip::tcp;

int main()
{
	crypto_proc cproc;
	std::string privatekey, publickey;
	cproc.SavePrivateKey(privatekey);
	cproc.SavePublicKey(publickey);

	char connect_ip[16];
	char connect_port[8];

	std::ifstream fs;
	fs.open("connect_conf.conf");
	if (fs.is_open())
	{
		std::string ip, port;

		std::getline(fs, ip);
		std::getline(fs, port);

		std::memset(connect_ip, 0, sizeof(connect_ip));
		std::memset(connect_port, 0, sizeof(connect_port));

		std::strcpy(connect_ip, ip.c_str());
		std::strcpy(connect_port, port.c_str());
	}
	else
	{
		std::cout << "Could not open connect_conf.conf\n";
		exit(1);
	}
	fs.close();
		
	using namespace std::chrono_literals;
	const int thread_num = 100;
	int client_num=1000, message_num=500, participant_num=20, loop_count;
	int result_wcount = 0, result_whcount = 0, result_rcount = 0, looped_count = 0;
	std::mutex wcount_mutex;
	std::queue<std::thread*> q;
	std::mutex qmutex;
	std::queue<double> avg_queue;
	std::mutex avg_mutex;

	std::cout << "client num:"; std::cin >> client_num;
	std::cout << "message_num:"; std::cin >> message_num;
	std::cout << "participants per room:"; std::cin >> participant_num;
	std::cout << "loop_count(infinite loop for zero):"; std::cin >> loop_count;

	while(!loop_count || looped_count < loop_count)
	{
		auto first_time = std::chrono::system_clock::now();

		int create_thread_num;
		create_thread_num = client_num;

		std::cout << "connecting..\n";

		//Reqeust for registering user info
		/// ////////////////////////////////////
		/*boost::asio::io_context ioc;
		tcp::resolver resolver(ioc);
		auto endpoints = resolver.resolve(connect_ip, connect_port);
		auto cli = std::make_shared<client>(ioc, endpoints);
		cli->request_create_key();
		for (int i = 0; i < 1000; i++)
		{
			std::string id = std::to_string(i);
			cli->request_register(id.c_str(), "testpassword", "nnn", "eee");
		}*/
		/// <//////////////////////////////////////////////

		std::vector<int> room_user_count;
		if (client_num % participant_num == 0)
			room_user_count.resize((client_num / participant_num) + 1);
		else
			room_user_count.resize((client_num / participant_num) + 2);
		std::mutex room_user_count_mutex;

		for (int i = 0; i < room_user_count.size(); i++)
			room_user_count[i] = 0;

		for (int i = 0; i < create_thread_num; i++)
		{
			int store_value = i;
			std::thread* t = new std::thread([&]()
				{
					auto term = 0ms;
					int tcount = store_value;
					i = -1;
					std::vector<std::shared_ptr<client>> cli_vec;
					boost::asio::io_context ioc;
					tcp::resolver resolver(ioc);
					auto endpoints = resolver.resolve(connect_ip, connect_port);
					auto cli = std::make_shared<client>(ioc, endpoints, privatekey, publickey);

					cli_vec.push_back(cli);
					cli->set_packet_num(message_num);
					cli->set_tcount(tcount);
					cli->request_create_key();
					cli->request_login(std::to_string(tcount).c_str(), "testpassword");
					if (cli->get_store_id() == "")
					{
						wcount_mutex.lock();
						result_whcount += cli->get_whcount();
						result_wcount += cli->get_wcount();
						result_rcount += cli->get_rcount();
						wcount_mutex.unlock();
						cli_vec.pop_back();
					}

					std::string roomname = std::to_string(tcount) + "aaaaa";
					if (tcount % participant_num == 0)
					{
						if (cli->get_store_id() != "")
						{
							cli->request_create_room(roomname.c_str());
							if (cli->get_room_number() == 0)
							{
								std::cout << "create failed\a\n";
							}
							else
							{
								if (cli->get_room_number() < 1 ||
									cli->get_room_number() >(long long)((int)(client_num / participant_num) + 1))
								{
									std::cout << cli->get_room_number() << " create error\n";
									exit(3);
								}
								room_user_count_mutex.lock();
								room_user_count[cli->get_room_number()]++;
								room_user_count_mutex.unlock();
							}
						}
					}
					else
					{
						if (cli->get_store_id() != "")
						{
							long long join_number = 0;
							join_number += (long long)((int)(tcount / participant_num) + 1);

							while (cli->get_room_number() == 0 && cli->re_send_count_ < 5)
							{
								cli->request_join_room(join_number);

								if (cli->get_room_number() == 0)
								{
									std::this_thread::sleep_for(std::chrono::milliseconds(1000));
									cli->re_send_count_++;
								}
							}
							if (cli->get_room_number() == 0)
							{
								std::cout << "join failed\a\n";
								if (join_number < 1 || join_number > (long long)((int)(client_num / participant_num) + 1))
								{
									std::cout << join_number << " join error\n";
									exit(3);
								}
								room_user_count_mutex.lock();
								room_user_count[join_number]++;
								room_user_count_mutex.unlock();
								cli->close();
								wcount_mutex.lock();
								result_whcount += cli->get_whcount();
								result_wcount += cli->get_wcount();
								result_rcount += cli->get_rcount();
								wcount_mutex.unlock();
								cli_vec.pop_back();
							}
							else
							{
								room_user_count_mutex.lock();
								room_user_count[cli->get_room_number()]++;
								room_user_count_mutex.unlock();
							}
						}
					}

					while (cli->get_room_number() != 0 &&
						room_user_count[cli->get_room_number()] < participant_num)
					{
						std::this_thread::sleep_for(500ms);
					}

					for (int idx = 0; idx < message_num; idx++)
					{
						for (auto it = cli_vec.begin(); it != cli_vec.end(); it++)
						{
							std::string str = "stress messasge";
							(*it)->reqeust_send_message(str.c_str());
							std::this_thread::sleep_for(term);
						}
					}

					for (auto it = cli_vec.begin(); it != cli_vec.end(); it++)
					{
						avg_mutex.lock();
						auto avg_result = (*it)->calc_res_avg();
						if (avg_result != 0.0f)
							avg_queue.push(avg_result);
						avg_mutex.unlock();

						(*it)->reqeust_exit_room();

						wcount_mutex.lock();
						result_whcount += (*it)->get_whcount();
						result_wcount += (*it)->get_wcount();
						result_rcount += (*it)->get_rcount();
						wcount_mutex.unlock();
						(*it)->close();
					}

					cli_vec.clear();
				});

			q.push(t);
			while (i != -1) std::this_thread::sleep_for(1ms);;
			i = store_value;
			std::this_thread::sleep_for(10ms);
		}

		int tcount = 0;
		while (!q.empty())
		{
			std::thread* tp = q.front();
			if (tp != nullptr && tp->joinable())
			{
				tp->join();
				tcount++;
			}

			delete tp;
			q.pop();
		}

		double sum = 0.0f;
		int size = avg_queue.size();
		std::cout << "[avg]\n";
		while (!avg_queue.empty())
		{
			auto value = avg_queue.front();
			sum += value;
			avg_queue.pop();
		}
		std::cout << "\n";
		std::cout << "avg queue size=" << size << std::endl;
		std::cout << "avg response second=" << (double)(sum / size) << std::endl;
		std::cout << "written header pakcet count=" << result_whcount << std::endl;
		std::cout << "written packet count=" << result_wcount << std::endl;
		std::cout << "read packet count=" << result_rcount << std::endl;
		std::chrono::duration<double> final_sec;
		final_sec = std::chrono::system_clock::now() - first_time;
		std::cout << "past time=" << final_sec.count() << "\n";

		std::cout << "looped count=" << ++looped_count << "\n";
		std::this_thread::sleep_for(1000ms);
	}
	system("pause");
	return 0;
}