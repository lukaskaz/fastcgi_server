#ifndef __RESPONDER_H__
#define __RESPONDER_H__

#include <string>
#include <vector>
#include "logging.hpp"
#include "support.hpp"

#define RESP_LOG_SIMPLE     false
#define RESP_LOG_ERROR      true
#define RESP_LOG_INFO       true
#define RESP_LOG_DEBUG      true

class responder: public Log {
public:
    responder();
    virtual ~responder() {}

    int8_t insertParams(std::vector<uint8_t> &);
    int8_t insertStream(std::vector<uint8_t> &);
    int8_t insertData(std::vector<uint8_t> &);

    virtual void createResponse(void) = 0;
    int8_t getResponse(std::vector<uint8_t> &);
    void release(void);

    bool isReadyToRespond(void) const;

protected:
    std::vector<uint8_t> params;
    std::vector<uint8_t> stream;
    std::vector<uint8_t> data;

    void writeHeader(const std::string &);
    void writeBody(const std::string &);

private:
    const std::string class_name;

    std::vector<uint8_t> body_header;
    std::vector<uint8_t> body_content;
    FCGI_Header_t header;

    bool params_ready;
    bool stream_ready;
    bool data_ready;

    size_t getHeaderBeginStdout(std::vector<uint8_t>::const_iterator &);
    size_t getStdoutBodyHeader(std::vector<uint8_t>::const_iterator &);
    size_t getStdoutBodyContent(std::vector<uint8_t>::const_iterator &);
    size_t getHeaderStdoutEOF(std::vector<uint8_t>::const_iterator &);
    size_t getHeaderEndRequest(std::vector<uint8_t>::const_iterator &);
    size_t getHeaderRequestComplete(std::vector<uint8_t>::const_iterator &);
};

#endif
