#pragma once

#include <random>
#include <type_traits>

namespace st::random
{
  namespace
  {
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

    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    template <typename T>
    std::enable_if_t<std::is_integral_v<T>, T> GetUniformImpl(T min, T max)
    {
      static std::random_device randomDevice;

      static std::mt19937 generator(randomDevice());

      std::uniform_int_distribution<T> distribution(min, max);

      return distribution(generator);
    }

    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    template <typename T>
    std::enable_if_t<!std::is_integral_v<T>, T> GetUniformImpl(T min, T max)
    {
      static std::random_device randomDevice;

      static std::mt19937 generator(randomDevice());

      std::uniform_real_distribution<T> distribution(min, max);

      return distribution(generator);
    }
  }

  //----------------------------------------------------------------------------
  //----------------------------------------------------------------------------
  template <typename T>
  T GetUniform()
  {
    return GetUniformImpl<T>();
  }

  //----------------------------------------------------------------------------
  //----------------------------------------------------------------------------
  template <typename T>
  T GetUniform(T Min, T Max)
  {
    return GetUniformImpl<T>(Min, Max);
  }
}
