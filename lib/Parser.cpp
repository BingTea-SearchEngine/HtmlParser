#include "Parser.hpp"

Parser::Parser(const std::string& html) {
    _tokens = Tokenizer(html);
}


std::vector<std::string> Parser::getWords() {
    return {};
}

std::vector<URL> Parser::getUrls() {
    return {};
}

std::vector<std::string> Parser::getTitle() {
    return {};
}

