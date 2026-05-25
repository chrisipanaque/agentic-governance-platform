// commit_manager.h
// Minimal git staging and commit helpers used by the orchestrator.

#pragma once

#include <string>

namespace Git {
    bool stage_file(const std::string& path);
    bool commit_staged(const std::string& message);
}
