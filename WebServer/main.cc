#include "http/HttpServer.h"
#include "Util.h"
#include "config.h"


#include <getopt.h>

int main(int argc, char* argv[]) {

  // pares args
  int opt;
  const char* str = "l:";
  while ((opt = getopt(argc, argv, str)) != -1) {
    switch (opt) {
      case 'l': {
        if (optarg == "DEBUG") {
          LOG_LEVEL = "DEBUG";
        } else if (optarg == "TRACE") {
          LOG_LEVEL = "TRACE";
        } else if (optarg == "ERROR") {
          LOG_LEVEL = "ERROR";
        }
        break;
      }
    }
  }

  handle_for_sigpipe();
  
  HttpServer httpServer(8080);
  httpServer.run();
}