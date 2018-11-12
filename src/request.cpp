#include "request.hpp"
#include "constants.hpp"

void request::beginRequest(std::vector<uint8_t> &input)
{
    role = (input[0] << 8) + input[1];
    flags = input[2];

    LOG_DEBUG(FUNC_ID(class_name),
        "Role: "  + CONV_NUM_GET_STR(uint32_t, dec, role) +
        "flags: " + CONV_NUM_GET_STR(uint8_t, dec, flags));
}

void request::abortRequest(std::vector<uint8_t> &input)
{
}

void request::setRequestParams(void)
{
    type = header.type_id;
    request_id = header.request_id;

    LOG_DEBUG(FUNC_ID(class_name),
        "Type: " + CONV_NUM_GET_STR(uint8_t, dec, type) +
        "len: "  + CONV_NUM_GET_STR(uint32_t, dec, header.input_len));
}

void request::prepareHeader(void)
{
    header.syncToData();
    header.prepareData();
    setRequestParams();
    header.clearAll();
}

bool request::keepConnection(void) const
{
    if((flags & FCGI_KEEP_CONN) == FCGI_KEEP_CONN) {
        return true;
    }

    return false;
}

