#include "HttpRequest.h"

#include <iostream>

std::unordered_map<std::string, HttpRequest::HTTP_HEADER>
HttpRequest::header_map = {
    {"HOST",                      HttpRequest::Host},
    {"USER-AGENT",                HttpRequest::User_Agent},
    {"CONNECTION",                HttpRequest::Connection},
    {"ACCEPT-ENCODING",           HttpRequest::Accept_Encoding},
    {"ACCEPT-LANGUAGE",           HttpRequest::Accept_Language},
    {"ACCEPT",                    HttpRequest::Accept},
    {"CACHE-CONTROL",             HttpRequest::Cache_Control},
    {"UPGRADE-INSECURE-REQUESTS", HttpRequest::Upgrade_Insecure_Requests}
};

std::ostream& operator<<(std::ostream& os, const HttpRequest& http_request) {
  os << "method: "  << http_request.method  << std::endl;
  os << "uri: "     << http_request.uri     << std::endl;
  os << "version: " << http_request.version << std::endl;

  for (auto it = http_request.headers.begin(); it != http_request.headers.end(); ++it) {
    os << it->first << ": " << it->second << std::endl;
  }
  return os;
}

std::ostream& operator<<(std::ostream& os, const HttpRequest::HTTP_METHOD& method) {
  switch(method) {
    case (HttpRequest::HEAD): {
      os << "HEAD";
    }
    case (HttpRequest::GET): {
      os << "GET";
    }
    default: {
      os << "METHOD_NOT_SUPPORT";
    }

  }
  return os;
}

std::ostream& operator<<(std::ostream& os, const HttpRequest::HTTP_VERSION& version) {
  switch(version) {
    case (HttpRequest::HTTP_10): {
      os << "HTTP/1.0";
    }
    case (HttpRequest::HTTP_11): {
      os << "HTTP/1.1";
    }
    default: {
      os << "VERSION_NOT_SUPPORT";
    }
  }
  return os;
}