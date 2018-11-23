#include <algorithm>
#include "support.hpp"


void FCGI_Header_t::prepareData(void)
{
    request_id = (type.std.request_id_B1 << 8) + type.std.request_id_B0;
    content_len = (type.std.content_len_B1 << 8) + type.std.content_len_B0;

    type_id = type.std.type;
    input_len = content_len + type.std.padding_len;
}

void FCGI_Header_t::clearData(void)
{
    for(uint32_t i = 0; i < size; i++) {
        type.raw[i] = 0;
    }
}

void FCGI_Header_t::syncToData(void)
{
    for(uint32_t i = 0; i < size && i < raw_vect.size(); i++) {
        type.raw[i] = raw_vect[i];
    }
}

void FCGI_Header_t::syncToRaw(void)
{
    raw_vect.clear();
    for(uint32_t i = 0; i < size; i++) {
        raw_vect.push_back(type.raw[i]);
    }
}

uint32_t input_data::get1ByteEncodedVar(void)
{
    return *it++;
}

uint32_t input_data::get4ByteEncodedVar(void)
{
    uint32_t ret = 0;

    ret += (*it++ & 0x7F) << 24;
    ret +=  *it++ << 16;
    ret +=  *it++ << 8;
    ret +=  *it++;

    return ret;
}

int8_t input_data::findNextElem(type_t type)
{
    static int8_t (input_data::*find_fn[])(void) =
    {
        [TYPE_PARAM]  = &input_data::findNextParamElem,
        [TYPE_STREAM] = &input_data::findNextStreamElem
    };

    if(find_fn[type] != nullptr) {
        return (this->*find_fn[type])();
    }

    return (-1);
}

int8_t input_data::findNextParamElem(void)
{
    if(is_empty() == false) {
        uint32_t nameLen = 0, valueLen = 0;

        nameLen = (is4ByteEncodedVar() == true) ?
                get4ByteEncodedVar() : get1ByteEncodedVar();

        valueLen = (is4ByteEncodedVar() == true) ?
                get4ByteEncodedVar() : get1ByteEncodedVar();

        name = std::string(it, it+nameLen);
        it += nameLen;
        value = std::string(it, it+valueLen);
        it += valueLen;
        return 0;
    }

    return (-1);
}

int8_t input_data::findNextStreamElem(void)
{
    static const char name_val_separator = '=';
    static const char elems_separator = '&';
    static const char coded_char_prefix = '%';

    if(is_empty() == false) {
        std::vector<uint8_t>::const_iterator tmp;

        tmp = std::find(it, end, name_val_separator);
        if(tmp != end) {
            name = std::string(it, tmp);

            value.clear();
            value.reserve(reservation_size);
            for(it = tmp + 1; it < end && *it != elems_separator; it++) {
                if(*it == coded_char_prefix) {
                    int16_t code = CONV_STR_GET_NUM(int16_t, hex, std::string(it+1, it+3));
                    if(code > 0) {
                        value.push_back((char)code);
                        it += 2;
                        continue;
                    }
                }

                value.push_back(*it);
            }

            it = (it == end) ? it : it + 1;
            return  0;
        }
    }

    return (-1);
}

bool input_data::isNameMatch(const std::string& req) const
{
    if(name.compare(req)) {
        return false;
    }

    return true;
}

int8_t input_data::getValue(std::string &val) const
{
    if(value.size()) {
        val = value;
        return 0;
    }

    return (-1);
}

int8_t customData::listGetEntry(std::pair<std::string, std::string> &param)
{
    if(list.empty() == false) {
        if(it != list.cend()) {
            param = *it++;
            return 0;
        }
    }

    return (-1);
}

int8_t customData::mapBuild(void)
{
    if(data.size() > 0) {
        input_data input(data);

        clear();
        while(input.findNextElem(type) == 0) {
            map.insert(input.getPair());
        }
        return 0;
    }

    return (-1);
}

const std::string &customData::mapGetValue(const std::string &key,
                                        const std::string &default_value) const
{
    if(map.size() > 0) {
        std::unordered_map<std::string, std::string>::const_iterator it;

        it = map.find(key);
        if(it != map.end()) {
            return it->second;
        }
    }

    return default_value;
}

