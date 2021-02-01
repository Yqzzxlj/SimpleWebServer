#include "HttpResponse.h"

#include "../Buffer.h"


std::unordered_map<std::string, MimeType> mime_map = {
    {".html", "text/html"},
    {".xml", "text/xml"},
    {".xhtml", "application/xhtml+xml"},
    {".txt", "text/plain"},
    {".rtf", "application/rtf"},
    {".pdf", "application/pdf"},
    {".word", "application/msword"},
    {".png", "image/png"},
    {".gif", "image/gif"},
    {".jpg", "image/jpeg"},
    {".jpeg", "image/jpeg"},
    {".au", "audio/basic"},
    {".mpeg", "video/mpeg"},
    {".mpg", "video/mpeg"},
    {".avi", "video/x-msvideo"},
    {".gz", "application/x-gzip"},
    {".tar", "application/x-tar"},
    {".css", "text/css"},
    {"", "text/plain"},
    {"default", "text/plain"}
};

void HttpResponse::appendBuffer(std::string& buffer) const {
  if (version == HttpRequest::HTTP_11) {
    buffer += "HTTP/1.1 ";
  } else {
    buffer += "HTTP/1.0 ";
  }
  buffer += std::to_string(state_code) + " " + short_msg + "\r\n";

  for (auto it = headers.begin(); it != headers.end(); ++it) {
    buffer += it->first + ": " + it->second + "\r\n";
  }

  buffer += "Content-type: " + mime.type + "\r\n";

  if (close_connection) {
    buffer += "Connection: close\r\n";
  } else {
    buffer += "Connection: keep-alive\r\n";
  }
}