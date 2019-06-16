#pragma once

#include <Signal/Signal.hpp>

namespace st
{
  class JsonAssembler
  {
    public:

      void Add(const std::string& Bytes);

      const dl::Signal<const std::string&>& GetSignalPacket() const;

    private:

      void Check();

      dl::Signal<const std::string&> mSignalPacket;

      std::string mBytes;
  };
}

