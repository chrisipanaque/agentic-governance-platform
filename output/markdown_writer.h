// markdown_writer.h
// Writes the planner markdown artifact into the repository so the branch has
// a concrete commit ahead of the base branch.

#pragma once

#include <string>

namespace Output {
    bool write_markdown_file(const std::string& path, const std::string& content);
}
