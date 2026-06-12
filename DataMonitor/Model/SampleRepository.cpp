#include "SampleRepository.h"
#include "../Util/JsonReader.h"
#include <stdexcept>

SampleRepository::SampleRepository(std::string filePath)
    : filePath_(std::move(filePath)) {
    load();
}

bool SampleRepository::load() {
    auto arr = JsonReader::readArray(filePath_, "samples");
    samples_.clear();
    for (const auto& obj : arr) {
        Sample s;
        if (auto it = obj.find("id");           it != obj.end()) s.id           = it->second;
        if (auto it = obj.find("name");         it != obj.end()) s.name         = it->second;
        if (auto it = obj.find("quantity");     it != obj.end()) s.quantity     = std::stoi(it->second);
        if (auto it = obj.find("yield");        it != obj.end()) s.yield        = std::stod(it->second);
        if (auto it = obj.find("cycleTime");    it != obj.end()) s.cycleTime    = std::stod(it->second);
        if (auto it = obj.find("registeredAt"); it != obj.end()) s.registeredAt = it->second;
        if (!s.id.empty()) samples_.push_back(std::move(s));
    }
    return !samples_.empty();
}

std::vector<Sample> SampleRepository::findAll() const {
    return samples_;
}

std::optional<Sample> SampleRepository::findById(const std::string& id) const {
    for (const auto& s : samples_)
        if (s.id == id) return s;
    return std::nullopt;
}
