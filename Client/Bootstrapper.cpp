#include "Updater.h"
#include <iostream>
#include <fstream>
#include "nlohmann/json.hpp"
#include <openssl/sha.h>
#include <iomanip>
#include <sstream>
#include <filesystem>

namespace fs = std::filesystem;

std::string calculateSHA256(const std::string& filePath) {
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_CTX sha256;
    SHA256_Init(&sha256);

    std::ifstream file(filePath, std::ifstream::binary);
    if (!file.is_open()) {
        return "";
    }

    char buffer[8192];
    while (file.good()) {
        file.read(buffer, sizeof(buffer));
        SHA256_Update(&sha256, buffer, file.gcount());
    }
    file.close();

    SHA256_Final(hash, &sha256);

    std::stringstream ss;
    for (int i = 0; i < SHA256_DIGEST_LENGTH; ++i) {
        ss << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];
    }

    return ss.str();
}

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
            std::string fileHash = json["file_hash"];

            std::cout << "Current Version: v" << currentVersion << std::endl;

            if (latestVersion != currentVersion) {
                std::cout << "Update Available: v" << latestVersion << std::endl;

                std::string downloadPath = "Updated.exe";

                if (updater.DownloadUpdate(updateUrl + "/update/" + fileName)) {
                    std::string computedHash = calculateSHA256(downloadPath);

                    if (computedHash == fileHash) {
                        std::cout << "Integrity check passed." << std::endl;

                        std::string newFilePath = updater.GetCurrentExecutablePath();

                        if (updater.ApplyUpdate(downloadPath)) {
                            fs::rename(downloadPath, newFilePath);
                            std::cout << "Updated successfully. Restart the program." << std::endl;
                        }
                        else {
                            std::cerr << "Failed to apply the update." << std::endl;
                        }
                    }
                    else {
                        std::cerr << "Integrity check failed!" << std::endl;
                    }
                }
                else {
                    std::cerr << "Failed to download the update." << std::endl;
                }
            }
            else {
                std::cout << "Up to date." << std::endl;
            }
        }
        catch (const nlohmann::json::exception& e) {
            // std::cerr << "JSON parsing error: " << e.what() << std::endl; // Debug if you have any issue with server
        }
    }
    else {
        std::cerr << "Connection failed." << std::endl;
    }

    // Replace Sleep(-1); with your own program continuation logic.
    Sleep(-1);

    return 0;
}
