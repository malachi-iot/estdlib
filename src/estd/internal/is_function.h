#pragma once

namespace estd {

// doesn't compile
#if 0

// lifted from https://en.cppreference.com/w/cpp/types/is_function

// primary template
template<class>
struct is_function : std::false_type { };

// specialization for regular functions
template<class Ret, class... Args>
struct is_function<Ret(Args...)> : std::true_type {};

// specialization for variadic functions such as std::printf
template<class Ret, class... Args>
struct is_function<Ret(Args......)> : std::true_type {};

// specialization for function types that have cv-qualifiers
template<class Ret, class... Args>
struct is_function<Ret(Args...) const> : std::true_type {};
template<class Ret, class... Args>
struct is_function<Ret(Args...) volatile> : std::true_type {};
template<class Ret, class... Args>
struct is_function<Ret(Args...) const volatile> : std::true_type {};
template<class Ret, class... Args>
struct is_function<Ret(Args......) const> : std::true_type {};
template<class Ret, class... Args>
struct is_function<Ret(Args......) volatile> : std::true_type {};
template<class Ret, class... Args>
struct is_function<Ret(Args......) const volatile> : std::true_type {};

// specialization for function types that have ref-qualifiers
template<class Ret, class... Args>
struct is_function<Ret(Args...) &> : std::true_type {};
template<class Ret, class... Args>
struct is_function<Ret(Args...) const &> : std::true_type {};
template<class Ret, class... Args>
struct is_function<Ret(Args...) volatile &> : std::true_type {};
template<class Ret, class... Args>
struct is_function<Ret(Args...) const volatile &> : std::true_type {};
template<class Ret, class... Args>
struct is_function<Ret(Args......) &> : std::true_type {};
template<class Ret, class... Args>
struct is_function<Ret(Args......) const &> : std::true_type {};
template<class Ret, class... Args>
struct is_function<Ret(Args......) volatile &> : std::true_type {};
template<class Ret, class... Args>
struct is_function<Ret(Args......) const volatile &> : std::true_type {};
template<class Ret, class... Args>
struct is_function<Ret(Args...) &&> : std::true_type {};
template<class Ret, class... Args>
struct is_function<Ret(Args...) const &&> : std::true_type {};
template<class Ret, class... Args>
struct is_function<Ret(Args...) volatile &&> : std::true_type {};
template<class Ret, class... Args>
struct is_function<Ret(Args...) const volatile &&> : std::true_type {};
template<class Ret, class... Args>
struct is_function<Ret(Args......) &&> : std::true_type {};
template<class Ret, class... Args>
struct is_function<Ret(Args......) const &&> : std::true_type {};
template<class Ret, class... Args>
struct is_function<Ret(Args......) volatile &&> : std::true_type {};
template<class Ret, class... Args>
struct is_function<Ret(Args......) const volatile &&> : std::true_type {};

// specializations for noexcept versions of all the above (C++17 and later)

template<class Ret, class... Args>
struct is_function<Ret(Args...) noexcept> : std::true_type {};
template<class Ret, class... Args>
struct is_function<Ret(Args......) noexcept> : std::true_type {};
template<class Ret, class... Args>
struct is_function<Ret(Args...) const noexcept> : std::true_type {};
template<class Ret, class... Args>
struct is_function<Ret(Args...) volatile noexcept> : std::true_type {};
template<class Ret, class... Args>
struct is_function<Ret(Args...) const volatile noexcept> : std::true_type {};
template<class Ret, class... Args>
struct is_function<Ret(Args......) const noexcept> : std::true_type {};
template<class Ret, class... Args>
struct is_function<Ret(Args......) volatile noexcept> : std::true_type {};
template<class Ret, class... Args>
struct is_function<Ret(Args......) const volatile noexcept> : std::true_type {};
template<class Ret, class... Args>
struct is_function<Ret(Args...) & noexcept> : std::true_type {};
template<class Ret, class... Args>
struct is_function<Ret(Args...) const & noexcept> : std::true_type {};
template<class Ret, class... Args>
struct is_function<Ret(Args...) volatile & noexcept> : std::true_type {};
template<class Ret, class... Args>
struct is_function<Ret(Args...) const volatile & noexcept> : std::true_type {};
template<class Ret, class... Args>
struct is_function<Ret(Args......) & noexcept> : std::true_type {};
template<class Ret, class... Args>
struct is_function<Ret(Args......) const & noexcept> : std::true_type {};
template<class Ret, class... Args>
struct is_function<Ret(Args......) volatile & noexcept> : std::true_type {};
template<class Ret, class... Args>
struct is_function<Ret(Args......) const volatile & noexcept> : std::true_type {};
template<class Ret, class... Args>
struct is_function<Ret(Args...) && noexcept> : std::true_type {};
template<class Ret, class... Args>
struct is_function<Ret(Args...) const && noexcept> : std::true_type {};
template<class Ret, class... Args>
struct is_function<Ret(Args...) volatile && noexcept> : std::true_type {};
template<class Ret, class... Args>
struct is_function<Ret(Args...) const volatile && noexcept> : std::true_type {};
template<class Ret, class... Args>
struct is_function<Ret(Args......) && noexcept> : std::true_type {};
template<class Ret, class... Args>
struct is_function<Ret(Args......) const && noexcept> : std::true_type {};
template<class Ret, class... Args>
struct is_function<Ret(Args......) volatile && noexcept> : std::true_type {};
template<class Ret, class... Args>
struct is_function<Ret(Args......) const volatile && noexcept> : std::true_type {};

#endif

}
