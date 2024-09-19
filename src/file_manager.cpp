#include <cerrno>
#include <cstdint>
#include <cstring>
#include <file_manager.h>
#include <filesystem>
#include <glog/raw_logging.h>

#include <fstream>
#include <iostream>
#include <istream>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <stdio.h>
#include <string>

FileManager::FileManager(const std::string& root) : root_(root){
    try {
        std::filesystem::directory_entry dir(root_);
        if (!dir.is_directory()) {
            RAW_LOG_FATAL("File manager init error: %s is not directory", root_.c_str());
        }
    } catch (std::filesystem::filesystem_error& err) {
        RAW_LOG_FATAL("File manager init error: %s", err.what());
    }
}

uintmax_t FileManager::Get(const std::string& path, std::unique_ptr<std::iostream>& out) {
    try {
        std::filesystem::path abs_path(root_ + path);

        std::filesystem::directory_entry file(abs_path);

        if (file.is_directory()) {
            return GetDirectory(file, out);
        }

        if (file.is_regular_file()) {
            return GetFile(file, out);
        }

        std::ostringstream msg;
        msg << "File " << file.path() << " not found.";
        throw std::runtime_error(msg.str());


    } catch (std::filesystem::filesystem_error& err) {
        std::ostringstream msg;
        msg << "Creating directory entry error: " << err.what();
        throw std::runtime_error(msg.str());
    }
}

uintmax_t FileManager::GetFile(const std::filesystem::directory_entry& file, std::unique_ptr<std::iostream>& out) {
    std::unique_ptr<std::fstream> ifs = std::make_unique<std::fstream> (file.path(), std::iostream::in | std::iostream::binary);

    if (!ifs->is_open()) {
        std::ostringstream msg;
        msg << "Ifstream init error for reading file: " << file.path();
        throw std::runtime_error(msg.str());
    }

    ifs->seekg(0, std::ios::end);
    std::ifstream::pos_type filesize = ifs->tellg();
    ifs->seekg(0, std::ios::beg);

    out.reset(ifs.release());

    RAW_LOG_INFO("Successfull file open: %s", file.path().c_str());

    return std::filesystem::file_size(file.path());
}

uintmax_t FileManager::GetDirectory(const std::filesystem::directory_entry& dir, std::unique_ptr<std::iostream>& out) {
    std::ostringstream command;
    command << "ls -lA --time-style=\"+%Y-%b-%d %H:%M:%S\" " << dir.path();
    FILE* fp = popen(command.str().c_str(), "r");
    
    if (fp == NULL) {
        std::ostringstream msg;
        msg << "ls -lA error for directory: " << dir;
        throw std::runtime_error(msg.str());
    }

    std::unique_ptr<std::stringstream> list_of_files = std::make_unique<std::stringstream>();
    int path_max = 100;
    char path[path_max];
    while (fgets(path, path_max, fp) != NULL) {
        *list_of_files << path;
    }

    size_t size = list_of_files->str().size();

    out.reset(list_of_files.release());

    RAW_LOG_INFO("Successfull directory reading: %s", dir.path().c_str());
    return size;
}

void FileManager::CreateFile(const std::string& path) {
    try {
        std::filesystem::path abs_path(root_ + path);
        std::filesystem::directory_entry dir_entry(abs_path);

        if (dir_entry.exists()) {
            std::ostringstream msg;
            msg << "Error creating file " << dir_entry.path() << ". File exists.";
            throw std::runtime_error(msg.str());
        }

        std::fstream file;
        file.open(abs_path, std::ios::out);

        if (!file.good()) {
            std::ostringstream msg;
            msg << "File create error: " << abs_path.filename();
            throw std::runtime_error(msg.str());
        }

        RAW_LOG_INFO("Successfull file creating: %s", dir_entry.path().c_str());

    } catch (std::filesystem::filesystem_error& err) {
        std::ostringstream msg;
        msg << "Creating directory entry error: " << err.what();
        throw std::runtime_error(msg.str());
    }
}

void FileManager::CreateDirectory(const std::string& path) {
    try {
        std::filesystem::path abs_path(root_ + path);
        std::filesystem::directory_entry dir_entry(abs_path);

        if (dir_entry.exists()) {
            std::ostringstream msg;
            msg << "Error creating directory " << dir_entry.path() << ". File exists.";
            throw std::runtime_error(msg.str());
        }

        try {
            std::filesystem::create_directory(abs_path);
        } catch (std::filesystem::filesystem_error& err) {
            std::ostringstream msg;
            msg << "Creating directory error:" << err.what();
            throw std::runtime_error(msg.str());
        }

        RAW_LOG_INFO("Successfull directory creating: %s", dir_entry.path().c_str());

    } catch (std::filesystem::filesystem_error& err) {
        std::ostringstream msg;
        msg << "Creating directory entry error: " << err.what();
        throw std::runtime_error(msg.str());
    }
}

void FileManager::Put(const std::string& path, std::list<std::unique_ptr<std::iostream>>& str) {
    try {
        std::filesystem::path abs_path(root_ + path);
        std::filesystem::directory_entry dir_entry(abs_path);

        if (!dir_entry.is_regular_file()) {
            throw std::runtime_error("File does not exist");
        }

        std::fstream file;
        file.open(abs_path.string(), std::ios::out | std::ios::binary);

        if (!file.good()) {
            std::ostringstream msg;
            msg << "File open error: " << abs_path.filename();
            throw std::runtime_error(msg.str());
        }

        for (auto& stream : str) {
            std::stringstream buff;

            buff << stream->rdbuf();

            file << buff.str();
        }

        RAW_LOG_INFO("Successfull put in file: %s", dir_entry.path().c_str());

    } catch (std::filesystem::filesystem_error& err) {
        std::ostringstream msg;
        msg << "Creating directory entry error: " << err.what();
        throw std::runtime_error(msg.str());
    }
} 

void FileManager::DeleteFile(const std::string& path) {
    try {
        std::filesystem::path abs_path(root_ + path);
        std::filesystem::directory_entry dir_entry(abs_path);

        if (!dir_entry.is_regular_file()) {
            std::ostringstream msg;
            msg << "An attempt to delete an object that is not a file: " << abs_path.filename();
            throw std::runtime_error(msg.str());
        }

        int res = std::remove(abs_path.c_str());

        if (res != 0) {
            int err = errno;
            std::ostringstream msg;
            msg << "File deletion error. File: " << dir_entry.path() << " Error: " << std::strerror(err);
            throw std::runtime_error(msg.str());
        }

        RAW_LOG_INFO("Successfull file deletion: %s", dir_entry.path().c_str());

    } catch (std::filesystem::filesystem_error& err) {
        std::ostringstream msg;
        msg << "Creating directory entry error: " << err.what();
        throw std::runtime_error(msg.str());
    }
}

void FileManager::DeleteDirectory(const std::string& path) {
    try {
        std::filesystem::path abs_path(root_ + path);
        std::filesystem::directory_entry dir_entry(abs_path);

        std::filesystem::path root_path(root_ + "/");
        if (abs_path == root_path) {
            std::ostringstream msg;
            msg << "An attempt to delete a root directory: " << abs_path.string();
            throw std::runtime_error(msg.str());
        }

        if (!dir_entry.is_directory()) {
            std::ostringstream msg;
            msg << "An attempt to delete an object that is not a directory: " << abs_path.string();
            throw std::runtime_error(msg.str());
        }


        int res = std::remove(abs_path.c_str());

        if (res != 0) {
            int err = errno;
            std::ostringstream msg;
            msg << "Directory deletion error. Directory: " << dir_entry.path() << " Error: " << std::strerror(err);
            throw std::runtime_error(msg.str());
        }

        RAW_LOG_INFO("Successfull directory deletion: %s", dir_entry.path().c_str());

    } catch (std::filesystem::filesystem_error& err) {
        std::ostringstream msg;
        msg << "Creating directory entry error: " << err.what();
        throw std::runtime_error(msg.str());
    }
}