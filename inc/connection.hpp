#ifndef __CONNECTION_H__
#define __CONNECTION_H__

#include <vector>

#include "logging.hpp"
#include "request.hpp"
#include "responder.hpp"

#define CONN_LOG_SIMPLE     false
#define CONN_LOG_ERROR      true
#define CONN_LOG_INFO       true
#define CONN_LOG_DEBUG      true

class connection: public Log {
public:
    connection() = delete;
    connection(int32_t, responder &);
    ~connection();

    int8_t readHeader(void);
    int8_t readBody(void);
    int8_t process(void);

private:
    const std::string class_name;

    int32_t conn_fd;
    request conn_req;
    responder &conn_resp;
    std::vector<uint8_t> input_buffer;
    std::vector<uint8_t> output_buffer;

    int8_t readSocket(std::vector<uint8_t> &, uint32_t);
    int8_t writeSocket(std::vector<uint8_t> &, uint32_t);
};

#endif
