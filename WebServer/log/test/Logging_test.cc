#include "../Logging.h"
#include <thread>
#include <chrono>
#include <unistd.h>

int main() {
  for (int i = 0; i < 100; ++i) {
    LOG << i;
  }
  sleep(3);
  return 0;
}