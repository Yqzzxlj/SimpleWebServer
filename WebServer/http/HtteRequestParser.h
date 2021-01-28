#pragma once

#include <string>

class Buffer;
class HttpRequest;

const char CR = '\r';
const char LF = '\n';
const char LINE_END = '\0';

class HttpRequestParser {
public:
 
  enum LINE_STATE {LINE_OK = 0, LINE_BAD, LINE_MORE};
  enum PARSE_STATE {PARSE_REQUESTLINE = 0, PARSE_HEADER, PARSE_BODY};
  enum HTTP_CODE {NO_REQUEST = 0, GET_REQUEST, HEAD_REQUEST, BAD_REQUEST, NOT_IMPLEMNTED, 
                  FORBIDDEN_REQUEST, INTERNAL_ERROR, CLOSED_CONNECTION};

  static LINE_STATE parse_line(Buffer& buffer);
  static HTTP_CODE parse_request_line(const std::string&, PARSE_STATE&, HttpRequest&);
  static HTTP_CODE parse_headers(const std::string&, PARSE_STATE&, HttpRequest&);
  static HTTP_CODE parse_body(const std::string&, HttpRequest&);
  static HTTP_CODE parse_content(Buffer& buffer, PARSE_STATE &parse_state, HttpRequest& request);
};