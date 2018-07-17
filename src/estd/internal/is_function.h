// normally we avoid these non-freestanding internal .h files, but in this case
// it's cleaner to do so


// lifted from https://en.cppreference.com/w/cpp/types/is_function

// primary template
template<class>
struct is_function : false_type { };

// specialization for regular functions
template<class Ret, class... Args>
struct is_function<Ret(Args...)> : true_type {};

// specialization for function types that have cv-qualifiers
template<class Ret, class... Args>
struct is_function<Ret(Args...) const> : true_type {};
template<class Ret, class... Args>
struct is_function<Ret(Args...) volatile> : true_type {};
template<class Ret, class... Args>
struct is_function<Ret(Args...) const volatile> : true_type {};

#ifdef FEATURE_CPP_AMBIGUOUS_ELLIPSES
// specialization for variadic functions such as printf
template<class Ret, class... Args>
struct is_function<Ret(Args......)> : true_type {};
template<class Ret, class... Args>
struct is_function<Ret(Args......) const> : true_type {};
template<class Ret, class... Args>
struct is_function<Ret(Args......) volatile> : true_type {};
template<class Ret, class... Args>
struct is_function<Ret(Args......) const volatile> : true_type {};
#endif

// specialization for function types that have ref-qualifiers
template<class Ret, class... Args>
struct is_function<Ret(Args...) &> : true_type {};
template<class Ret, class... Args>
struct is_function<Ret(Args...) const &> : true_type {};
template<class Ret, class... Args>
struct is_function<Ret(Args...) volatile &> : true_type {};
template<class Ret, class... Args>
struct is_function<Ret(Args...) const volatile &> : true_type {};
#ifdef FEATURE_CPP_AMBIGUOUS_ELLIPSES
template<class Ret, class... Args>
struct is_function<Ret(Args......) &> : true_type {};
template<class Ret, class... Args>
struct is_function<Ret(Args......) const &> : true_type {};
template<class Ret, class... Args>
struct is_function<Ret(Args......) volatile &> : true_type {};
template<class Ret, class... Args>
struct is_function<Ret(Args......) const volatile &> : true_type {};
#endif
template<class Ret, class... Args>
struct is_function<Ret(Args...) &&> : true_type {};
template<class Ret, class... Args>
struct is_function<Ret(Args...) const &&> : true_type {};
template<class Ret, class... Args>
struct is_function<Ret(Args...) volatile &&> : true_type {};
template<class Ret, class... Args>
struct is_function<Ret(Args...) const volatile &&> : true_type {};
#ifdef FEATURE_CPP_AMBIGUOUS_ELLIPSES
template<class Ret, class... Args>
struct is_function<Ret(Args......) &&> : true_type {};
template<class Ret, class... Args>
struct is_function<Ret(Args......) const &&> : true_type {};
template<class Ret, class... Args>
struct is_function<Ret(Args......) volatile &&> : true_type {};
template<class Ret, class... Args>
struct is_function<Ret(Args......) const volatile &&> : true_type {};
#endif

// specializations for noexcept versions of all the above (C++17 and later)

#if __cplusplus >= 201703L

template<class Ret, class... Args>
struct is_function<Ret(Args...) noexcept> : true_type {};
template<class Ret, class... Args>
struct is_function<Ret(Args......) noexcept> : true_type {};
template<class Ret, class... Args>
struct is_function<Ret(Args...) const noexcept> : true_type {};
template<class Ret, class... Args>
struct is_function<Ret(Args...) volatile noexcept> : true_type {};
template<class Ret, class... Args>
struct is_function<Ret(Args...) const volatile noexcept> : true_type {};
template<class Ret, class... Args>
struct is_function<Ret(Args......) const noexcept> : true_type {};
template<class Ret, class... Args>
struct is_function<Ret(Args......) volatile noexcept> : true_type {};
template<class Ret, class... Args>
struct is_function<Ret(Args......) const volatile noexcept> : true_type {};
template<class Ret, class... Args>
struct is_function<Ret(Args...) & noexcept> : true_type {};
template<class Ret, class... Args>
struct is_function<Ret(Args...) const & noexcept> : true_type {};
template<class Ret, class... Args>
struct is_function<Ret(Args...) volatile & noexcept> : true_type {};
template<class Ret, class... Args>
struct is_function<Ret(Args...) const volatile & noexcept> : true_type {};
template<class Ret, class... Args>
struct is_function<Ret(Args......) & noexcept> : true_type {};
template<class Ret, class... Args>
struct is_function<Ret(Args......) const & noexcept> : true_type {};
template<class Ret, class... Args>
struct is_function<Ret(Args......) volatile & noexcept> : true_type {};
template<class Ret, class... Args>
struct is_function<Ret(Args......) const volatile & noexcept> : true_type {};
template<class Ret, class... Args>
struct is_function<Ret(Args...) && noexcept> : true_type {};
template<class Ret, class... Args>
struct is_function<Ret(Args...) const && noexcept> : true_type {};
template<class Ret, class... Args>
struct is_function<Ret(Args...) volatile && noexcept> : true_type {};
template<class Ret, class... Args>
struct is_function<Ret(Args...) const volatile && noexcept> : true_type {};
template<class Ret, class... Args>
struct is_function<Ret(Args......) && noexcept> : true_type {};
template<class Ret, class... Args>
struct is_function<Ret(Args......) const && noexcept> : true_type {};
template<class Ret, class... Args>
struct is_function<Ret(Args......) volatile && noexcept> : true_type {};
template<class Ret, class... Args>
struct is_function<Ret(Args......) const volatile && noexcept> : true_type {};
#endif
