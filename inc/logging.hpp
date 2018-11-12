#ifndef __LOGGING_H__
#define __LOGGING_H__

#include <mutex>

#define DEBUG_MODE  0

class Log
{
public:
    Log(): Log(true, false, false, false) {};
    Log(bool simple, bool error, bool info, bool debug): SHOW_SIMPLE_LOGS(simple),
        SHOW_ERROR_LOGS(error), SHOW_INFO_LOGS(info), SHOW_DEBUG_LOGS(debug) {};

    enum log_t{ LOG_SIMPLE = 0, LOG_ERROR, LOG_INFO, LOG_DEBUG };

    void log(const std::string &txt) const { log_simple("", txt); };
    void log(log_t type, const std::string &, const std::string &);

private:
    const bool SHOW_SIMPLE_LOGS;
    const bool SHOW_ERROR_LOGS;
    const bool SHOW_INFO_LOGS;
    const bool SHOW_DEBUG_LOGS;
    static std::mutex log_mtx;

    void log_simple(const std::string &source, const std::string &txt) const;
    void log_error(const std::string &source, const std::string &txt)  const;
    void log_info(const std::string &source, const std::string &txt)   const;
    void log_debug(const std::string &source, const std::string &txt)  const;
};

#define FUNC_ID(class_id)       (class_id) + std::string(__func__)
#define LOG_ERROR(src, txt)     log(Log::LOG_ERROR, src, txt)

#if DEBUG_MODE == 1
#define LOG_INFO(src, txt)      log(Log::LOG_INFO, src, txt)
#define LOG_DEBUG(src, txt)     log(Log::LOG_DEBUG, src, txt)
#else
#define LOG_INFO(src, txt)
#define LOG_DEBUG(src, txt)
#endif

#endif
