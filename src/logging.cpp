#include <iostream>
#include "logging.hpp"

typedef void (Log::*log_fn_t)(const std::string &, const std::string &) const;


std::mutex Log::log_mtx;

void Log::log(log_t type, const std::string &source, const std::string &txt)
{
    static log_fn_t log_fn[] =
    {
        [LOG_SIMPLE] = &Log::log_simple,
        [LOG_ERROR]  = &Log::log_error,
        [LOG_INFO]   = &Log::log_info,
        [LOG_DEBUG]  = &Log::log_debug
    };
    static const uint8_t log_fn_size = sizeof(log_fn)/sizeof(*log_fn);

    if(type < log_fn_size && log_fn[type] != NULL) {
        std::unique_lock<std::mutex> lock(log_mtx);
        (this->*log_fn[type])(source, txt);
        lock.unlock();
    }
}

void Log::log_simple(const std::string &source, const std::string &txt) const
{
    if(SHOW_SIMPLE_LOGS == true) {
        std::cout<<"[LOG] "<<txt<<std::endl;
    }
}

void Log::log_error(const std::string &source, const std::string &txt) const
{
    if(SHOW_ERROR_LOGS == true) {
        std::cerr<<"[ERROR]["<<source<<"] "<<txt<<std::endl;
    }
}

void Log::log_info(const std::string &source, const std::string &txt) const
{
    if(SHOW_INFO_LOGS == true) {
        std::cerr<<"[INFO]["<<source<<"] "<<txt<<std::endl;
    }
}

void Log::log_debug(const std::string &source, const std::string &txt) const
{
    if(SHOW_DEBUG_LOGS == true) {
        std::cerr<<"[DEBUG]["<<source<<"] "<<txt<<std::endl;
    }
}

