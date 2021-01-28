#include "http/HttpServer.h"

int main(int argc, const char** argv) {
  HttpServer httpServer(8080);
  httpServer.run();
}