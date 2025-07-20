#include "crow_all.h"
#include <fstream>
#include <unordered_set>
#include <iostream>
#include <regex>
#include <curl/curl.h>

// helper for curl
static size_t cb(void *contents, size_t size, size_t nmemb, void* userp) {
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

std::unordered_set<std::string> whitelist;

// simple host extractor
std::string extract_host(const std::string& url) {
    std::regex re(R"(https?://([^/]+))");
    std::smatch m;
    return (std::regex_search(url, m, re)) ? m[1].str() : "";
}

int main() {
    // load whitelist
    std::ifstream in("src/whitelist.txt");
    std::string line;
    while (std::getline(in, line)) {
        if (!line.empty() && line[0] != '#')
            whitelist.insert(line);
    }

    crow::SimpleApp app;

    CROW_ROUTE(app, "/").methods("GET"_method)
    ([](){
        return crow::response(200, "<html><body>"
            "<h2>12ft‑POST Proxy</h2>"
            "<form method='POST' action='/fetch'>"
            "URL: <input name='url' size=60><input type='submit'></form>"
            "</body></html>");
    });

    CROW_ROUTE(app, "/fetch").methods("POST"_method)
    ([](const crow::request& req){
        auto url = crow::query_string(req.body)["url"];
        if (url.empty())
            return crow::response(400, "Missing URL");

        auto host = extract_host(url);
        if (whitelist.find(host) == whitelist.end()) {
            // not allowed
            crow::response res(302);
            res.add_header("Location", "/");
            res.body = "Host not whitelisted. Paste URL in form.";
            return res;
        }

        CURL *curl = curl_easy_init();
        std::string html;
        if (!curl)
            return crow::response(500, "Failed init HTTP");

        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, cb);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &html);
        CURLcode code = curl_easy_perform(curl);
        curl_easy_cleanup(curl);

        if (code != CURLE_OK)
            return crow::response(502, "Upstream fetch error");

        return crow::response(200, html);
    });

    app.port(12000).multithreaded().run();
}
