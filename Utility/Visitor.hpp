#pragma once

namespace util
{
  template<class... Ts>
  struct Visitor : Ts...
  {
    using Ts::operator()...;
  };

  template<class... Ts>
  Visitor(Ts...) -> Visitor<Ts...>;

  template<class... Ts>
  Visitor<Ts...> MakeVisitor(Ts&&... ts)
  {
    return Visitor<Ts...>{std::forward<Ts>(ts)...};
  }

}
