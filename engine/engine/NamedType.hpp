#pragma once

#include "engine/Clamp.hpp"

#include <compare>

template <typename T, typename Parameter, template <typename> class... Mixins>
class NamedType : public Mixins<NamedType<T, Parameter, Mixins...>>...
{
public:
  constexpr explicit NamedType(const T& value)
    : value(value)
  {
  }

  template <typename Integral>
  constexpr explicit NamedType(Integral value)
    : value(clampedTo<T>(value))
  {
  }

  [[nodiscard]] constexpr T& get() { return value; }
  [[nodiscard]] constexpr const T& get() const { return value; }

  [[nodiscard]] constexpr T& _get() { return value; }
  [[nodiscard]] constexpr const T& _get() const { return value; }

private:
  T value;
};

template <typename T, typename Parameter, template <typename> class... Mixins>
class Percentage : public Mixins<Percentage<T, Parameter, Mixins...>>...
{
public:
  constexpr explicit Percentage(const T& value)
    : value(value)
  {
  }

  template <typename Integral>
  constexpr explicit Percentage(Integral value)
    : value(clampedTo<T>(value))
  {
  }

  [[nodiscard]] constexpr T& in_percent() { return value; }
  [[nodiscard]] constexpr const T& in_percent() const { return value; }

  [[nodiscard]] constexpr T& _get() { return value; }
  [[nodiscard]] constexpr const T& _get() const { return value; }

private:
  T value;
};

template <typename T, template <typename> class crtpType>
struct MixinBase
{
  constexpr T& underlying() { return static_cast<T&>(*this); }
  constexpr const T& underlying() const { return static_cast<const T&>(*this); }
};

template <typename T>
struct Addable : MixinBase<T, Addable>
{
  [[nodiscard]] constexpr T operator+(const T& other) const { return T(this->underlying()._get() + other._get()); }

  constexpr T& operator+=(const T& other)
  {
    this->underlying()._get() += other._get();
    return this->underlying();
  }
};

template <typename T>
struct Incrementable : MixinBase<T, Incrementable>
{
  constexpr T& operator++() { ++(this->underlying()._get()); return this->underlying(); }
  constexpr T operator++(int) { return T((this->underlying()._get())++); }
};

template <typename T>
struct Negation : MixinBase<T, Negation>
{
  [[nodiscard]] constexpr T operator-() const { return T(-this->underlying()._get()); }
};

template <typename T>
struct Subtractable : MixinBase<T, Subtractable>
{
  [[nodiscard]] T operator-(const T& other) const { return T(this->underlying()._get() - other._get()); }

  constexpr T& operator-=(const T& other)
  {
    this->underlying()._get() -= other._get();
    return this->underlying();
  }
};

template <typename T>
struct Scalable : MixinBase<T, Scalable>
{
  template <typename Scalar>
  [[nodiscard]] T operator*(Scalar other) const
  {
    return T(this->underlying()._get() * other);
  }

  template <typename Scalar>
  [[nodiscard]] T operator/(Scalar other) const
  {
    return T(this->underlying()._get() / other);
  }

  template <typename Scalar>
  constexpr T& operator*=(Scalar other)
  {
    this->underlying()._get() *= other;
    return this->underlying();
  }

  template <typename Scalar>
  constexpr T& operator/=(Scalar other)
  {
    this->underlying()._get() /= other;
    return this->underlying();
  }
};

template <typename T>
struct PercentOf : MixinBase<T, PercentOf>
{
  constexpr T percentage(unsigned percent) const
  {
    using value_type = std::remove_reference_t<decltype(this->underlying()._get())>;
    return T(clampedTo<value_type>(this->underlying()._get() * percent / 100));
  }
};

template <typename T>
struct Comparable : MixinBase<T, Comparable>
{
};

template <typename T>
constexpr auto operator<=>(const Comparable<T>& lhs, const Comparable<T>& rhs)
{
  return lhs.underlying()._get() <=> rhs.underlying()._get();
}

template <typename T>
constexpr bool operator==(const Comparable<T>& lhs, const Comparable<T>& rhs)
{
  return lhs.underlying()._get() == rhs.underlying()._get();
}
