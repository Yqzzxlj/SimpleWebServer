#pragma once
#include "HttpRequest.h"

#include <string>
#include <unordered_map>

struct MimeType {
  MimeType(const std::string& str) : type(str) {}
  MimeType(const char *str): type(str) {}
  
  std::string type;
};

extern std::unordered_map<std::string, MimeType> mime_map;

class HttpResponse {
public:
  enum HttpStatusCode {
    Unknow,
    k200Ok = 200,
    k403forbiden = 403,
    k404NotFound = 404
  };

  explicit HttpResponse(bool close = false) : close_connection(close),
                                      version(HttpRequest::HTTP_11),
                                      state_code(Unknow),
                                      mime("text/html") {}
  
  void appendBuffer(std::string&) const;

  HttpRequest::HTTP_VERSION version;
  HttpStatusCode state_code;
  std::string short_msg;

  std::unordered_map<std::string, std::string> headers;

  std::string filepath;
  MimeType mime;

  std::string content;
  int content_length;

  bool close_connection;
};