#pragma once

#include <vector>
#include <string>

class Tokenizer {
public:
    Tokenizer(const std::string& html);

    // Should add numEntries to buf. Return the number of entries actually read
    size_t read(std::vector<std::string> buf, size_t numEntries);

    // Should move _index to start + offset. offset can be negative but _index cannot go below 0
    void seek(size_t start, int offset);
private:
    std::vector<std::string> _tokens;

    size_t _index;
};
