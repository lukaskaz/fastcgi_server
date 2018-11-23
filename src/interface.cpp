#include "server.hpp"
#include "responder.hpp"
#include "interface.hpp"


class customResponder: public responder
{
public:
    customResponder(ipcDataStream<ipcData> &ipc):
        request_nb(0), custom_stream(input_data::TYPE_STREAM, stream),
        custom_params(input_data::TYPE_PARAM, params), ipc_data_stream(ipc) {}
    void createResponse(void)
    {
        std::pair<std::string, std::string> param;

        collectData();
        collectParams();

        writeHeader("Content-Type: text/html\n\n");
        writeBody(
            "<!DOCTYPE html>\n"
            "<html>\n"
            "   <head>\n"
            "       <title>User WebIF</title>\n"
            "   </head>\n"
            "   <body>\n"
            "   <form id=\"interface\" method=\"POST\">\n"
        );

        while(custom_params.listGetEntry(param) == 0) {
            writeBody("       <p>"+param.first+param.second+"</p>\n");
        }

        while(custom_stream.listGetEntry(param) == 0) {
            writeBody("       <p>"+param.first+param.second+"</p>\n");
        }

        writeBody(
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
            "</html>"
        );

        storeData();
    };

private:
    int32_t request_nb;
    customData custom_stream;
    customData custom_params;
    ipcData ipc_data;
    ipcDataStream<ipcData> &ipc_data_stream;

    void collectData(void) {
        custom_stream.mapBuild();

        ipc_data.request_nb = ++request_nb;
        ipc_data.nick = custom_stream.mapGetValue("nick", "N/A");
        ipc_data.name = custom_stream.mapGetValue("name", "N/A");

        custom_stream.listAddEntry("Name: ", ipc_data.name);
        custom_stream.listAddEntry("Nick: ", ipc_data.nick);
        custom_stream.listAddEntry("Requests: ", std::to_string(request_nb));
        custom_stream.listReset();
    }

    void collectParams(void) {
        custom_params.mapBuild();
        custom_params.listAddEntry("Server name: ",  custom_params.mapGetValue("SERVER_NAME", "N/A"));
        custom_params.listAddEntry("Server admin: ", custom_params.mapGetValue("SERVER_ADMIN", "N/A"));
        custom_params.listAddEntry("Server address: ", custom_params.mapGetValue("SERVER_ADDR", "N/A"));
        custom_params.listAddEntry("Server port: ", custom_params.mapGetValue("SERVER_PORT", "N/A"));
        custom_params.listAddEntry("Client address: ", custom_params.mapGetValue("REMOTE_ADDR", "N/A"));
        custom_params.listAddEntry("Server software: ", custom_params.mapGetValue("SERVER_SOFTWARE", "N/A"));
        custom_params.listAddEntry("Server script: ", custom_params.mapGetValue("SCRIPT_NAME", "N/A"));
        custom_params.listAddEntry( "Request type: ", custom_params.mapGetValue("REQUEST_METHOD", "N/A"));
        custom_params.listReset();
    }

    void storeData(void) {
        ipc_data_stream.put(ipc_data);
    }
};

void serveHttpThread(ipcDataStream<ipcData> &ipc_data_stream)
{
    customResponder custom(ipc_data_stream);
    server fcgiServer(custom);
    fcgiServer.handleRequests();
}
