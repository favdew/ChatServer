#ifndef CHAT_MESSAGE
#define CHAT_MESSAGE

#include <cstring>
#include <string>
#include <memory>

#pragma warning(disable:4996)

#define MAX_BUF_SIZE 10240
#define HEADER_SIZE 16
#define BODY_SIZE MAX_BUF_SIZE - HEADER_SIZE

#define MAX_ID_LENGTH 64
#define MAX_NICKNAME_LENGTH 64
#define MAX_PASSWORD_LENGTH 64
#define MAX_ROOM_NAME_LENGTH 400
#define MAX_NAME_LENGTH 32
#define MAX_EMAIL_LENGTH 64

#define CRYPTOPP_DEFAULT_BLOCK_SIZE 16
#define MAX_KEY_SIZE 8192

#define CODE_ALREADY_LOGINED "#101101"

enum MESSAGE_TYPE {
    MT_SEND_MESSAGE = 1,
    MT_CREATE_ROOM_MESSAGE,
    MT_JOIN_ROOM_MESSAGE
};

class talk_data
{
    typedef unsigned char td_flag;

public:
    talk_data()
        : id_(""), nickname_(""), flag_(0), content_length_(0), content_("")
    {}

    talk_data(std::string id, std::string nickname, td_flag flag, std::string content)
        : id_(id), nickname_(nickname), flag_(flag), content_(content)
    {
        content_length_ = content.size();
    }

    talk_data(const talk_data& data)
        : id_(data.id()), nickname_(data.nickname()), flag_(data.flag()),
        content_length_(data.content_length()), content_(data.content())
    {}

    talk_data(const char* data)
    {
        std::size_t pt = 0;

        char id_buf[MAX_ID_LENGTH];
        char nickname_buf[MAX_NICKNAME_LENGTH];

        std::memset(id_buf, 0, MAX_ID_LENGTH);
        std::memcpy(id_buf, data, MAX_ID_LENGTH);
        id_ = id_buf;
        pt += MAX_ID_LENGTH;

        std::memset(nickname_buf, 0, MAX_NICKNAME_LENGTH);
        std::memcpy(nickname_buf, data + pt, MAX_NICKNAME_LENGTH);
        nickname_ = nickname_buf;
        pt += MAX_NICKNAME_LENGTH;

        std::memcpy(&flag_, data + pt, sizeof(flag_));
        pt += sizeof(flag_);

        std::memcpy(&content_length_, data + pt, sizeof(content_length_));
        pt += sizeof(content_length_);

        char* content_buf = new char[content_length_ + 1];
        std::memset(content_buf, 0, content_length_ + 1);
        std::memcpy(content_buf, data + pt, content_length_ + 1);
        content_ = content_buf;
        delete[] content_buf;
    }

    char* serialization()
    {
        char* dest;
        std::size_t length = MAX_ID_LENGTH + MAX_NICKNAME_LENGTH + sizeof(content_length_) +
            sizeof(flag_) + (content_length_ + 1);
        dest = new char[length];
        std::memset(dest, 0, length);
        std::size_t pt = 0;

        char id_buf[MAX_ID_LENGTH];
        char nickname_buf[MAX_NICKNAME_LENGTH];

        std::memset(id_buf, 0, MAX_ID_LENGTH);
        std::memcpy(id_buf, id_.c_str(), id_.size() + 1);

        std::memset(nickname_buf, 0, MAX_NICKNAME_LENGTH);
        std::memcpy(nickname_buf, nickname_.c_str(), nickname_.size() + 1);

        std::memcpy(dest, id_buf, MAX_ID_LENGTH);
        pt += MAX_ID_LENGTH;

        std::memcpy(dest + pt, nickname_buf, MAX_NICKNAME_LENGTH);
        pt += MAX_NICKNAME_LENGTH;

        std::memcpy(dest + pt, &flag_, sizeof(flag_));
        pt += sizeof(flag_);

        std::memcpy(dest + pt, &content_length_, sizeof(content_length_));
        pt += sizeof(content_length_);

        std::memcpy(dest + pt, content_.c_str(), content_length_ + 1);

        return dest;
    }

    unsigned long long size()
    {
        unsigned long long ret = 0;

        ret += MAX_ID_LENGTH;
        ret += MAX_NICKNAME_LENGTH;
        ret += sizeof(flag_);
        ret += sizeof(content_length_);
        ret += content_length_ + 1;

        return ret;
    }

    const std::string id() { return id_; }
    const std::string id() const { return id_; }
    const std::string nickname() { return nickname_; }
    const std::string nickname() const { return nickname_; }
    void set_nickname(std::string nickname) { nickname_ = nickname; }
    void set_nickname(const char* nickname) { nickname_ = nickname; }
    const td_flag flag() { return flag_; }
    const td_flag flag() const { return flag_; }
    const unsigned long long content_length() { return content_length_; }
    const unsigned long long content_length() const { return content_length_; }
    const std::string content() { return content_; }
    const std::string content() const { return content_; }

    enum talk_data_flag {
        TDF_INVITE = 0x01,
        TDF_KICK,
        TDF_EXIT,
        TDF_JOIN,
        TDF_MESSAGE,
        TDF_CREATE
    };
private:
    std::string id_;
    std::string nickname_;
    td_flag flag_;
    unsigned long long content_length_;
    std::string content_;
};

class chat_message_header
{
public:
    chat_message_header()
        : body_size(0), message_type(0)
    {

    }

    chat_message_header(const char* data)
    {
        int pt = 0;
        std::memcpy(&body_size, data, sizeof(body_size));
        pt += sizeof(body_size);
        std::memcpy(&message_type, data + pt, sizeof(message_type));
    }

    chat_message_header(const unsigned long long body_size, const unsigned long long message_type)
        : body_size(body_size), message_type(message_type)
    {

    }

    chat_message_header(const chat_message_header& header)
        : body_size(header.body_size), message_type(header.message_type)
    {

    }

    bool is_valid()
    {
        if (body_size < 0 || body_size > BODY_SIZE)
        {
            return false;
        }

        for (unsigned long long mt = MT_FRONTVALUE + 1; mt < MT_LASTVALUE; mt++)
        {
            if (message_type == mt) return true;
        }

        return false;
    }

    char* serialization()
    {
        char* ser_buf;
        std::size_t length = sizeof(body_size) + sizeof(message_type);
        ser_buf = new char[length];
        std::size_t pt = 0;
        std::memcpy(ser_buf, &body_size, sizeof(body_size));
        pt += sizeof(body_size);
        std::memcpy(ser_buf + pt, &message_type, sizeof(message_type));
        return ser_buf;
    }

    static void convert_to_class(const char* data, chat_message_header* dest)
    {
        std::size_t pt = 0;
        std::memcpy(&dest->body_size, data, sizeof(dest->body_size));
        pt += sizeof(short);
        std::memcpy(&dest->message_type, data + pt, sizeof(dest->message_type));
    }

public:
    unsigned long long body_size;
    unsigned long long message_type;

    enum message_type {
        MT_FRONTVALUE = 0,
        MT_SENDMESSAGE,
        MT_CREATEROOM,
        MT_JOINROOM,
        MT_EXITROOM,
        MT_LOGIN,
        MT_CREATEKEY,
        MT_REGISTER,
        MT_CONFIRMUINFO,
        MT_CHANGEPASSWORD,
        MT_LASTVALUE
    };
};

static chat_message_header convert_to_header(const char* data)
{
    chat_message_header ret;
    std::memcpy(&ret.body_size, data, sizeof(ret.body_size));
    std::memcpy(&ret.message_type, data + sizeof(ret.body_size),
        sizeof(ret.body_size));

    return ret;
}

class chat_message : public std::enable_shared_from_this<chat_message>
{
public:
    chat_message() {}

    virtual char* serialization() = 0;
};

class cm_send_message : public chat_message,
    public std::enable_shared_from_this<cm_send_message>
{
public:
    cm_send_message(const char* data)
    {
        int pt = 0;
        std::memcpy(id, data, sizeof(id));
        pt += sizeof(id);
        std::memcpy(&room_number, data + pt, sizeof(room_number));
        pt += sizeof(room_number);
        std::memcpy(message_content, data + pt, std::strlen(data + pt));
    }

    ~cm_send_message()
    {
        if (message_content != nullptr) delete[] message_content;
    }

    char* serialization()
    {
        char* ser_buf;
        std::size_t length = sizeof(id) + sizeof(room_number)
            + (std::strlen(message_content) + 1);
        ser_buf = new char[length];
        std::size_t pt = 0;
        std::memcpy(ser_buf, id, sizeof(id));
        pt += sizeof(id);
        std::memcpy(ser_buf + pt, &room_number, sizeof(room_number));
        pt += sizeof(room_number);
        std::memcpy(ser_buf + pt, message_content, std::strlen(message_content) + 1);
        return ser_buf;
    }

    static void convert_to_class(const char* data, cm_send_message* dest)
    {
        std::size_t pt = 0;
        std::memcpy(dest->id, data, MAX_ID_LENGTH);
        pt += MAX_ID_LENGTH;
        std::memcpy(&dest->room_number, data + pt, sizeof(room_number));
        pt += sizeof(room_number);
        std::memcpy(&dest->message_length, data + pt, sizeof(message_length));
        pt += sizeof(message_length);
        dest->message_content = new char[dest->message_length];
        std::memcpy(dest->message_content, data + pt, dest->message_length);
    }
public:
    char id[MAX_ID_LENGTH];
    long long room_number;
    unsigned long long message_length;
    char* message_content;

};

class cm_room_message : public chat_message,
    public std::enable_shared_from_this<cm_room_message>
{
public:
    cm_room_message(const char* _id, const char* _room_name, long long _room_number = 0,
        unsigned long long _participants_num = 0)
        : room_number(room_number), participants_num(_participants_num),
        room_participants(nullptr)
    {
        std::memset(id, 0, MAX_ID_LENGTH);
        std::memset(room_name, 0, MAX_ROOM_NAME_LENGTH);
        std::memcpy(id, _id, MAX_ID_LENGTH);
        std::memcpy(room_name, _room_name, MAX_ROOM_NAME_LENGTH);
    }

    cm_room_message(const char* data)
    {
        unsigned long long pt = 0;
        std::memset(id, 0, MAX_ID_LENGTH);
        std::memset(room_name, 0, MAX_ROOM_NAME_LENGTH);
        std::memcpy(id, data, sizeof(id));
        pt += sizeof(id);
        std::memcpy(&room_number, data + pt, sizeof(room_number));
        pt += sizeof(room_number);
        std::memcpy(room_name, data + pt, sizeof(room_name));
        pt += sizeof(room_name);
        std::memcpy(&participants_num, data + pt, sizeof(participants_num));
        pt += sizeof(participants_num);

        if (participants_num != 0)
        {
            set_room_participants_id(data);
        }
        else
        {
            room_participants = nullptr;
        }
    }

    ~cm_room_message()
    {
        if (room_participants != nullptr)
        {
            for (std::size_t pnum = 0; pnum < participants_num; pnum++)
            {
                delete[] room_participants[pnum];
            }
            delete[] room_participants;
            room_participants = nullptr;
        }
    }

    char* serialization()
    {
        char* ser_buf;
        unsigned int length = sizeof(id) + sizeof(room_number) +
            sizeof(room_name) + sizeof(participants_num);
        for (unsigned long long i = 0; i < participants_num; i++)
            length += MAX_ID_LENGTH;
        ser_buf = new char[length];

        std::memset(ser_buf, 0, length);
        std::size_t pt = 0;
        std::memcpy(ser_buf, this->id, sizeof(id));
        pt += sizeof(id);
        std::memcpy(ser_buf + pt, &room_number, sizeof(room_number));
        pt += sizeof(room_number);
        std::memcpy(ser_buf + pt, room_name, sizeof(room_name));
        pt += sizeof(room_name);
        std::memcpy(ser_buf + pt, &participants_num, sizeof(participants_num));
        pt += sizeof(participants_num);
        for (unsigned long long i = 0; i < participants_num; i++)
        {
            std::memcpy(ser_buf + pt, room_participants[i], MAX_ID_LENGTH);
            pt += MAX_ID_LENGTH;
        }
        return ser_buf;
    }

    //It will be deleted
    static void convert_to_class(const char* data, cm_room_message* dest)
    {
        std::size_t pt = 0;
        std::memcpy(dest->id, data, sizeof(dest->id));
        pt += sizeof(dest->id);
        std::memcpy(&dest->room_number, data + pt, sizeof(dest->room_number));
        pt += sizeof(dest->room_number);
        std::memcpy(dest->room_name, data + pt, sizeof(dest->room_name));
    }

    void set_room_participants_id(const char* data)
    {
        unsigned int pt = MAX_ID_LENGTH + sizeof(room_number) +
            MAX_ROOM_NAME_LENGTH + sizeof(participants_num);
        const char* buf = data + pt;

        unsigned int part_num;
        if (participants_num > (unsigned long long)(UINT32_MAX))
            part_num = UINT32_MAX;
        else
            part_num = (unsigned int)(participants_num);

        room_participants = new char* [part_num];
        for (unsigned long long i = 0; i < participants_num; i++)
        {
            room_participants[i] = new char[MAX_ID_LENGTH];
            std::memset(room_participants[i], 0, MAX_ID_LENGTH);
            std::memcpy(room_participants[i], data + pt, MAX_ID_LENGTH);
            pt += MAX_ID_LENGTH;
        }
    }

    unsigned long long size()
    {
        unsigned long long ret = sizeof(id) + sizeof(room_number) +
            sizeof(room_name) + sizeof(participants_num) +
            (participants_num * MAX_ID_LENGTH);
        return ret;
    }

public:
    char id[MAX_ID_LENGTH];
    long long room_number;
    char room_name[MAX_ROOM_NAME_LENGTH];
    unsigned long long participants_num;
    char** room_participants;
};

class cm_send_room_message : public chat_message,
    std::enable_shared_from_this<cm_send_room_message>
{
public:
    cm_send_room_message()
    {}

    cm_send_room_message(const talk_data& _td)
        : td(_td)
    {}

    cm_send_room_message(const char* data)
        : td(data)
    {}

    char* serialization()
    {
        char* ret = td.serialization();
        return ret;
    }

    unsigned long long size()
    {
        unsigned long long ret = 0;

        ret += td.size();

        return ret;
    }
public:
    talk_data td;
};

class cm_join_room_message : public chat_message,
    public std::enable_shared_from_this<cm_join_room_message>
{
public:
    cm_join_room_message()
        : room_number(0)
    {
        std::memset(id, 0, MAX_ID_LENGTH);
        std::memset(room_name, 0, MAX_ROOM_NAME_LENGTH);
    }

    cm_join_room_message(const char* _id, long long _room_number, const char* _room_name)
        : room_number(_room_number)
    {
        std::memset(id, 0, MAX_ID_LENGTH);
        std::memcpy(id, _id, MAX_ID_LENGTH);

        std::memset(room_name, 0, MAX_ROOM_NAME_LENGTH);
        std::memcpy(room_name, _room_name, MAX_ROOM_NAME_LENGTH);
    }

    cm_join_room_message(const char* data)
    {
        std::size_t pt = 0;

        std::memset(id, 0, MAX_ID_LENGTH);
        std::memcpy(id, data, MAX_ID_LENGTH);
        pt += MAX_ID_LENGTH;

        std::memcpy(&room_number, data + pt, sizeof(room_number));
        pt += sizeof(room_number);

        std::memset(room_name, 0, MAX_ROOM_NAME_LENGTH);
        std::memcpy(room_name, data + pt, MAX_ROOM_NAME_LENGTH);
    }

    char* serialization()
    {
        char* ser_buf;
        std::size_t length = MAX_ID_LENGTH + sizeof(room_number) + MAX_ROOM_NAME_LENGTH;
        ser_buf = new char[length];
        std::memset(ser_buf, 0, length);

        std::size_t pt = 0;

        std::memcpy(ser_buf, id, MAX_ID_LENGTH);
        pt += MAX_ID_LENGTH;

        std::memcpy(ser_buf + pt, &room_number, sizeof(room_number));
        pt += sizeof(room_number);

        std::memcpy(ser_buf + pt, room_name, MAX_ROOM_NAME_LENGTH);

        return ser_buf;
    }

    unsigned long long size()
    {
        unsigned long long length = MAX_ID_LENGTH + sizeof(room_number) + MAX_ROOM_NAME_LENGTH;
        return length;
    }
public:
    char id[MAX_ID_LENGTH];
    long long room_number;
    char room_name[MAX_ROOM_NAME_LENGTH];
};

class cm_login_message : public chat_message,
    std::enable_shared_from_this<cm_login_message>
{
public:
    cm_login_message()
    {
        std::memset(id, 0, sizeof(id));
        std::memset(password, 0, sizeof(password));
        is_valid = false;
        std::memset(iv, 0, sizeof(iv));
    }

    cm_login_message(const char* _id, const char* _password, bool _is_valid = false)
        : is_valid(_is_valid)
    {
        std::memcpy(id, _id, sizeof(id));
        std::memcpy(password, _password, sizeof(password));
        std::memset(iv, 0, sizeof(iv));
    }

    cm_login_message(const char* data)
    {
        std::size_t pt = 0;
        std::memcpy(id, data, sizeof(id));
        pt += sizeof(id);
        std::memcpy(password, data + pt, sizeof(password));
        pt += sizeof(password);
        std::memcpy(&is_valid, data + pt, sizeof(is_valid));
        pt += sizeof(is_valid);
        std::memcpy(iv, data + pt, sizeof(iv));
    }

    char* serialization()
    {
        char* ser_buf;
        ser_buf = new char[sizeof(id) + sizeof(password) + sizeof(is_valid) + sizeof(iv)];

        std::size_t pt = 0;
        std::memcpy(ser_buf, id, sizeof(id));
        pt += sizeof(id);
        std::memcpy(ser_buf + pt, password, sizeof(password));
        pt += sizeof(password);
        std::memcpy(ser_buf + pt, &is_valid, sizeof(is_valid));
        pt += sizeof(is_valid);
        std::memcpy(ser_buf + pt, iv, sizeof(iv));

        return ser_buf;
    }

    char* serialization_except_iv()
    {
        char* ser_buf;
        ser_buf = new char[sizeof(id) + sizeof(password) + sizeof(is_valid)];

        std::size_t pt = 0;
        std::memcpy(ser_buf, id, sizeof(id));
        pt += sizeof(id);
        std::memcpy(ser_buf + pt, password, sizeof(password));
        pt += sizeof(password);
        std::memcpy(ser_buf + pt, &is_valid, sizeof(is_valid));

        return ser_buf;
    }

    unsigned long long size()
    {
        unsigned long long ret = 0;
        ret += sizeof(id);
        ret += sizeof(password);
        ret += sizeof(is_valid);
        ret += sizeof(iv);

        return ret;
    }

    unsigned long long iv_size()
    {
        return sizeof(iv);
    }

    void set_iv(unsigned char* _iv, std::size_t size = CRYPTOPP_DEFAULT_BLOCK_SIZE)
    {
        std::memcpy(iv, _iv, size);
    }

public:
    char id[MAX_ID_LENGTH];
    char password[MAX_PASSWORD_LENGTH];
    bool is_valid;
    unsigned char iv[CRYPTOPP_DEFAULT_BLOCK_SIZE];
};

class cm_exit_room_message : public chat_message,
    std::enable_shared_from_this<cm_exit_room_message>
{
public:
    cm_exit_room_message()
    {
        std::memset(id, 0, MAX_ID_LENGTH);
        std::memset(room_name, 0, MAX_ROOM_NAME_LENGTH);
    }

    cm_exit_room_message(const char* _id, const char* _room_name)
    {
        std::strcpy(id, _id);
        std::strcpy(room_name, _room_name);
    }

    cm_exit_room_message(const char* data)
    {
        std::size_t pt = 0;

        std::memset(id, 0, MAX_ID_LENGTH);
        std::memcpy(id, data, MAX_ID_LENGTH);
        pt += MAX_ID_LENGTH;

        std::memset(room_name, 0, MAX_ROOM_NAME_LENGTH);
        std::memcpy(room_name, data + pt, MAX_ROOM_NAME_LENGTH);
    }

    char* serialization()
    {
        char* ser_buf = new char[MAX_ID_LENGTH + MAX_ROOM_NAME_LENGTH];
        std::size_t pt = 0;

        std::memcpy(ser_buf, id, MAX_ID_LENGTH);
        pt += MAX_ID_LENGTH;

        std::memcpy(ser_buf + pt, room_name, MAX_ROOM_NAME_LENGTH);

        return ser_buf;
    }

    unsigned long long size()
    {
        unsigned long long ret = 0;
        ret += MAX_ID_LENGTH;
        ret += MAX_ROOM_NAME_LENGTH;
        return ret;
    }

public:
    char id[MAX_ID_LENGTH];
    char room_name[MAX_ROOM_NAME_LENGTH];
};

class cm_create_key_message : public chat_message,
    std::enable_shared_from_this<cm_create_key_message>
{
public:
    cm_create_key_message()
        : key_length(0)
    {
        std::memset(data, 0, MAX_KEY_SIZE);
    }

    cm_create_key_message(const char* _data, std::size_t data_size)
    {
        std::memset(data, 0, MAX_KEY_SIZE);
        std::memcpy(data, _data, data_size <= MAX_KEY_SIZE ? data_size : MAX_KEY_SIZE);
        key_length = data_size <= MAX_KEY_SIZE ? data_size : MAX_KEY_SIZE;
    }

    //When it is initialized with serialized data
    cm_create_key_message(const char* _data)
    {
        std::size_t pt = 0;
        std::memcpy(data, _data, sizeof(data));
        pt += sizeof(data);
        std::memcpy(&key_length, _data + pt, sizeof(key_length));
    }

    char* serialization()
    {
        char* ser_buf;
        std::size_t size = sizeof(data) + sizeof(key_length);
        ser_buf = new char[size];
        std::size_t pt = 0;

        std::memset(ser_buf, 0, size);

        std::memcpy(ser_buf, data, sizeof(data));
        pt += sizeof(data);

        std::memcpy(ser_buf + pt, &key_length, sizeof(key_length));

        return ser_buf;
    }

    unsigned long long size()
    {
        unsigned long long ret = 0;

        ret += sizeof(data);
        ret += sizeof(key_length);

        return ret;
    }

public:
    char data[MAX_KEY_SIZE];
    unsigned long long key_length;
};

class cm_register_message
{
public:
    cm_register_message()
    {
        std::memset(id, 0, sizeof(id));
        std::memset(password, 0, sizeof(password));
        std::memset(name, 0, sizeof(name));
        std::memset(email, 0, sizeof(email));
        std::memset(iv, 0, sizeof(iv));
    }

    cm_register_message(const char* _id, const char* _password, const char* _name, const char* _email)
    {
        std::memcpy(id, _id, sizeof(id));
        std::memcpy(password, _password, sizeof(password));
        std::memcpy(name, _name, sizeof(name));
        std::memcpy(email, _email, sizeof(email));
        std::memset(iv, 0, sizeof(iv));
    }

    cm_register_message(const char* data)
    {
        std::size_t pt = 0;

        std::memcpy(id, data, sizeof(id));
        pt += sizeof(id);

        std::memcpy(password, data + pt, sizeof(password));
        pt += sizeof(password);

        std::memcpy(name, data + pt, sizeof(name));
        pt += sizeof(name);

        std::memcpy(email, data + pt, sizeof(email));
        pt += sizeof(email);

        std::memcpy(iv, data + pt, sizeof(iv));
    }

    char* serialization()
    {
        char* ser_buf;
        std::size_t size = sizeof(id) + sizeof(password) + sizeof(name) + sizeof(email) +
            sizeof(iv);
        ser_buf = new char[size];
        std::size_t pt = 0;

        std::memcpy(ser_buf, id, sizeof(id));
        pt += sizeof(id);

        std::memcpy(ser_buf + pt, password, sizeof(password));
        pt += sizeof(password);

        std::memcpy(ser_buf + pt, name, sizeof(name));
        pt += sizeof(name);

        std::memcpy(ser_buf + pt, email, sizeof(email));
        pt += sizeof(email);

        std::memcpy(ser_buf + pt, iv, sizeof(iv));

        return ser_buf;
    }

    char* serialization_except_iv()
    {
        char* ser_buf;
        std::size_t size = sizeof(id) + sizeof(password) + sizeof(name) + sizeof(email);
        ser_buf = new char[size];
        std::size_t pt = 0;

        std::memcpy(ser_buf, id, sizeof(id));
        pt += sizeof(id);

        std::memcpy(ser_buf + pt, password, sizeof(password));
        pt += sizeof(password);

        std::memcpy(ser_buf + pt, name, sizeof(name));
        pt += sizeof(name);

        std::memcpy(ser_buf + pt, email, sizeof(email));

        return ser_buf;
    }

    unsigned long long size()
    {
        unsigned long long ret = 0;

        ret += sizeof(id);
        ret += sizeof(password);
        ret += sizeof(name);
        ret += sizeof(email);
        ret += sizeof(iv);

        return ret;
    }

    unsigned long long iv_size()
    {
        return sizeof(iv);
    }

    void set_iv(unsigned char* _iv, std::size_t size = CRYPTOPP_DEFAULT_BLOCK_SIZE)
    {
        std::memcpy(iv, _iv, size);
    }
public:
    char id[MAX_ID_LENGTH];
    char password[MAX_PASSWORD_LENGTH];
    char name[MAX_NAME_LENGTH];
    char email[MAX_NAME_LENGTH];
    unsigned char iv[CRYPTOPP_DEFAULT_BLOCK_SIZE];
};

class cm_confirm_uinfo_message
{
public:
    cm_confirm_uinfo_message()
    {
        std::memset(id, 0, sizeof(id));
        std::memset(name, 0, sizeof(name));
        std::memset(email, 0, sizeof(email));
        std::memset(iv, 0, sizeof(iv));
    }

    cm_confirm_uinfo_message(const char* _id, const char* _name, const char* _email)
    {
        std::memcpy(id, _id, sizeof(id));
        std::memcpy(name, _name, sizeof(name));
        std::memcpy(email, _email, sizeof(email));
        std::memset(iv, 0, sizeof(iv));
    }

    cm_confirm_uinfo_message(const char* data)
    {
        std::size_t pt = 0;

        std::memcpy(id, data, sizeof(id));
        pt += sizeof(id);

        std::memcpy(name, data + pt, sizeof(name));
        pt += sizeof(name);

        std::memcpy(email, data + pt, sizeof(email));
        pt += sizeof(email);

        std::memcpy(iv, data + pt, sizeof(iv));
    }

    char* serialization()
    {
        char* ser_buf;
        std::size_t size = sizeof(id) + sizeof(name) + sizeof(email) + sizeof(iv);
        ser_buf = new char[size];
        std::size_t pt = 0;

        std::memcpy(ser_buf, id, sizeof(id));
        pt += sizeof(id);

        std::memcpy(ser_buf + pt, name, sizeof(name));
        pt += sizeof(name);

        std::memcpy(ser_buf + pt, email, sizeof(email));
        pt += sizeof(email);

        std::memcpy(ser_buf + pt, iv, sizeof(iv));

        return ser_buf;
    }

    char* serialization_except_iv()
    {
        char* ser_buf;
        std::size_t size = sizeof(id) + sizeof(name) + sizeof(email);
        ser_buf = new char[size];
        std::size_t pt = 0;

        std::memcpy(ser_buf, id, sizeof(id));
        pt += sizeof(id);

        std::memcpy(ser_buf + pt, name, sizeof(name));
        pt += sizeof(name);

        std::memcpy(ser_buf + pt, email, sizeof(email));

        return ser_buf;
    }

    unsigned long long size()
    {
        unsigned long long ret = 0;

        ret += sizeof(id);
        ret += sizeof(name);
        ret += sizeof(email);
        ret += sizeof(iv);

        return ret;
    }

    unsigned long long iv_size()
    {
        return sizeof(iv);
    }

    void set_iv(unsigned char* _iv, std::size_t size = CRYPTOPP_DEFAULT_BLOCK_SIZE)
    {
        std::memcpy(iv, _iv, size);
    }
public:
    char id[MAX_ID_LENGTH];
    char name[MAX_NAME_LENGTH];
    char email[MAX_EMAIL_LENGTH];
    unsigned char iv[CRYPTOPP_DEFAULT_BLOCK_SIZE];
};

class cm_change_password_message
{
public:
    cm_change_password_message()
    {
        std::memset(id, 0, sizeof(id));
        std::memset(password, 0, sizeof(password));
        std::memset(iv, 0, sizeof(iv));
    }

    cm_change_password_message(const char* _id, const char* _password)
    {
        std::memcpy(id, _id, sizeof(id));
        std::memcpy(password, _password, sizeof(password));
        std::memset(iv, 0, sizeof(iv));
    }

    cm_change_password_message(const char* data)
    {
        std::size_t pt = 0;

        std::memcpy(id, data, sizeof(id));
        pt += sizeof(id);
        std::memcpy(password, data + pt, sizeof(password));
        pt += sizeof(password);
        std::memcpy(iv, data + pt, sizeof(iv));
    }

    char* serialization()
    {
        char* ser_buf;
        ser_buf = new char[sizeof(id) + sizeof(password) + sizeof(iv)];
        std::size_t pt = 0;

        std::memcpy(ser_buf, id, sizeof(id));
        pt += sizeof(id);

        std::memcpy(ser_buf + pt, password, sizeof(password));
        pt += sizeof(password);

        std::memcpy(ser_buf + pt, iv, sizeof(iv));

        return ser_buf;
    }

    char* serialization_except_iv()
    {
        char* ser_buf;
        ser_buf = new char[sizeof(id) + sizeof(password)];
        std::size_t pt = 0;

        std::memcpy(ser_buf, id, sizeof(id));
        pt += sizeof(id);

        std::memcpy(ser_buf + pt, password, sizeof(password));

        return ser_buf;
    }

    unsigned long long size()
    {
        unsigned long long ret = 0;

        ret += sizeof(id);
        ret += sizeof(password);
        ret += sizeof(iv);

        return ret;
    }

    unsigned long long iv_size()
    {
        return sizeof(iv);
    }

    void set_iv(unsigned char* _iv, std::size_t size = CRYPTOPP_DEFAULT_BLOCK_SIZE)
    {
        std::memcpy(iv, _iv, size);
    }
public:
    char id[MAX_ID_LENGTH];
    char password[MAX_PASSWORD_LENGTH];
    unsigned char iv[CRYPTOPP_DEFAULT_BLOCK_SIZE];
};

#endif