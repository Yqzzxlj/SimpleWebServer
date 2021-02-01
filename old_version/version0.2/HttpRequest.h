#pragma once
#include <string>
#include <unordered_map>
namespace http {



class HttpRequest {
public:



  enum HTTP_VERSION {HTTP_10 = 0, HTTP_11, VERSION_NOT_SUPPORT};
  enum HTTP_METHOD {GET = 0, HEAD, METHOD_NOT_SUPPORT};
  enum HTTP_HEADER {Host = 0, User_Agent, Connection, Accept_Encoding,
                    Accept_Language, Accept, Cache_Control, Upgrade_Insecure_Requests};

  struct EnumClassHash {
    template <typename T>
    std::size_t operator()(T t) const {
      return static_cast<std::size_t>(t);
    }
  };

  HttpRequest(const std::string& _uri = "",
              HTTP_METHOD _method = METHOD_NOT_SUPPORT,
              HTTP_VERSION _version = VERSION_NOT_SUPPORT)
      : uri(_uri), method(_method), version(_version) {}

  ~HttpRequest(){}

  friend std::ostream& operator<<(std::ostream&, const HttpRequest&);
  friend std::ostream& operator<<(std::ostream&, const HTTP_METHOD&);
  friend std::ostream& operator<<(std::ostream&, const HTTP_VERSION&);


  HTTP_METHOD method;
  std::string uri;
  HTTP_VERSION version;

  static std::unordered_map<std::string, HTTP_HEADER> header_map;
  std::unordered_map<HTTP_HEADER, std::string, EnumClassHash> headers;

  std::string content;

};

std::ostream& operator<<(std::ostream&, const HttpRequest&);
std::ostream& operator<<(std::ostream&, const HttpRequest::HTTP_VERSION&);
std::ostream& operator<<(std::ostream&, const HttpRequest::HTTP_METHOD&);



} // namespace http