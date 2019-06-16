#include "JsonAssembler.hpp"

using st::JsonAssembler;

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void JsonAssembler::Add(const std::string& Bytes)
{
  mBytes += Bytes;

  Check();
}


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void JsonAssembler::Check()
{
  while (!mBytes.empty() && mBytes[0] != '{')
  {
    mBytes = mBytes.substr(1);
  }

  int OpenCount = 0;

  int ClosedCount = 0;

  size_t Index = 0;

  for (const auto Character : mBytes)
  {
    Index++;

    if (Character == '{')
    {
      OpenCount++;
    }
    else if (Character == '}')
    {
      ClosedCount++;
    }

    if (ClosedCount == OpenCount)
    {
      mSignalPacket(mBytes.substr(0, Index));

      mBytes = mBytes.substr(Index);

      return Check();
    }
  }
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
const dl::Signal<const std::string&>& JsonAssembler::GetSignalPacket() const
{
  return mSignalPacket;
}
