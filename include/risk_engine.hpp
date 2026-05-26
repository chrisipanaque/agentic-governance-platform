#pragma once

#include <string>
#include <vector>
#include "diff_scanner.hpp"

class RiskEngine {
public:
    enum class Severity {
        LOW,
        MEDIUM,
        HIGH,
        CRITICAL
    };

    struct RiskScore {
        double score = 0.0;
        Severity severity = Severity::LOW;
        std::vector<std::string> risk_factors;
        std::string recommendation;
    };

    struct RiskRule {
        std::string category;
        std::string pattern;
        double weight = 0.0;
    };

    explicit RiskEngine(const std::string& rules_path);

    void load_rules();
    RiskScore calculate_score(const DiffScanner::DiffStats& diff_stats);
    void print_score(const RiskScore& score) const;

private:
    std::string rules_path_;
    std::vector<RiskRule> rules_;

    Severity score_to_severity(double score) const;
    std::string get_recommendation(const RiskScore& score) const;
    double calculate_diff_size_risk(const DiffScanner::DiffStats& diff_stats) const;
    bool matches_pattern(const std::string& text, const std::string& pattern) const;
};
