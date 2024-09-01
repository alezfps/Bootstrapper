#ifndef UPDATER_H
#define UPDATER_H

#include <string>
#include "curl/curl.h"

class Updater {
public:
    Updater(const std::string& updateUrl, const std::string& currentVersion);
    bool CheckForUpdate();
    bool DownloadUpdate(const std::string& fileUrl);
    bool ApplyUpdate(const std::string& newFilePath);

private:
    std::string GetLatestVersion() const;
    std::string updateUrl;
    std::string currentVersion;
    std::string latestVersion;
    std::string latestFileName;
    std::string GetCurrentExecutablePath() const;
    std::string GetTempDirectory() const;
};

#endif