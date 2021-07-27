#ifndef IP_CALL
#define IP_CALL
#define IP_END_POINT "https://ipecho.net/plain"

#include <curl/curl.h>
#include <string>

namespace turbobeep {
namespace socket_utils {

/**
 * Callback to curl function
 * @see https://curl.se/libcurl/c/CURLOPT_WRITEFUNCTION.html
 */
size_t WriteCallback(char *contents, size_t size, size_t nmemb, void *userp) {
  ((std::string *)userp)->append((char *)contents, size * nmemb);
  return size * nmemb;
}

/**
 * Gets user's ip address from given endpoint call
 *
 * @param[in, out] httpCode the response code from http request
 * @param[in, out] readBuffer the response ip address from http request
 */
inline void getIpAddress(long *httpCode, std::string *readBuffer) {
  std::string url = IP_END_POINT;

  curl_global_init(CURL_GLOBAL_ALL);
  CURL *curl = curl_easy_init();
  curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, socket_utils::WriteCallback);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, readBuffer);
  curl_easy_perform(curl);
  curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, httpCode);
  curl_easy_cleanup(curl);
  curl_global_cleanup();
}

} // namespace socket_utils
} // namespace turbobeep

#endif