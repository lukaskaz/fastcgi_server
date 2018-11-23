#include <string>
#include <thread>
#include <functional>
#include <unistd.h>
#include <signal.h>

#include "logging.hpp"
#include "support.hpp"
#include "interface.hpp"


static bool exit_service = false;

// ToDo: Implement signal handler
void signal_handler(int sig)
{
    Log trace;

    trace.log("Received signal: " + CONV_NUM_GET_STR(int32_t, hex, sig));
    exit_service = true;
}

int main(int argc, char* argv[])
{
    Log trace;
    ipcData data;
    ipcDataStream<ipcData> ipc_data_stream;

    signal(SIGINT, signal_handler);

    std::thread serverThread(serveHttpThread, std::ref(ipc_data_stream));
    serverThread.detach();


    while(exit_service == false) {
        if(ipc_data_stream.isEmpty() == false) {
            ipc_data_stream.get(data);
        }
        trace.log("Waiting for http requests, already received " +
            std::to_string(data.request_nb) + " requests");
        if(data.name.size() > 0 && data.nick.size() > 0) {
            trace.log("Last user name: " + data.name + " (nick: " + data.nick + ")");
        }
        sleep(1);
    }

    trace.log("Exitting!!");

    return 0;
}
