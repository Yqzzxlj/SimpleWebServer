#include "HtteRequestParser.h"

#include "HttpRequest.h"
#include "util.h"

#include <iostream>
#include <string>
#include <sstream>
#include <algorithm>

http::HttpRequestParser::LINE_STATE
http::HttpRequestParser::parse_line(util::Buffer& buffer) {
  if (buffer.findCRLF() == NULL) return LINE_MORE;
  return LINE_OK;
}


http::HttpRequestParser::HTTP_CODE
http::HttpRequestParser::parse_request_line(const std::string& request_line,
                                            PARSE_STATE& parse_state,
                                            http::HttpRequest& request) {
  std::stringstream ss(request_line);
  std::string method_str;
  std::string version_str;
  std::string uri_str;
  ss >> method_str >> uri_str >> version_str;

  if (method_str.empty()) {
    return http::HttpRequestParser::BAD_REQUEST;
  } else if (method_str == "GET") {
    request.method = http::HttpRequest::GET;
  } else if (method_str == "HEAD") {
    request.method = http::HttpRequest::HEAD;
  } else {
    return http::HttpRequestParser::NOT_IMPLEMNTED;
  }

  if (version_str == "HTTP/1.1") {
    request.version = http::HttpRequest::HTTP_11;
  } else if (version_str == "HTTP/1.0") {
    request.version = http::HttpRequest::HTTP_10;
  } else {
    return BAD_REQUEST;
  }

  if (uri_str.empty() || uri_str[0] != '/') {
    return BAD_REQUEST;
  }
  request.uri = uri_str;
  parse_state = PARSE_HEADER;
  return NO_REQUEST;
}

http::HttpRequestParser::HTTP_CODE
http::HttpRequestParser::parse_headers(const std::string& header_line,
                                       PARSE_STATE& parse_state,
                                       HttpRequest& request) {
  if (header_line == "\r\n") {
    switch (request.method) {
      case HttpRequest::GET:
        return GET_REQUEST;
      case HttpRequest::HEAD:
        return HEAD_REQUEST;
      default:
        parse_state = PARSE_BODY;
        return NO_REQUEST;
    }
  }

  ssize_t pos = header_line.find(':');
  std::string header_name = header_line.substr(0, pos);
  std::string header_val = header_line.substr(pos + 1);
  header_name = util::trim(header_name);
  std::transform(header_name.begin(), header_name.end(), header_name.begin(), ::toupper);
  header_val = util::trim(header_val);

  if (header_name == "UPGRADE-INSECURE-REQUESTS") {
    return NO_REQUEST;
  }

  auto it = HttpRequest::header_map.find(header_name);
  if (it != HttpRequest::header_map.end()) {
    request.headers.insert(std::make_pair(it->second, header_val));
  } else {
    std::cout << "Header not support: "
              << header_name << ": " 
              << header_val << std::endl;
  }
  return NO_REQUEST;
}

http::HttpRequestParser::HTTP_CODE
http::HttpRequestParser::parse_body(const std::string& body, HttpRequest& request) {
  request.content = body;
  return GET_REQUEST;
}

http::HttpRequestParser::HTTP_CODE
http::HttpRequestParser::parse_content(util::Buffer& buffer,
                                       http::HttpRequestParser::PARSE_STATE &parse_state,
                                       HttpRequest &request) {
  LINE_STATE line_state = LINE_OK;
  HTTP_CODE retcode = NO_REQUEST;

  while ((line_state = parse_line(buffer)) == LINE_OK) {
    std::string line = buffer.retrieve_line();

    switch (parse_state) {
      case PARSE_REQUESTLINE: {
        retcode = parse_request_line(line, parse_state, request);
        if (retcode == BAD_REQUEST) {
          return BAD_REQUEST;
        }
        break;
      }
      case PARSE_HEADER: {
        retcode = parse_headers(line, parse_state, request);
        if (retcode == BAD_REQUEST) {
          return BAD_REQUEST;
        } else if (retcode == GET_REQUEST) {
          return GET_REQUEST;
        }
        break;
      }
      case PARSE_BODY: {
        retcode = parse_body(line, request);
        if (retcode == GET_REQUEST) {
          return GET_REQUEST;
        }
        return BAD_REQUEST;
      }
      default:
        return INTERNAL_ERROR;
    }
  }
  
  if (line_state == LINE_MORE) {
    return NO_REQUEST;
  } else {
    return BAD_REQUEST;
  }

}
