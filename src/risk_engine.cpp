#include "risk_engine.hpp"
#include <nlohmann/json.hpp>
#include <fstream>
#include <iostream>
#include <algorithm>
#include <cmath>

using json = nlohmann::json;

RiskEngine::RiskEngine(const std::string& rules_path)
    : rules_path_(rules_path) {}

void RiskEngine::load_rules() {
    std::ifstream file(rules_path_);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open risk rules file: " + rules_path_);
    }

    json config = json::parse(file);
    
    if (config.contains("rules") && config["rules"].is_array()) {
        for (const auto& rule : config["rules"]) {
            rules_.push_back({
                rule["category"].get<std::string>(),
                rule["pattern"].get<std::string>(),
                rule["weight"].get<double>()
            });
        }
    }
}

bool RiskEngine::matches_pattern(const std::string& text, const std::string& pattern) const {
    // Simple glob-like matching
    if (pattern.find('*') == std::string::npos) {
        return text.find(pattern) != std::string::npos || text == pattern;
    }
    
    size_t text_idx = 0;
    size_t pattern_idx = 0;
    
    while (text_idx < text.length() && pattern_idx < pattern.length()) {
        if (pattern[pattern_idx] == '*') {
            if (pattern_idx == pattern.length() - 1) return true;
            pattern_idx++;
            size_t next_match = text.find(pattern[pattern_idx], text_idx);
            if (next_match == std::string::npos) return false;
            text_idx = next_match;
        } else if (pattern[pattern_idx] == text[text_idx]) {
            pattern_idx++;
            text_idx++;
        } else {
            return false;
        }
    }
    
    while (pattern_idx < pattern.length() && pattern[pattern_idx] == '*') {
        pattern_idx++;
    }
    
    return pattern_idx == pattern.length();
}

double RiskEngine::calculate_diff_size_risk(const DiffScanner::DiffStats& diff_stats) const {
    int total_changes = diff_stats.total_additions + diff_stats.total_deletions;
    
    // Large diffs are higher risk (logarithmic scale)
    if (total_changes > 1000) return 15.0;
    if (total_changes > 500) return 10.0;
    if (total_changes > 100) return 5.0;
    if (total_changes > 50) return 2.0;
    return 0.0;
}

RiskEngine::Severity RiskEngine::score_to_severity(double score) const {
    if (score >= 75.0) return Severity::CRITICAL;
    if (score >= 50.0) return Severity::HIGH;
    if (score >= 25.0) return Severity::MEDIUM;
    return Severity::LOW;
}

std::string RiskEngine::get_recommendation(const RiskScore& score) const {
    switch (score.severity) {
        case Severity::CRITICAL:
            return "BLOCK: Do not approve. Escalate to security team.";
        case Severity::HIGH:
            return "REVIEW: Requires manual security review before approval.";
        case Severity::MEDIUM:
            return "CAUTION: Review carefully. Consider additional testing.";
        case Severity::LOW:
            return "PROCEED: Standard review sufficient.";
    }
    return "UNKNOWN";
}

RiskEngine::RiskScore RiskEngine::calculate_score(const DiffScanner::DiffStats& diff_stats) {
    RiskScore result;
    result.score = 0.0;
    
    // Score each changed file against rules
    for (const auto& file : diff_stats.files) {
        for (const auto& rule : rules_) {
            if (matches_pattern(file.file_path, rule.pattern)) {
                result.score += rule.weight;
                result.risk_factors.push_back(
                    rule.category + ": " + file.file_path + " (+" + 
                    std::to_string(rule.weight) + " points)"
                );
            }
        }
    }
    
    // Add risk for large diffs
    double diff_risk = calculate_diff_size_risk(diff_stats);
    if (diff_risk > 0.0) {
        result.score += diff_risk;
        result.risk_factors.push_back(
            "Large diff: " + std::to_string(diff_stats.files.size()) + " files, " +
            std::to_string(diff_stats.total_additions) + " additions, " +
            std::to_string(diff_stats.total_deletions) + " deletions (+" + 
            std::to_string(static_cast<int>(diff_risk)) + " points)"
        );
    }
    
    // Cap score at 100
    result.score = std::min(100.0, result.score);
    result.severity = score_to_severity(result.score);
    result.recommendation = get_recommendation(result);
    
    return result;
}

void RiskEngine::print_score(const RiskScore& score) const {
    std::cout << "Risk Score: " << static_cast<int>(score.score) << "/100\n";
    
    const char* severity_str = "UNKNOWN";
    switch (score.severity) {
        case Severity::LOW: severity_str = "LOW"; break;
        case Severity::MEDIUM: severity_str = "MEDIUM"; break;
        case Severity::HIGH: severity_str = "HIGH"; break;
        case Severity::CRITICAL: severity_str = "CRITICAL"; break;
    }
    std::cout << "Severity: " << severity_str << '\n';
    
    if (!score.risk_factors.empty()) {
        std::cout << "Risk factors:\n";
        for (const auto& factor : score.risk_factors) {
            std::cout << "  - " << factor << '\n';
        }
    }
    
    std::cout << "Recommendation: " << score.recommendation << '\n';
}
