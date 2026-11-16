#pragma once
#include <cstddef>
#include <map>
#include <stdexcept>
#include <string>
#include <string_view>
namespace lapislazuli::net {
enum class Method { Get, Head, Post, Put, Patch, Delete };
struct Request { Method method = Method::Get; std::string url; std::map<std::string,std::string> headers; std::string body; };
struct Response { long statusCode = 0; std::map<std::string,std::string> headers; std::string body; std::string effectiveUrl; std::string contentType; bool redirected = false; };
struct ClientConfig { long connectTimeoutMs=5000; long totalTimeoutMs=15000; long maxRedirects=5; std::size_t maxResponseBytes=8*1024*1024; bool allowInsecureHttp=false; bool verifyTlsPeer=true; };
class NetworkError : public std::runtime_error { public: using std::runtime_error::runtime_error; };
class HttpClient { public: explicit HttpClient(ClientConfig config={}); [[nodiscard]] Response perform(const Request& request) const; static void validateUrl(std::string_view url,bool allowInsecureHttp=false); static void validateHeader(std::string_view name,std::string_view value); private: ClientConfig config_; };
}
