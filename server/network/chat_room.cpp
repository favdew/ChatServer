#include "chat_room.h"

typedef std::shared_ptr<chat_participant> participant_ptr;
typedef std::shared_ptr<talk_data> talk_data_ptr;

chat_room::chat_room(long long room_number, std::string room_name,
    std::string owner_id, participant_ptr owner_participant)
    : room_number_(room_number), room_name_(room_name), owner_id_(owner_id)
{
    join(owner_participant);
}

chat_room::~chat_room()
{
    part_set_mutex_.lock();
    for(auto it = participant_set_.begin(); it != participant_set_.end();)
        it = participant_set_.erase(it);
    part_set_mutex_.unlock();

    td_list_.clear();   
}

long long chat_room::room_number()
{
    return room_number_;
}

long long chat_room::room_number() const
{
    return room_number_;
}


std::string chat_room::room_name()
{
    return room_name_;
}

std::string chat_room::room_name() const
{
    return room_name_;
}

void chat_room::set_room_name(const std::string &room_name)
{
    room_name_ = room_name;
}

void chat_room::set_room_name(const char* room_name)
{
    room_name_ = room_name;
}

std::string chat_room::owner_id()
{
    return owner_id_;
}

std::string chat_room::owner_id() const
{
    return owner_id_;
}

void chat_room::set_owner_id(const std::string &owner_id)
{
    owner_id_ = owner_id;
}

void chat_room::set_owner_id(const char* owner_id)
{
    owner_id_ = owner_id;
}

/*ThreadedSet<participant_ptr>& chat_room::get_participants()
{
    return participant_set_;
}*/

std::set<participant_ptr>& chat_room::get_participants()
{
    return participant_set_;
}

int chat_room::join(participant_ptr participant)
{
    part_set_mutex_.lock();
    auto result = participant_set_.emplace(participant);

    if(!result.second) 
    {
        part_set_mutex_.unlock();
        return -1;
    }
    else 
    {
        int ret = (int)(participant_set_.size());
        part_set_mutex_.unlock();
        return ret;
    }
}

void chat_room::delivery(const char* header_data, const char* data)
{
    td_list_.emplace_back(std::make_shared<talk_data>(data));
    part_set_mutex_.lock();
    for(auto it = participant_set_.begin(); it != participant_set_.end(); it++)
    {
        (*it)->delivery(header_data, data);
    }
    part_set_mutex_.unlock();
}

int chat_room::quit(participant_ptr participant)
{
    part_set_mutex_.lock();
    auto finding = participant_set_.find(participant);
    
    if(finding == participant_set_.end())
    {
        part_set_mutex_.unlock();
        return -1;
    }
    
    auto result = participant_set_.erase(finding);
    if(result == participant_set_.end())
    {
        part_set_mutex_.unlock();
        return 0;
    }
    else
    {
        int ret = (int)(participant_set_.size());
        part_set_mutex_.unlock();
        return ret;
    }
}

std::size_t chat_room::get_participants_num()
{
    return participant_set_.size();
}

bool chat_room::operator==(const chat_room& room)
{
    return (room_number_ == room.room_number());
}

bool chat_room::operator>(const chat_room& room)
{
    return (room_number_ > room.room_number());
}

bool chat_room::operator<(const chat_room& room)
{
    return (room_number_ < room.room_number());
}

bool chat_room_compare::operator()(const room_ptr& left, const room_ptr& right) const
{
    return left->room_number() < right->room_number();
}