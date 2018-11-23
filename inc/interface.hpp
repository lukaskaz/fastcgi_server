#ifndef __INTERFACE_H__
#define __INTERFACE_H__

#include <mutex>
#include <condition_variable>


struct ipcData {
    ipcData(): request_nb(0) {}

    int32_t request_nb;
    std::string nick;
    std::string name;
};

template <typename T>
class ipcDataStream
{
public:
    void get(T& item) {
        std::unique_lock<std::mutex> lock(mutex);
        while (empty == true) {
            cond_var.wait(lock);
        }

        empty = true;
        item = data;
    }

    void put(T& item) {
        std::unique_lock<std::mutex> lock(mutex);
        data = item;
        empty = false;

        lock.unlock();
        cond_var.notify_one();
    }

    bool isEmpty(void) const { return empty; }

private:
    T data;
    bool empty;
    std::mutex mutex;
    std::condition_variable cond_var;
};

void serveHttpThread(ipcDataStream<ipcData> &);

#endif
