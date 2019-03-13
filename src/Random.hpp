#pragma once

#include <random>
#include <type_traits>

namespace dl::random
{
  //namespace
  //{
    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    template <typename T>
    std::enable_if_t<std::is_integral_v<T>, T> GetUniformImpl()
    {
      static std::random_device randomDevice;

      static std::mt19937 generator(randomDevice());

      static std::uniform_int_distribution<T> distribution;

      return distribution(generator);
    }

    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    template <typename T>
    std::enable_if_t<!std::is_integral_v<T>, T> GetUniformImpl()
    {
      static std::random_device randomDevice;

      static std::mt19937 generator(randomDevice());

      static std::uniform_real_distribution<T> distribution;

      return distribution(generator);
    }
    //}

  //----------------------------------------------------------------------------
  //----------------------------------------------------------------------------
  template <typename T>
  T GetUniform()
  {
    return GetUniformImpl<T>();
  }
}
