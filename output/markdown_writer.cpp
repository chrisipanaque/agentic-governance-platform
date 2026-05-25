// markdown_writer.cpp
// Simple writer for markdown planning artifacts.

#include "markdown_writer.h"
#include <fstream>
#include <filesystem>

namespace fs = std::filesystem;

namespace Output {
    bool write_markdown_file(const std::string& path, const std::string& content) {
        fs::path file_path(path);
        if (file_path.has_parent_path()) {
            fs::create_directories(file_path.parent_path());
        }
        std::ofstream ofs(file_path, std::ios::out | std::ios::trunc);
        if (!ofs.is_open()) {
            return false;
        }
        ofs << content;
        return ofs.good();
    }
}
