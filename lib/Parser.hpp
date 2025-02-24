#pragma once

#include <vector>
#include <string>

#include "HtmlTags.h"

struct URL {
    std::string url;
    std::vector<std::string> anchorText;
};

class Parser {
public:
    struct ParsingFlags {
        bool tagAnchor = false;
        bool tagTitle = false;
        bool tagEmphasize = false;

        bool baseFound = false;
        bool closingTag = false;
        bool pushed = false;
    };
    Parser() = delete;
    
    // Pass the html that is in a std::string
    Parser(std::string& html);

    // Get all the words in an html
    std::vector<std::string> getWords();

    // Get all the urls in an html
    std::vector<URL> getUrls();
    
    // Get all the titles in an html
    std::vector<std::string> getTitle();

    // Get all the emphasized words in an html (currently H1-H3, bolds, italics)
    std::vector<std::string> getEmphasized();

    // Get the number of images
    int getNumImages();

private:
    std::string::iterator current;
    std::string::iterator end;

    // Helper functions
    void handleDiscardSection(size_t taglen);
    void extractHref(std::string &out);
    void extractSrc(std::string &out);
    void pushWord(ParsingFlags flags, std::vector<std::string>& anchorText, std::string& currWord);

    // Data
    std::vector<std::string> words, titleWords, emphasized;
    std::vector<URL> links;
    URL baseURL;
    int numImages;
};
