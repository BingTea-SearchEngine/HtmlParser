#pragma once

#include <vector>
#include <string>

#include "Tokenizer.hpp"

struct URL {
    std::string url;
    std::vector<std::string> anchorText;
};

class Parser {
public:
    Parser() = delete;
    
    // Pass the html that is in a std::string
    Parser(const std::string& html);

    // Get all the words in an html
    std::vector<std::string> getWords();

    // Get all the urls in an html
    std::vector<URL> getUrls();
    
    // Get all the titles in an html
    std::vector<std::string> getTitle();

private:
    Tokenizer _tokens;
};
