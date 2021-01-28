#include "../Logging.h"
#include <thread>
#include <chrono>
#include <unistd.h>

int main() {
  LOG_WARN << std::this_thread::get_id();
  sleep(3);
  return 0;
}