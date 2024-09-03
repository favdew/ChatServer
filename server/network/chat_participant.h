#ifndef CHAT_PARTICIPANT
#define CHAT_PARTICIPANT

#include <string>

class chat_participant
{
public:
    virtual ~chat_participant() {}
    virtual void delivery(const char* header_data, const char* data) = 0;
    virtual std::string id() = 0;
    virtual std::string id() const = 0;
    virtual bool operator<(const chat_participant& participant) = 0;
    virtual bool operator>(const chat_participant& participant) = 0;
    virtual bool operator==(const chat_participant& participant) = 0;
};

#endif