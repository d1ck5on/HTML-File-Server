#pragma once

#include <filesystem>
#include <istream>
#include <list>
#include <string>

class FileManager {
public:
    explicit FileManager(const std::string& root);

    uintmax_t Get(const std::string& path, std::unique_ptr<std::iostream>& out);

    void CreateFile(const std::string& path);
    void CreateDirectory(const std::string& path);

    void Put(const std::string& path, std::list<std::unique_ptr<std::iostream>>& str);

    void DeleteFile(const std::string& path);
    void DeleteDirectory(const std::string& path);

    
private:
    uintmax_t GetFile(const std::filesystem::directory_entry& file, std::unique_ptr<std::iostream>& out);
    uintmax_t GetDirectory(const std::filesystem::directory_entry& dir, std::unique_ptr<std::iostream>& out);

private:
    const std::string root_;
};