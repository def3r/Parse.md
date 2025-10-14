#include "parsemd/utils.h"

namespace markdown {

void ltrim(std::string_view& s) {
  s.remove_prefix(s.find_first_not_of(' '));
}

void trim(std::string_view& sv, size_t lPos, size_t rPos) {
  sv.remove_prefix(lPos);
  sv.remove_prefix(sv.find_first_not_of(' '));
  sv.remove_suffix(sv.size() - sv.find_last_not_of(' ') - 1);
  sv.remove_suffix(rPos);
}

}  // namespace markdown
