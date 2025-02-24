#include <ctype.h>
#include <cstring>
#include <string>
#include <cassert>
#include <algorithm>
#include "HtmlTags.h"

// name points to beginning of the possible HTML tag name.
// nameEnd points to one past last character.
// Comparison is case-insensitive.
// Use a binary search.
// If the name is found in the TagsRecognized table, return
// the corresponding action.
// If the name is not found, return OrdinaryText.

DesiredAction LookupPossibleTag(std::string::iterator name, std::string::iterator nameEnd) {
   size_t namelen = nameEnd - name;
   int l = 0; 
   int r = NumberOfTags - 1;
   if(namelen > LongestTagLength){ // if the length is too long, it cannot be a tag
      return DesiredAction::OrdinaryText; 
   }
   std::string lowerName(name, nameEnd);
   std::transform(lowerName.begin(), lowerName.end(), lowerName.begin(), ::tolower);
   
   while(l <= r){
      int mid = l + (r - l) / 2;
      const std::string& currTag = TagsRecognized[mid].Tag;
      size_t currlen = currTag.length();

      int cmp = lowerName.compare(0, std::min(currlen, namelen), currTag);
      if(cmp == 0){
         if(namelen == currlen){
            return TagsRecognized[mid].Action;
         } 
         else if(namelen < currlen){
            r = mid - 1;
         } 
         else{
            l = mid + 1;
         }
      } 
      else if(cmp < 0){
         r = mid - 1;
      } 
      else{
         l = mid + 1;
      }
   }
   return DesiredAction::OrdinaryText; // Not found
}