#ifndef __SUPPORT_H__
#define __SUPPORT_H__

#include <list>
#include <string>
#include <vector>
#include <sstream>
#include <unordered_map>
#include "constants.hpp"


struct FCGI_Header_t {
    static const size_t size = FCGI_HEADER_LEN;
    uint8_t type_id;
    uint32_t request_id;
    uint32_t content_len;
    uint32_t input_len;

    std::vector<uint8_t> raw_vect;
    union {
        struct {
            uint8_t version;
            uint8_t type;
            uint8_t request_id_B1;
            uint8_t request_id_B0;
            uint8_t content_len_B1;
            uint8_t content_len_B0;
            uint8_t padding_len;
            uint8_t reserved;
        } std;

        struct {
            uint8_t app_status_B3;
            uint8_t app_status_B2;
            uint8_t app_status_B1;
            uint8_t app_status_B0;
            uint8_t proto_status;
            uint8_t reserved[3];
        } end;

        uint8_t raw[size];
    } type;

    FCGI_Header_t() { clearAll(); raw_vect.reserve(size); }

    void prepareData(void);
    void syncToData(void);
    void syncToRaw(void);
    void clearData(void);
    void clearAll(void) { raw_vect.clear(); clearData(); }

    size_t stdSize(void) const { return size; }
    size_t endSize(void) const { return size; }
};

struct input_data {
public:
    enum type_t { TYPE_PARAM = 0, TYPE_STREAM };
    input_data() = delete;
    input_data(const std::vector<uint8_t> &src):
        it(src.cbegin()), end(src.cend()) {}

    int8_t findNextElem(type_t);
    bool isNameMatch(const std::string&) const;
    int8_t getValue(std::string &) const;
    std::pair <std::string, std::string> getPair(void) const
            { return std::make_pair(name, value); }

private:
    static const size_t reservation_size = 32;
    std::string name;
    std::string value;
    std::vector<uint8_t>::const_iterator it;
    std::vector<uint8_t>::const_iterator end;

    bool is_empty(void) const { return (it == end) ? true : false; }
    bool is4ByteEncodedVar(void) { return ((*it & 0x80) != 0) ? true : false; }
    uint32_t get1ByteEncodedVar(void);
    uint32_t get4ByteEncodedVar(void);
    int8_t findNextParamElem(void);
    int8_t findNextStreamElem(void);
};

struct customData
{
public:
    customData() = delete;
    customData(const input_data::type_t t, const std::vector<uint8_t> &d):
                                                        type(t), data(d) {};

    int8_t buildMap(void);
    int8_t getNextListEntry(std::pair<std::string, std::string> &);
    void addToList(const std::string &name, const std::string &value) {
        list.push_back(std::make_pair(name, value));
        it = list.cbegin();
    }

    const std::string &searchValue(const std::string &, const std::string &) const;

private:
    const input_data::type_t type;
    const std::vector<uint8_t> &data;

    std::unordered_map<std::string, std::string> map;
    std::list<std::pair<std::string, std::string>> list;
    std::list<std::pair<std::string, std::string>>::const_iterator it;

    void clear(void) {
        map.clear();
        list.clear();
    }
};

template <typename T>
class convertNumber {
public:
    convertNumber() = delete;
    convertNumber(const T input, std::ios_base &(*base_ref)(std::ios_base &)):
        base(base_ref), str(""), num(input) {}
    convertNumber(const std::string& input, std::ios_base &(*base_ref)(std::ios_base &)):
        base(base_ref), str(input), num(0)  {}

    const T fromString(void);
    const std::string toString(void);

private:
    std::stringstream stream;

    std::ios_base &(*base)(std::ios_base&);
    const std::string str;
    const T num;
};

template <typename T>
const T convertNumber<T>::fromString(void)
{
    T ret = (-1);

    if(str.size() > 0) {
        for(uint32_t i = 0; i < str.size(); i++) {
            if(isxdigit(str[i]) == 0) {
                return ret;
            }
        }

        stream<<base<<str;
        stream>>ret;
    }

    return ret;
}

template <typename T>
const std::string convertNumber<T>::toString(void)
{
    stream<<std::showbase<<base<<+num<<std::dec;
    return stream.str() + " ";
}


#define CONV_NUM_GET_STR(num_typedef, num_base, num)    \
    convertNumber<num_typedef>(num, std::num_base).toString()

#define CONV_STR_GET_NUM(num_typedef, num_base, string) \
    convertNumber<num_typedef>(string, std::num_base).fromString()

#define PREP_CLASS_NAME (std::string(__func__) + std::string(":"))

#endif
