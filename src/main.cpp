#include <glog/logging.h>
#include <glog/raw_logging.h>
#include <http_server.h>
#include <utils/input_parser.h>

#include <cstdlib>
#include <string>

std::string InitArg(InputParser& input, const std::string& name, 
                    const std::string& env_name, const std::string& default_value) {
    if (input.cmdOptionExists(name)) {
        return input.getCmdOption(name);
    }

    if (const char* env_host = std::getenv(env_name.c_str())) {
        return env_host;
    }

    return default_value;                     
}

int main(int argc, char** argv) {
    InputParser input (argc, argv);

    std::string host = InitArg(input, "--host", "SERVER_HOST", "0.0.0.0");
    int port = std::stoi(InitArg(input, "--port", "SERVER_PORT", "8080"));
    std::string work_dir = InitArg(input, "--working-directory", "SERVER_WORKING_DIRECTORY", "");
    std::string domain = InitArg(input, "--server-domain", "SERVER_DOMAIN", "");

    if (work_dir == "") {
        exit(1);
    }

    FLAGS_logtostderr = 1;
    FLAGS_stderrthreshold = 3;
    google::InitGoogleLogging(argv[0]);

    HttpServer server(host, port, work_dir, "Simple Http 1.1 File Server");

    server.Start();
}