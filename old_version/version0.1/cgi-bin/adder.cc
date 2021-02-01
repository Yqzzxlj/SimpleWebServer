#include "../util.h"
#include <string>

int main(void) {

  int n1 = 0, n2 = 0;
  char* buf = getenv("QUERY_STRING");
  if (buf != NULL) {
    std::string args(buf);
    int idx = args.find('&');
    std::string arg1 = args.substr(0, idx);
    std::string arg2 = args.substr(idx + 1);
    n1 = std::stoi(arg1);
    n2 = std::stoi(arg2);
  }

  /* Make the response body */
  std::string content;
  content += "Welcome to add.com: ";
  content += "THE Internet addition portal.\r\n<p>";
  content += "The answer is: ";
  content += std::to_string(n1) + " + " + std::to_string(n2);
  content += " = " + std::to_string(n1 + n2) + "\r\n<p>";
  content += "Thanks for visiting!\r\n";



  printf("Content-length: %d\r\n", (int)content.size());
  printf("Content-type: text/html\r\n\r\n");
  printf("%s", content.c_str());
  fflush(stdout);
  exit(0);
}