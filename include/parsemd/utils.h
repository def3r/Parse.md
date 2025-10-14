#ifndef PARSEMD_UTILS_H_
#define PARSEMD_UTILS_H_

#include <string_view>

namespace markdown {

void ltrim(std::string_view&);
void trim(std::string_view& sv, size_t lPos = 0, size_t rPos = 0);

}  // namespace markdown

#endif  // !PARSEMD_UTILS_H_
