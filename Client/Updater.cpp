#include "Updater.h"
#include <iostream>
#include <fstream>
#include <cstdio>
#include <thread>
#include <filesystem>

Updater::Updater(const std::string& updateUrl, const std::string& currentVersion)
    : updateUrl(updateUrl), currentVersion(currentVersion) {}

bool Updater::CheckForUpdate() {
    CURL* curl = curl_easy_init();
    if (!curl) return false;

    std::string url = updateUrl + "/latest_version";
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, [](char* ptr, size_t size, size_t nmemb, std::string* data) {
        data->append(ptr, size * nmemb);
        return size * nmemb;
        });

    std::string response;
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

    CURLcode res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK) return false;

    size_t delimiterPos = response.find(':');
    if (delimiterPos == std::string::npos) return false;

    latestVersion = response.substr(0, delimiterPos);
    latestFileName = response.substr(delimiterPos + 1);

    return latestVersion != currentVersion;
}

bool Updater::DownloadUpdate(const std::string& fileUrl) {
    CURL* curl = curl_easy_init();
    if (!curl) return false;

    std::string outputFile = "Updated.exe";
    FILE* fp = fopen(outputFile.c_str(), "wb");
    if (!fp) return false;

    curl_easy_setopt(curl, CURLOPT_URL, fileUrl.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);

    CURLcode res = curl_easy_perform(curl);
    fclose(fp);
    curl_easy_cleanup(curl);

    return res == CURLE_OK;
}
namespace fs = std::filesystem;

std::string Updater::GetCurrentExecutablePath() const {
    char buffer[MAX_PATH];
    if (GetModuleFileNameA(NULL, buffer, MAX_PATH) == 0) {
        return "";
    }
    return std::string(buffer);
}
std::string Updater::GetTempDirectory() const {
    char tempPath[MAX_PATH];
    if (GetTempPath(MAX_PATH, tempPath)) {
        return std::string(tempPath);
    }
    else {
        return "";
    }
}
bool Updater::ApplyUpdate(const std::string& newFilePath) {
    std::string currentExe = GetCurrentExecutablePath();

    try {
        if (currentExe.empty()) {
            return false;
        }
        if (!fs::exists(newFilePath)) {
            return false;
        }
        if (fs::exists(currentExe)) {
            fs::path oldExePath = currentExe;
            oldExePath += ".old";
            std::string tempDir = GetTempDirectory();
            if (tempDir.empty()) {
                return false;
            }

            fs::path tempOldExePath = fs::path(tempDir) / oldExePath.filename();
            fs::rename(currentExe, tempOldExePath);
        }
        fs::path updatedExePath = fs::path(GetCurrentExecutablePath()).parent_path() / "Updated.exe";
        fs::path bootstrapExePath = newFilePath;
        if (fs::exists(updatedExePath)) {
            fs::rename(updatedExePath, bootstrapExePath);
        }
        else {
            return false;
        }

        return true;
    }
    catch (const fs::filesystem_error& e) {
        return false;
    }
    catch (const std::exception& e) {
        return false;
    }
}

std::string Updater::GetLatestVersion() const {
    return latestVersion;
}
