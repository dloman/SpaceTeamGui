#pragma once

#include <fmt/format.h>
#include <NamedType/named_type.hpp>

namespace st
{
  using PanelId = fluent::NamedType<uint64_t, struct PanelIdTag, fluent::Comparable, fluent::Printable>;
  using SerialId = fluent::NamedType<uint64_t, struct SerialIdTag, fluent::Comparable, fluent::Hashable, fluent::Printable>;
  using OutputId = fluent::NamedType<unsigned, struct OutputIdTag, fluent::Comparable, fluent::Printable>;
  using HardwareDirection = fluent::NamedType<uint64_t, struct HardwareDirectionTag, fluent::Comparable, fluent::Printable>;
  using HardwareValue = fluent::NamedType<uint64_t, struct HardwareValueTag, fluent::Comparable, fluent::Printable>;
  using ButtonIndex = fluent::NamedType<unsigned, struct ButtonIdTag, fluent::Comparable, fluent::Printable, fluent::Readable,fluent::Hashable>;

  struct ButtonId
  {
    ButtonId(st::ButtonIndex Index, st::SerialId Serial)
    : mSerial(Serial),
      mButtonIndex(Index)
    {
    }

    SerialId mSerial;
    ButtonIndex mButtonIndex;

    bool operator == (const ButtonId& Rhs) const
    {
      return mSerial == Rhs.mSerial &&
        mButtonIndex == Rhs.mButtonIndex;
    }

    bool operator != (const ButtonId& Rhs) const
    {
      return !(*this == Rhs);
    }

    friend std::ostream& operator<<(std::ostream&, const ButtonId&);
    friend std::istream& operator>>(std::istream&, ButtonId&);
  };

  inline
  std::ostream& operator<<(std::ostream& Stream, const ButtonId& Id)
  {
    Stream << '[' << Id.mSerial << ',' << Id.mButtonIndex;

    return Stream;
  }

  inline
  std::istream& operator>>(std::istream& Stream, ButtonId& Id)
  {
    std::string Value;

    Stream >> Value;

    std::cout << Value;

    return Stream;
  }
}

namespace std
{
  template <>
  struct hash<st::ButtonId>
  {
    size_t operator()(const st::ButtonId& Id) const noexcept
    {
      const auto Hash1 = std::hash<st::SerialId>{}(Id.mSerial);

      const auto Hash2 = std::hash<st::ButtonIndex>{}(Id.mButtonIndex);

      return Hash1 ^ (Hash2 << 1);
    }
  };
}
