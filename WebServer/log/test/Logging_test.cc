#include "../Logging.h"
#include <thread>
#include <chrono>
#include <unistd.h>

int main() {
  for (int i = 0; i < 100; ++i) {
    LOG_INFO << i;
    LOG_WARN << i + 1;
  }
  sleep(3);
  return 0;
}