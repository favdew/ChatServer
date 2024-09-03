#ifndef CHATROOM
#define CHATROOM
#include <string>
#include <list>
#include <memory>
#include <application/ThreadedSet.hpp>
#include <network/chat_message.hpp>
#include <network/chat_participant.h>

class chat_room : public std::enable_shared_from_this<chat_room>
{
typedef std::shared_ptr<chat_participant> participant_ptr;
typedef std::shared_ptr<talk_data> talk_data_ptr;
public:
    chat_room(long long room_number, std::string room_name, std::string owner_id,
        participant_ptr owner_participant);
    ~chat_room();

    long long room_number();
    long long room_number() const;
    std::string room_name();
    std::string room_name() const;
    void set_room_name(const std::string &room_name);
    void set_room_name(const char* room_name);
    std::string owner_id();
    std::string owner_id() const;
    void set_owner_id(const std::string &owner_id);
    void set_owner_id(const char* owner_id);
    //ThreadedSet<participant_ptr>& get_participants();
    std::set<participant_ptr>& get_participants();

    int join(participant_ptr participant);
    void delivery(const char* header_data, const char* data);
    int quit(participant_ptr participant);
    std::size_t get_participants_num();

    bool operator==(const chat_room& room);
    bool operator>(const chat_room& room);
    bool operator<(const chat_room& room);

    std::mutex part_set_mutex_;
private:
    long long room_number_;         //room identifier
    std::string room_name_;
    std::string owner_id_;          //(not accepting other session,
                                    //only allowed kick and invite other sessions)

    //ThreadedSet<participant_ptr> participant_set_;
    std::set<participant_ptr> participant_set_;
    std::list<talk_data_ptr> td_list_;
};

struct chat_room_compare
{
    typedef std::shared_ptr<chat_room> room_ptr;
    bool operator()(const room_ptr& left, const room_ptr& right) const;
};

#endif