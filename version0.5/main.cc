#include "Server.h"

int main(int argc, const char** argv) {
  net::HttpServer httpServer(8080);
  httpServer.run();
}