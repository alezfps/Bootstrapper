#include "Updater.h"
#include <iostream>
#include "nlohmann/json.hpp"


size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* userData) {
    size_t totalSize = size * nmemb;
    userData->append((char*)contents, totalSize);
    return totalSize;
}
bool ServerConnection(const std::string& url, std::string& response) {
    CURL* curl;
    CURLcode res;
    curl = curl_easy_init();
    if (!curl) {
        return false;
    }

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

    res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        curl_easy_cleanup(curl);
        return false;
    }

    curl_easy_cleanup(curl);
    return true;
}


int main(int argc, char* argv[]) {

    std::string updateUrl = "http://127.0.0.1:5000"; // Set your server IP
    std::string latestVersionUrl = updateUrl + "/latest_version";
    std::string currentVersion = "1.0.0";
    std::string response;

    Updater updater(updateUrl, currentVersion);

    std::cout << "Connecting..." << std::endl;
    if (ServerConnection(latestVersionUrl, response)) {
        std::cout << "Connected." << std::endl;

        try {
            auto json = nlohmann::json::parse(response);
            std::string latestVersion = json["version"];
            std::string fileName = json["file_name"];

            std::cout << "Current Version: v" << currentVersion << std::endl;

            if (latestVersion != currentVersion) {
                std::cout << "Update Available: v" << latestVersion << std::endl;
                if (updater.DownloadUpdate(updateUrl + "/update/" + fileName)) {
                    if (updater.ApplyUpdate(fileName)) {
                        std::cout << "Updated. Restart the program." << std::endl;
                    }
                }
            }
            else {
                std::cout << "Up to date." << std::endl;
            }
        }
        catch (const nlohmann::json::exception& e) {
            //std::cerr << "JSON parsing error: " << e.what() << std::endl; // Debug if you have any issue with server
        }
    }
    else {
        std::cerr << "Connection failed." << std::endl;
    }


    // Remove Sleep(-1); and replace with your own program.
    Sleep(-1);

    return 0;
}