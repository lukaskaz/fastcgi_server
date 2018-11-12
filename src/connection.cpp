#include <unistd.h>
#include <string.h>
#include <sys/socket.h>

#include "connection.hpp"
#include "constants.hpp"
#include "support.hpp"

connection::connection(int32_t fd, responder &user_responder):
    Log(CONN_LOG_SIMPLE, CONN_LOG_ERROR, CONN_LOG_INFO, CONN_LOG_DEBUG),
    class_name(PREP_CLASS_NAME), conn_fd(fd), conn_resp(user_responder)
{
    input_buffer.reserve(FCGI_BUFFER_SIZE);
    output_buffer.reserve(FCGI_BUFFER_SIZE);
}

connection::~connection()
{
    shutdown(conn_fd, SHUT_RDWR);
    close(conn_fd);
    conn_fd = (-1);
    conn_resp.release();
}

int8_t connection::readSocket(std::vector<uint8_t> &buffer, uint32_t bytes_to_read)
{
    std::vector<uint8_t> tmp_vect(bytes_to_read, 0);
    std::vector<uint8_t>::iterator it = tmp_vect.begin();
    int32_t received = 0;

    while(bytes_to_read) {
        received = read(conn_fd, &(*it), bytes_to_read);
        if(received > 0) {
            LOG_DEBUG(FUNC_ID(class_name),
                "Receiving: " + CONV_NUM_GET_STR(int32_t, dec, received) +
                "reqSize: "   + CONV_NUM_GET_STR(uint32_t, dec, bytes_to_read));

            it += received;
            bytes_to_read -= received;
        }
        else {
            LOG_ERROR(FUNC_ID(class_name), "Cannot receive from client");
            return (-1);
        }
    }
    buffer.insert(buffer.end(), tmp_vect.begin(), tmp_vect.end());

    return 0;
}

int8_t connection::writeSocket(std::vector<uint8_t> &buffer, uint32_t bytes_to_write)
{
    std::vector<uint8_t>::const_iterator it = buffer.begin();
    int32_t written = 0;

    while(bytes_to_write) {
        written = write(conn_fd, &(*it), bytes_to_write);
        if(written > 0) {
            it += written;
            bytes_to_write -= written;
        }
        else {
            LOG_ERROR(FUNC_ID(class_name), "Cannot respond to client");
            return (-1);
        }
    }

    return 0;
}

int8_t connection::process(void)
{
    LOG_DEBUG(FUNC_ID(class_name),
        "Type: " + CONV_NUM_GET_STR(uint8_t, dec, conn_req.getType())  +
        "len: "  + CONV_NUM_GET_STR(size_t, dec, input_buffer.size()) +
        "prep: " + CONV_NUM_GET_STR(bool, dec, conn_resp.isReadyToRespond()));

    switch (conn_req.getType()) {
        case FCGI_BEGIN_REQUEST:
            conn_req.beginRequest(input_buffer);
            break;
        case FCGI_ABORT_REQUEST:
            conn_req.abortRequest(input_buffer);
            break;
        case FCGI_PARAMS:
            conn_resp.insertParams(input_buffer);
            break;
        case FCGI_STDIN:
            conn_resp.insertStream(input_buffer);
            break;
        case FCGI_DATA:
            conn_resp.insertData(input_buffer);
            break;
    }

    LOG_DEBUG(FUNC_ID(class_name),
        "Prep: " + CONV_NUM_GET_STR(bool, dec, conn_resp.isReadyToRespond()));

    if(conn_resp.isReadyToRespond() == true) {
        conn_resp.createResponse();
        if(conn_resp.getResponse(output_buffer) == 0) {
            writeSocket(output_buffer, output_buffer.size());
        }

        if (conn_req.keepConnection() == false) {
            return (-1);
        }
    }

    return 0;
}

int8_t connection::readHeader(void)
{
    if(readSocket(conn_req.getHeaderBuffer(), conn_req.getHeaderSize()) == 0) {
        conn_req.prepareHeader();
        return 0;
    }

    return (-1);
}

int8_t connection::readBody(void)
{
    input_buffer.clear();
    return readSocket(input_buffer, conn_req.getInputSize());
};

