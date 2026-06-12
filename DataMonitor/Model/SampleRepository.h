#pragma once
#include <vector>
#include <optional>
#include <string>
#include "Sample.h"

class SampleRepository {
public:
    explicit SampleRepository(std::string filePath);

    std::vector<Sample>   findAll() const;
    std::optional<Sample> findById(const std::string& id) const;
    bool                  load();

    const std::string& filePath() const { return filePath_; }

private:
    std::string          filePath_;
    std::vector<Sample>  samples_;
};
