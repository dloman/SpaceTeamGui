#pragma once

namespace st
{
  template<class... Ts>
  struct Visitor : Ts...
  {
    using Ts::operator()...;
  };

  template<class... Ts>
  Visitor(Ts...) -> Visitor<Ts...>;
}
