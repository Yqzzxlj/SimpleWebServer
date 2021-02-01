#include "http/HttpServer.h"
#include "Util.h"
#include "log/Logging.h"
#include <getopt.h>

int main(int argc, char* argv[]) {

  // pares args
  int opt;
  const char* str = "l:";
  while ((opt = getopt(argc, argv, str)) != -1) {
    switch (opt) {
      case 'l': {
        if (strcmp(optarg, "DEBUG") == 0) {
          Logger::setLogLevel(Logger::DEBUG);
        } else if (strcmp(optarg, "TRACE") == 0) {
          Logger::setLogLevel(Logger::TRACE);
        } else if (strcmp(optarg, "ERROR") == 0) {
          Logger::setLogLevel(Logger::ERROR);
        }
        break;
      }
    }
  }

  handle_for_sigpipe();
  
  HttpServer httpServer(8080);
  httpServer.run();
}