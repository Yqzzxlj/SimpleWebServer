#include "HttpRequest.h"

#include <iostream>

std::unordered_map<std::string, http::HttpRequest::HTTP_HEADER>
http::HttpRequest::header_map = {
    {"HOST",                      http::HttpRequest::Host},
    {"USER-AGENT",                http::HttpRequest::User_Agent},
    {"CONNECTION",                http::HttpRequest::Connection},
    {"ACCEPT-ENCODING",           http::HttpRequest::Accept_Encoding},
    {"ACCEPT-LANGUAGE",           http::HttpRequest::Accept_Language},
    {"ACCEPT",                    http::HttpRequest::Accept},
    {"CACHE-CONTROL",             http::HttpRequest::Cache_Control},
    {"UPGRADE-INSECURE-REQUESTS", http::HttpRequest::Upgrade_Insecure_Requests}
};

std::ostream& http::operator<<(std::ostream& os, const http::HttpRequest& http_request) {
  os << "method: "  << http_request.method  << std::endl;
  os << "uri: "     << http_request.uri     << std::endl;
  os << "version: " << http_request.version << std::endl;

  for (auto it = http_request.headers.begin(); it != http_request.headers.end(); ++it) {
    os << it->first << ": " << it->second << std::endl;
  }
  return os;
}

std::ostream& http::operator<<(std::ostream& os, const HttpRequest::HTTP_METHOD& method) {
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

std::ostream& http::operator<<(std::ostream& os, const HttpRequest::HTTP_VERSION& version) {
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
}