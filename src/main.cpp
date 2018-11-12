#include <string>
#include <thread>
#include <unistd.h>

#include "server.hpp"
#include "responder.hpp"
#include "logging.hpp"
#include "support.hpp"


int32_t request_nb = 0;

class customResponder: public responder
{
public:
    void createResponse(void)
    {
        request_nb++;

        std::string header("Content-Type: text/html\n\n");
        std::string body =
            "<!DOCTYPE html>\n"
            "<html>\n"
            "   <head>\n"
            "       <title>User WebIF</title>\n"
            "   </head>\n"
            "   <body>\n"
            "   <form id=\"interface\" method=\"POST\">\n"
            "       <p>Server name: "+getParamValue("SERVER_NAME", "N/A")+"</p>\n"
            "       <p>Server administrator: "+getParamValue("SERVER_ADMIN", "N/A")+"</p><br>\n"
            "       <p>Host server address: "+getParamValue("SERVER_ADDR", "N/A")+"</p>\n"
            "       <p>Host server port: "+getParamValue("SERVER_PORT", "N/A")+"</p>\n"
            "       <p>Client server address: "+getParamValue("REMOTE_ADDR", "N/A")+"</p>\n"
            "       <p>Server software: "+getParamValue("SERVER_SOFTWARE", "N/A")+"</p>\n"
            "       <p>Server script: "+getParamValue("SCRIPT_NAME", "N/A")+"</p>\n"
            "       <p>Request: "+getParamValue("REQUEST_METHOD", "N/A")+"</p>\n"
            "       <p>Username: "+getStreamValue("name", "N/A")+"\n"
            "       <p>Usernick: "+getStreamValue("nick", "N/A")+"\n"
            "       <p>Submit: "+getStreamValue("submit", "N/A")+"</p><br>\n"
            "       <p>Request number: "+std::to_string(request_nb)+"</p><br>\n"
            "       <table style=\"width:30%\">\n"
            "       <tr>\n"
            "           <td>Please insert your name: </td>\n"
            "           <td><input type=\"text\" name=\"name\" value=\"\" style=\"height:50px; width:200px\" ></td>\n"
            "       </tr><tr>\n"
            "           <td>Please insert your nick: </td>\n"
            "           <td><input type=\"text\" name=\"nick\" value=\"\" style=\"height:50px; width:200px\" ></td>\n"
            "       </tr><tr>\n"
            "           <td><br><br><input type=\"submit\" name=\"submit\" value=\"Confirm!\" style=\"height:40px; width:150px\" ></td>\n"
            "       </tr>\n"
            "       </table>\n"
            "    </form>\n"
            "    <script>\n"
            "    (function(){\n"
            "        var enter_code = 13;\n"
            "        var blocked_elems = document.getElementsByTagName(\'input\');\n"
            "        for(var i = 0; i < blocked_elems.length; i++) {\n"
            "            if(blocked_elems[i].getAttribute(\'type\') == \'text\') {\n"
            "                blocked_elems[i].addEventListener(\'keypress\',function(event) {\n"
            "                    if(event.keyCode == enter_code) {\n"
            "                        event.preventDefault();\n"
            "                    }\n"
            "                });\n"
            "            }\n"
            "        }\n"
            "    }());\n"
            "    </script>\n"
            "    </body>\n"
            "</html>";

        writeHeader(header);
        writeBody(body);
    };

private:
    //int32_t request_nb;
};


static bool exit_service = false;

void serveHttpThread(void)
{
    customResponder custom;
    server fcgiServer(custom);
    fcgiServer.handleRequests();
}

// ToDo: Implement signal handler
#include <signal.h>

void signal_handler(int sig)
{
    Log trace;

    trace.log("Received signal: " + CONV_NUM_GET_STR(int32_t, hex, sig));
    exit_service = true;
}

int main(int argc, char* argv[])
{
    Log trace;

    signal(SIGINT, signal_handler);

    std::thread serverThread(serveHttpThread);
    serverThread.detach();


    while(exit_service == false) {
        trace.log("Waiting for http requests, already received " +
            CONV_NUM_GET_STR(int32_t, dec, request_nb) + "requests");
        sleep(1);
    }

    trace.log("Exitting!!");

    return 0;
}
