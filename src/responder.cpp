#include <string.h>
#include "constants.hpp"
#include "responder.hpp"


responder::responder():
    Log(RESP_LOG_SIMPLE, RESP_LOG_ERROR, RESP_LOG_INFO, RESP_LOG_DEBUG),
    class_name(PREP_CLASS_NAME)
{
    body_header.reserve(FCGI_BUFFER_SIZE);
    body_content.reserve(FCGI_BUFFER_SIZE);
    params.reserve(FCGI_BUFFER_SIZE);
    stream.reserve(FCGI_BUFFER_SIZE);

    release();
}

void responder::writeHeader(const std::string &header)
{
    body_header.insert(body_header.end(), header.begin(), header.end());
}

void responder::writeBody(const std::string& body)
{
    body_content.insert(body_content.end(), body.begin(), body.end());
}

int8_t responder::insertParams(std::vector<uint8_t> &input)
{
    if(input.size() > 0) {
        params.insert(params.end(), input.begin(), input.end());
        //buildParamsMap();
        return 0;
    }
    else {
        params_ready = true;
    }

    return (-1);
}

int8_t responder::insertStream(std::vector<uint8_t> &input)
{
    if(input.size() > 0) {
        stream.insert(stream.end(), input.begin(), input.end());
        //buildStreamMap();
        return 0;
    }
    else {
        stream_ready = true;
    }

    return (-1);
}

int8_t responder::insertData(std::vector<uint8_t> &input)
{
    if(input.size() > 0) {
        data.insert(stream.end(), input.begin(), input.end());
        return 0;
    }
    else {
        data_ready = true;
    }

    return (-1);
}

bool responder::isReadyToRespond(void) const
{
    if(params_ready == true && stream_ready == true) {
        return true;
    }

    return false;
}

void responder::release(void)
{
    body_content.clear();
    body_header.clear();
    params.clear();
    stream.clear();
    data.clear();

    params_ready = false;
    stream_ready = false;
    data_ready = false;
}

int8_t responder::getResponse(std::vector<uint8_t> &output)
{
    size_t size = 0;
    std::vector<uint8_t>::const_iterator it;
    static size_t (responder::*resp_fn[])(std::vector<uint8_t>::const_iterator &) =
    {
        &responder::getHeaderBeginStdout,
        &responder::getStdoutBodyHeader,
        &responder::getStdoutBodyContent,
        &responder::getHeaderStdoutEOF,
        &responder::getHeaderEndRequest,
        &responder::getHeaderRequestComplete
    };
    static size_t resp_fn_size = sizeof(resp_fn)/sizeof(resp_fn[0]);

    for(uint8_t i = 0; i < resp_fn_size; i++) {
        if(resp_fn[i] != nullptr) {
            size = (this->*resp_fn[i])(it);
            if(size > 0) {
                output.insert(output.end(), it, it+size);
            }
            else {
                return (-1);
            }
        }
    }

    return 0;
}

size_t responder::getHeaderBeginStdout(std::vector<uint8_t>::const_iterator &it)
{
    size_t length = body_header.size() + body_content.size();

    header.clearAll();
    header.type.std.version = FCGI_VERSION_1;
    header.type.std.type    = FCGI_STDOUT;
    header.type.std.request_id_B1  = 0x00;
    header.type.std.request_id_B0  = 0x01;
    header.type.std.content_len_B1 = (length>>8)&0xFF;
    header.type.std.content_len_B0 = length & 0xFF;
    header.type.std.padding_len = 0x00;
    header.type.std.reserved    = 0x00;
    header.syncToRaw();

    it = header.raw_vect.cbegin();
    return header.stdSize();
}

size_t responder::getStdoutBodyHeader(std::vector<uint8_t>::const_iterator &it)
{
    it = body_header.cbegin();
    return body_header.size();
}

size_t responder::getStdoutBodyContent(std::vector<uint8_t>::const_iterator &it)
{
    it = body_content.cbegin();
    return body_content.size();
}

size_t responder::getHeaderStdoutEOF(std::vector<uint8_t>::const_iterator &it)
{
    size_t length = 0;

    header.clearAll();
    header.type.std.version = FCGI_VERSION_1;
    header.type.std.type    = FCGI_STDOUT;
    header.type.std.request_id_B1  = 0x00;
    header.type.std.request_id_B0  = 0x01;
    header.type.std.content_len_B1 = (length>>8)&0xFF;
    header.type.std.content_len_B0 = length & 0xFF;
    header.type.std.padding_len = 0x00;
    header.type.std.reserved    = 0x00;
    header.syncToRaw();

    it = header.raw_vect.cbegin();
    return header.stdSize();
}

size_t responder::getHeaderEndRequest(std::vector<uint8_t>::const_iterator &it)
{
    size_t length = header.endSize();

    header.clearAll();
    header.type.std.version = FCGI_VERSION_1;
    header.type.std.type    = FCGI_END_REQUEST;
    header.type.std.request_id_B1  = 0x00;
    header.type.std.request_id_B0  = 0x01;
    header.type.std.content_len_B1 = (length>>8)&0xFF;
    header.type.std.content_len_B0 = length & 0xFF;
    header.type.std.padding_len = 0x00;
    header.type.std.reserved    = 0x00;
    header.syncToRaw();

    it = header.raw_vect.cbegin();
    return header.stdSize();
}

size_t responder::getHeaderRequestComplete(std::vector<uint8_t>::const_iterator &it)
{
    header.clearAll();
    header.type.end.app_status_B3 = 0x00;
    header.type.end.app_status_B2 = 0x00;
    header.type.end.app_status_B1 = 0x00;
    header.type.end.app_status_B0 = 0x00;
    header.type.end.proto_status  = FCGI_REQUEST_COMPLETE;
    memset(header.type.end.reserved, 0, (size_t)sizeof(header.type.end.reserved));
    header.syncToRaw();

    it = header.raw_vect.cbegin();
    return header.endSize();
}
