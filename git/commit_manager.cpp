// commit_manager.cpp
// Minimal git commit helper. Uses git add and git commit to create an explicit
// commit containing the generated planning artifact.

#include "commit_manager.h"
#include <cstdlib>
#include <string>
#include <fstream>
#include <unistd.h>
#include <fcntl.h>

namespace Git {
    bool stage_file(const std::string& path) {
        std::string cmd = "git add -- \"" + path + "\"";
        int result = std::system(cmd.c_str());
        return result == 0;
    }

    bool commit_staged(const std::string& message) {
        char tmp_name[] = "/tmp/planner-cli-msg-XXXXXX";
        int fd = mkstemp(tmp_name);
        if (fd == -1) {
            return false;
        }
        std::ofstream ofs(tmp_name);
        if (!ofs.is_open()) {
            close(fd);
            unlink(tmp_name);
            return false;
        }
        ofs << message;
        ofs.close();
        close(fd);

        std::string cmd = "git commit -F \"" + std::string(tmp_name) + "\"";
        int result = std::system(cmd.c_str());
        unlink(tmp_name);
        return result == 0;
    }
}
