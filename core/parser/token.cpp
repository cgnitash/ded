


#include<string>
#include"token.h"


namespace life {

token_type
    parse_token_type(std::string s)
{
  if (s == "{") return token_type::open_brace;
  if (s == "}") return token_type::close_brace;
  if (s == "=") return token_type::assignment;
  if (s[0] == '$') return token_type::component;
  if (s[0] == '!') return token_type::variable;
  if ((s.find('<') != std::string::npos || s.find('>') != std::string::npos))
    return token_type::tag_rewrite;
  return token_type::word;
}
}

