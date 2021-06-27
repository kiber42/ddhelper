#pragma once

#include <compare>

template <typename T, typename Parameter, template <typename> class... Mixins>
class NamedType : public Mixins<NamedType<T, Parameter, Mixins...>>...
{
public:
  constexpr explicit NamedType(T const& value)
    : value(value)
  {
  }
  [[nodiscard]] constexpr T& get() { return value; }
  [[nodiscard]] constexpr const T& get() const { return value; }

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
  [[nodiscard]] constexpr T operator+(const T& other) const { return T(this->underlying().get() + other.get()); }

  constexpr T& operator+=(const T& other)
  {
    this->underlying().get() += other.get();
    return this->underlying();
  }
};

template <typename T>
struct Subtractable : MixinBase<T, Subtractable>
{
  [[nodiscard]] T operator-(const T& other) const { return T(this->underlying().get() - other.get()); }

  constexpr T& operator-=(const T& other)
  {
    this->underlying().get() -= other.get();
    return this->underlying();
  }
};

template <typename T>
struct Scalable : MixinBase<T, Scalable>
{
  template <typename Scalar>
  [[nodiscard]] T operator*(Scalar other) const { return T(this->underlying().get() * other); }

  template <typename Scalar>
  [[nodiscard]] T operator/(Scalar other) const { return T(this->underlying().get() / other); }

  template <typename Scalar>
  constexpr T& operator*=(Scalar other)
  {
    this->underlying().get() *= other;
    return this->underlying();
  }

  template <typename Scalar>
  constexpr T& operator/=(Scalar other)
  {
    this->underlying().get() /= other;
    return this->underlying();
  }
};

template <typename T>
struct Comparable : MixinBase<T, Comparable>
{
};

template <typename T>
auto operator<=>(const Comparable<T>& lhs, const Comparable<T>& rhs)
{
  return lhs.underlying().get() <=> rhs.underlying().get();
}
