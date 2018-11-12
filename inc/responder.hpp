#ifndef __RESPONDER_H__
#define __RESPONDER_H__

#include <string>
#include <vector>
#include <map>
#include "logging.hpp"
#include "support.hpp"

#define RESP_LOG_SIMPLE     false
#define RESP_LOG_ERROR      true
#define RESP_LOG_INFO       true
#define RESP_LOG_DEBUG      true

class responder: public Log {
private:
    const std::string class_name;

    std::vector<uint8_t> body_header;
    std::vector<uint8_t> body_content;
    std::vector<uint8_t> params;
    std::vector<uint8_t> stream;
    std::vector<uint8_t> data;
    std::map<std::string, std::string> params_map;
    std::map<std::string, std::string> stream_map;
    FCGI_Header_t header;

    bool params_ready;
    bool stream_ready;
    bool data_ready;

    int8_t buildParamsMap(void);
    int8_t buildStreamMap(void);
    size_t getHeaderBeginStdout(std::vector<uint8_t>::const_iterator &);
    size_t getStdoutBodyHeader(std::vector<uint8_t>::const_iterator &);
    size_t getStdoutBodyContent(std::vector<uint8_t>::const_iterator &);
    size_t getHeaderStdoutEOF(std::vector<uint8_t>::const_iterator &);
    size_t getHeaderEndRequest(std::vector<uint8_t>::const_iterator &);
    size_t getHeaderRequestComplete(std::vector<uint8_t>::const_iterator &);

protected:
    void writeHeader(std::string &);
    void writeBody(std::string &);
    const std::string &getParamValue(const std::string &, const std::string &) const;
    const std::string &getStreamValue(const std::string &, const std::string &) const;

public:
    responder();
    virtual ~responder() {}
    int8_t insertParams(std::vector<uint8_t> &);
    int8_t insertStream(std::vector<uint8_t> &);
    int8_t insertData(std::vector<uint8_t> &);

    bool isReadyToRespond(void) const;
    int8_t getResponse(std::vector<uint8_t> &);
    virtual void createResponse(void) = 0;

    void release(void);
};

#endif
