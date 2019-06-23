#pragma once

#include <fmt/format.h>
#include <NamedType/named_type.hpp>

namespace st
{
  using PanelId = fluent::NamedType<uint64_t, struct PanelIdTag, fluent::Comparable, fluent::Printable>;
  using SerialId = fluent::NamedType<uint64_t, struct SerialIdTag, fluent::Comparable, fluent::Hashable, fluent::Printable>;
  using ButtonId = fluent::NamedType<unsigned, struct ButtonIdTag, fluent::Comparable, fluent::Printable>;
  using OutputId = fluent::NamedType<unsigned, struct OutputIdTag, fluent::Comparable, fluent::Printable>;
  using HardwareDirection = fluent::NamedType<uint64_t, struct HardwareDirectionTag, fluent::Comparable, fluent::Printable>;
  using HardwareValue = fluent::NamedType<uint64_t, struct HardwareValueTag, fluent::Comparable, fluent::Printable>;
}
