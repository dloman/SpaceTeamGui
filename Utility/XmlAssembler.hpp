#pragma once

#include <Signal/Signal.hpp>

namespace st
{
  class XmlAssembler
  {
    public:

      void Add(const std::string& Bytes);

      const dl::Signal<const std::string&>& GetSignalPacket() const;

    private:

      dl::Signal<const std::string&> mSignalPacket;

      std::string mBytes;
  };
}

