#pragma once

#include <string>
#include <curl/curl.h>
#include <iostream>
#include <functional>
#include <print> // C++23 for cleaner output

/**
 * @brief Simple HTTP client wrapper around libcurl.
 * Designed to fetch JSON data from SPARQL endpoints.
 */
class NetworkClient {
public:
    /**
     * @brief Initialize global CURL environment.
     * @warning Not thread-safe if multiple clients are created concurrently 
     * without external global init.
     */
    NetworkClient() {
        curl_global_init(CURL_GLOBAL_DEFAULT);
    }

    ~NetworkClient() {
        curl_global_cleanup();
    }

    /**
     * @brief Callback function for cURL to write received data into a string.
     * * @param contents Pointer to the data received.
     * @param size Size of one element.
     * @param nmemb Number of elements.
     * @param userp User pointer (destination std::string).
     * @return size_t Total bytes handled.
     */
    static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
        size_t totalSize = size * nmemb;
        auto* strBuffer = static_cast<std::string*>(userp);
        strBuffer->append(static_cast<char*>(contents), totalSize);
        return totalSize;
    }

    /**
     * @brief Executes a GET request to the specified URL.
     * * @param url The target SPARQL endpoint URL (already encoded with query).
     * @return std::string The raw response body.
     */
    [[nodiscard]] 
    std::string performGet(const std::string& url) {
        CURL* curl = curl_easy_init();
        std::string readBuffer;

        if (!curl) {
            std::println(stderr, "Error: Failed to initialize CURL.");
            return "";
        }

        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        
        // Set User-Agent to avoid being blocked by Wikidata/DBpedia
        curl_easy_setopt(curl, CURLOPT_USERAGENT, "SparqReflect/0.1.0");

        // Setup write callback
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

        // Accept JSON
        struct curl_slist* headers = nullptr;
        headers = curl_slist_append(headers, "Accept: application/sparql-results+json");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

        // Perform request
        CURLcode res = curl_easy_perform(curl);
        
        if (res != CURLE_OK) {
            std::println(stderr, "curl_easy_perform() failed: {}", curl_easy_strerror(res));
        }

        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
        
        return readBuffer;
    }
    
    /**
     * @brief Helper to URL-encode a query string.
     * * @param value The raw SPARQL query string.
     * @return std::string URL-encoded string.
     */
    [[nodiscard]]
    std::string urlEncode(const std::string& value) {
        CURL* curl = curl_easy_init();
        if (curl) {
            char* output = curl_easy_escape(curl, value.c_str(), static_cast<int>(value.length()));
            if (output) {
                std::string result(output);
                curl_free(output);
                curl_easy_cleanup(curl);
                return result;
            }
            curl_easy_cleanup(curl);
        }
        return "";
    }
};