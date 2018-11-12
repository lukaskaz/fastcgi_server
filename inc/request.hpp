#ifndef __REQUEST_H__
#define __REQUEST_H__

#include <vector>
#include "logging.hpp"
#include "support.hpp"

#define REQ_LOG_SIMPLE      false
#define REQ_LOG_ERROR       true
#define REQ_LOG_INFO        true
#define REQ_LOG_DEBUG       true

class request: public Log {
private:
    const std::string class_name;

    uint32_t request_id;
    uint32_t role;
    uint8_t type;
    uint8_t flags;
    FCGI_Header_t header;

    void setRequestParams(void);
public:
    request(): Log(REQ_LOG_SIMPLE, REQ_LOG_ERROR, REQ_LOG_INFO, REQ_LOG_DEBUG),
                class_name(PREP_CLASS_NAME),
                request_id(0), role(0), type(0), flags(0) {}
    ~request() {}

    void beginRequest(std::vector<uint8_t> &);
    void abortRequest(std::vector<uint8_t> &);
    void prepareHeader(void);
    std::vector<uint8_t> &getHeaderBuffer(void) { return header.raw_vect; }

    uint32_t getRole(void) const { return role; }
    uint8_t getType(void)  const { return type; }
    bool keepConnection(void)  const;
    size_t getHeaderSize(void) const { return header.stdSize(); }
    size_t getInputSize(void)  const { return (size_t)header.input_len; }
};

#endif
