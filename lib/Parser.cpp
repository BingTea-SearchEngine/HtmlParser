#include <iostream>
#include <algorithm>
#include <cstring>
#include <cctype>

#include "Parser.hpp"
#include "HtmlTags.h"

std::vector<std::string> Parser::getWords(){
    return words;
}

std::vector<URL> Parser::getUrls(){
    return links;
}

std::vector<std::string> Parser::getTitle(){
    return titleWords;
}

std::vector<std::string> Parser::getEmphasized(){
    return emphasized;
}

int Parser::getNumImages(){
    return numImages;
}

std::string Parser::getLanguage(){
    return language;
}

static inline bool isSpace(char c){ // returns true if c is any form of whitespace
    return(c == ' ' || c == '\t' || c == '\r' || c == '\n' || c == '\v' || c == '\f');
}

static inline bool isPunctuation(char c){ // returns true if c is punctuation (does NOT check for HTML punctuation) 
    return c == ',' || c == '.' || c == '!' || c == '?' || c == ':' || c == ';' || c == '-' || c == '\'' || c == '\"';
}

inline void Parser::pushWord(ParsingFlags flags, std::vector<std::string>& anchorText, std::string& currWord){
    if(flags.tagAnchor){
        anchorText.push_back(currWord);
    }
    if(flags.tagTitle){
        titleWords.push_back(currWord);
    }
    else if(flags.tagEmphasize){
        emphasized.push_back(currWord);
    }
    else{
        words.push_back(currWord);
    }
    
}

void Parser::handleDiscardSection(size_t taglen){ 
// moves pos from the first char after the tag to the first char after the end of the closing tag
    static const std::string tagclose = "</";
    current = std::find(current, end, '>');

    for(; current < end; current++){ 
        current = std::search(current, end, tagclose.begin(), tagclose.end());
        if (current == end){
            break;
        }
        current += 2;
        std::string::iterator tag = current;
        for(; current < end && *current != '>' && !isSpace(*current) && *current != '/'; current++);
        
        if(current - tag < 7){ // if tag is too long to be the correct closer, don't check it
            std::string currtag(tag, current);
            transform(currtag.begin(), currtag.end(), currtag.begin(), ::tolower);
            if((taglen == 5 && currtag == "style") || (taglen == 6 && currtag == "script") || (taglen == 3 && currtag == "svg")){
                current = std::find(current, end, '>');
                return;
            }
        }
    }
    current = end;
}

void Parser::extractAttribute(std::string attributeName, std::string &out){
    std::string::iterator attrValueStart = end;
    std::string::iterator attrValueEnd = end;

    size_t attrLen = attributeName.size();

    for(; current < end && *current != '>'; ++current){
        if(static_cast<size_t>(std::distance(current, end)) >= attrLen 
            && std::equal(attributeName.begin(), attributeName.end(), current)){

            current += attrLen;
            for(; current < end && isSpace(*current); current++);

            if (current < end && *current == '=') {
                current++;
                while (current < end && isSpace(*current)) {
                    ++current;
                }

                if (current < end && (*current == '"' || *current == '\'')) {
                    char quoteType = *current++;
                    attrValueStart = current;
                    attrValueEnd = std::find(current, end, quoteType);
                    if (attrValueEnd != end) {
                        break;
                    }
                }
            }
        }
    }

    current = std::find(current, end, '>');

    if (attrValueStart != end && attrValueEnd != end) {
        out = std::string(attrValueStart, attrValueEnd);
    } else {
        out.clear();
    }
}

Parser::Parser(std::string& html){
    current = html.begin(); // set current and end
    end = html.end();

    ParsingFlags flags;

    std::string::iterator word = current; // points to start of current word
    URL currURL;

    for(; current < end; current++){ //iterate through buffer character by character
        if(*current == '<'){ // detected start of a tag
            flags.pushed = false;
            flags.closingTag = false;

            if(word != current){ // start of a tag is also the end of a word
                flags.pushed = true;
                std::string currword = std::string(word, current);
                pushWord(flags, currURL.anchorText, currword);
            }
            
            if(*++current == '/'){ // closing tag
                flags.closingTag = true;
                current++;
            }

            std::string::iterator tag = current; // note the start position of the buffer
            for(; current < end && *current!= '>' && *current != '/' && !isSpace(*current); current++); // find the end of the tag

            DesiredAction action = LookupPossibleTag(tag, current);

            if(flags.closingTag && (!(action == DesiredAction::Anchor && flags.tagAnchor) && 
            !(action == DesiredAction::Title && flags.tagTitle) && !(action == DesiredAction::Emphasize && flags.tagEmphasize))){
                current = std::find(current, end, '>');
            }
            else{
                switch(action){
                    case DesiredAction::Language: // <html> tag, search for a language, closing tag will automatically be ignored
                        {
                        std::string lang;
                        extractAttribute("lang", lang);
                        if(!lang.empty()){
                            language = lang;
                        }
                        current = std::find(current, end, '>');
                        break;
                        }
                    case DesiredAction::Discard:
                        current = std::find(current, end, '>');
                        break;

                    case DesiredAction::Image:
                        numImages++;
                        current = std::find(current, end, '>');
                        break;
                    
                    case DesiredAction::Emphasize:
                        if(!flags.closingTag){
                            flags.tagEmphasize = true;
                        }
                        else{
                            flags.tagEmphasize = false;
                        }
                        current = std::find(current, end, '>');
                        break;
                        
                    case DesiredAction::Title:
                        if(!flags.closingTag){
                            flags.tagTitle = true;
                        }
                        else{
                            flags.tagTitle = false;
                        }
                        current = std::find(current, end, '>');
                        break;

                    case DesiredAction::Comment:
                        static const std::string tagclose = "-->";
                        current = std::search(current, end, tagclose.begin(), tagclose.end());
                        if(current != end) {
                            current += 2; // Move iterator past "-->"
                        }
                        break;

                    case DesiredAction::DiscardSection: {
                        size_t taglen = current - tag;
                        handleDiscardSection(taglen);
                        break;
                    }

                    case DesiredAction::Anchor:
                        if(flags.closingTag){ // Closing anchor
                            flags.tagAnchor = false;
                            current = std::find(current, end, '>');
                            links.push_back(std::move(currURL));
                            currURL = URL();
                        } 
                        else if(!flags.tagAnchor){
                            std::string href;
                            extractAttribute("href", href);
                            if(!href.empty()){ // Ignore anchors with no URL
                                currURL.url = std::move(href);
                                flags.tagAnchor = true;
                            }
                        }
                        else{ // This acts like a closing for the current anchor and starts a new anchor
                            std::string href;
                            extractAttribute("href", href);
                            if(!href.empty()){
                                links.push_back(std::move(currURL));
                                currURL.url = std::move(href);
                            }
                        }
                        break;

                    case DesiredAction::Base:
                        if(!flags.baseFound){
                            std::string href;
                            extractAttribute("href", href); // bases are self closing
                            if(!href.empty()){
                                baseURL = {std::move(href),{}};
                                flags.baseFound = true;
                            }
                        }
                        else{
                            current = std::find(current, end, '>');
                        }
                        break;

                    case DesiredAction::Embed:{
                        std::string src;
                        extractAttribute("src", src); // embeds are self closing
                        if(!src.empty()){
                            links.push_back({std::move(src), {}});
                        }
                        break;
                    }
                    case DesiredAction::OrdinaryText:
                        if(flags.pushed){ // If a word was pushed at the opening of the tag e.g., asdf<fake, we need to remove it from the back of the vector
                            words.pop_back();
                            if(flags.tagAnchor && !currURL.anchorText.empty()){
                                currURL.anchorText.pop_back();
                            }
                        }
                        break;
                }
            }
            if(action != DesiredAction::OrdinaryText){
                word = current + 1;
            }
        }
        bool endWord = isSpace(*current) || isPunctuation(*current);
        if(endWord && word != current){ // whitespace and a word exists
            std::string currword = std::string(word, current);
            pushWord(flags, currURL.anchorText, currword);
            word = current + 1;
        }
        else if(endWord){ // whitespace with no words means advancing both pointers
            word++;
        }
    }
    if(current != word){
        words.push_back(std::string(word, current));
    }
}

