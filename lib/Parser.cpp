#include "Parser.hpp"
#include <iostream>
#include <algorithm>
#include <cstring>
#include <cctype>

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

void Parser::extractHref(std::string &out){
// for <a> tags, extracts href = "url" and moves current to the end of the opening tag
    std::string::iterator url = end;
    std::string::iterator endUrl = end;

    for(; current < end && *current != '>'; current++){
        if(end - current > 4 && *current == 'h' && std::equal(current, current + 4, "href")){
            current += 4;
            for(; current < end && isSpace(*current); current++);

            if(*current == '='){
                current++;
                for(; current < end && isSpace(*current); current++);

                if (current < end && (*current == '"' || *current == '\'')) {
                    char quoteType = *current++;
                    url = current;
                    endUrl = std::find(current, end, quoteType);
                    if(endUrl != end){ // found a complete quote
                        break;
                    }
                }
            }
        }
    }

    current = std::find(current, end, '>'); // Move to '>'
    out = (url != end && endUrl != end) ? std::string(url, endUrl) : "";
}   

void Parser::extractSrc(std::string &out){
    // extracts src = "url" and moves current to the end of the opening tag
    std::string::iterator url = end;
    std::string::iterator endUrl = end;

    for(; current < end && *current != '>'; current++){
        if(end - current > 3 && *current == 's' && std::equal(current, current + 3, "src")){
            current += 3;
            for(; current < end && isSpace(*current); current++);

            if(*current == '='){
                current++;
                for(; current < end && isSpace(*current); current++);

                if (current < end && (*current == '"' || *current == '\'')) {
                    char quoteType = *current++;
                    url = current;
                    endUrl = std::find(current, end, quoteType);
                    if(endUrl != end){ // found a complete quote
                        break;
                    }
                }
            }
        }
    }

    current = std::find(current, end, '>'); // Move to '>'
    out = (url != end && endUrl != end) ? std::string(url, endUrl) : "";
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
                            extractHref(href);
                            if(!href.empty()){ // Ignore anchors with no URL
                                currURL.url = std::move(href);
                                flags.tagAnchor = true;
                            }
                        }
                        else{ // This acts like a closing for the current anchor and starts a new anchor
                            std::string href;
                            extractHref(href);
                            if(!href.empty()){
                                links.push_back(std::move(currURL));
                                currURL.url = std::move(href);
                            }
                        }
                        break;

                    case DesiredAction::Base:
                        if(!flags.baseFound){
                            std::string href;
                            extractHref(href); // bases are self closing
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
                        extractSrc(src); // embeds are self closing
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

