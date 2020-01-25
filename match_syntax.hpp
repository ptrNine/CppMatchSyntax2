#pragma once
#include <array>
#include <exception>
#include <type_traits>
#include <variant>
#include <functional>

namespace nma {
    template <typename T = void>
    struct Void { using type = void; };

    template <typename... Ts>
    struct MergeLambdas : Ts... {
        using Ts::operator()...;
    };

    template <typename... Ts>
    MergeLambdas(Ts...) -> MergeLambdas<Ts...>;

    template <typename T>
    struct MarkAsCondition : T {
        using Condition = void;
    };

    template <typename T>
    MarkAsCondition(T) -> MarkAsCondition<T>;

    template <typename T, typename U = void>
    struct is_condition {
    	static inline constexpr bool value = false;
    };

    template <typename T>
    struct is_condition<T, typename Void<typename T::Condition>::type> {
    	static inline constexpr bool value = true;
    };

    template <typename T, typename U = void>
    struct has_callable_operator {
        static inline constexpr bool value = false;
    };

    template <typename T>
    struct has_callable_operator<T, typename Void<decltype(&T::operator())>::type> {
        static inline constexpr bool value = true;
    };

    template <typename T, typename = void>
    struct is_function : std::false_type {};

    template <typename FunctorT>
    struct is_function<FunctorT, typename Void<decltype(&FunctorT::operator())>::type>
            : public is_function<decltype(&FunctorT::operator())> {};

    template <typename FunctorT, typename ReturnT, typename... ArgsT>
    struct is_function<ReturnT(FunctorT::*)(ArgsT...)> : std::true_type {};

    template <typename FunctorT, typename ReturnT, typename... ArgsT>
    struct is_function<ReturnT(FunctorT::*)(ArgsT...) const> : std::true_type {};

    template <typename ReturnT, typename... ArgsT>
    struct is_function<ReturnT(ArgsT...)> : std::true_type {};

    template <typename ReturnT, typename... ArgsT>
    struct is_function<ReturnT(*)(ArgsT...)> : std::true_type {};

    template <typename T>
    inline constexpr bool is_function_v = is_function<T>::value;

    template <typename T, template <typename...> typename T2>
    struct is_specialization_of : std::false_type {};

    template <template <typename...> typename T, typename... ArgsT>
    struct is_specialization_of<T<ArgsT...>, T>: std::true_type {};


// Variant matching

#define _GEN_OPERATOR(OP) \
    using Kek = std::decay_t<decltype(v)>; \
    if constexpr (std::is_convertible_v<Kek, T>) \
        return v OP t; \
    else if constexpr (std::is_convertible_v<T, Kek>) \
        return t OP v; \
    else \
        return false

#define _GEN_OPERATOR_LMAO(OP) \
    template <typename T> \
    bool operator OP (const T& t) const { \
        return std::visit(nma::MergeLambdas{ \
                [&](const ArgsT& v) { _GEN_OPERATOR(OP); }... , \
                [&](VariantEqOperator&& v) { return false; } \
        }, static_cast<const V&>(std::ref(*this).get())); \
    }

    template <typename... ArgsT>
    struct VariantEqOperator : public std::variant<ArgsT...> {
        using V = std::variant<ArgsT...>;
        using V::operator=;

        explicit VariantEqOperator(const V& v) : V(v) {}
        explicit VariantEqOperator(V&& v) noexcept : V(std::move(v)) {}

        _GEN_OPERATOR_LMAO(==)
        _GEN_OPERATOR_LMAO(>)
        _GEN_OPERATOR_LMAO(>=)

        template <typename T>
        bool operator!=(const T& t) const { return !operator==(t); }
        template <typename T>
        bool operator<(const T& t) const  { return !operator>=(t); }
        template <typename T>
        bool operator<=(const T& t) const { return !operator>(t); }
    };

#undef _GEN_OPERATOR
#undef _GEN_OPERATOR_LMAO



    template <typename T, typename CondT>
    bool _condition_check(const T& pattern, const CondT& condition) {
        if constexpr (std::is_invocable_v<CondT, T>) {
            return condition(pattern);
        } else if constexpr (std::is_convertible_v<T, CondT>) {
            return condition == pattern;
        } else if constexpr (std::is_convertible_v<CondT, T> || is_specialization_of<T, VariantEqOperator>::value) {
            return pattern == condition;
        } else {
            return false;
        }
    }

    template <typename T, typename CondT, typename CaseT, typename... ArgsT>
    auto _match_iteration(const T& pattern, const CondT& condition, const CaseT& case0, ArgsT&&... cases) {
        if (_condition_check(pattern, condition)) {
            if constexpr (is_function_v<CaseT>)
                return case0();
            else
                return case0;
        } else {
            if constexpr (sizeof...(ArgsT) < 2) {
                throw std::runtime_error("Unhandled");

                if constexpr (is_function_v<CaseT>)
                    return case0();
                else
                    return case0;
            } else {
                return _match_iteration(pattern, std::forward<ArgsT>(cases)...);
            }
        }
    }

    template <typename T, typename... ArgsT>
    struct car_type { using type = T; };

    template <typename T, typename... ArgsT>
    auto match(const T& pattern, ArgsT&&... cases) {
        using CondT = typename car_type<ArgsT...>::type;

        if constexpr (is_specialization_of<T, std::variant>::value) {
            if constexpr (is_function_v<CondT> && !is_condition<CondT>::value) {
                return std::visit(MergeLambdas{std::forward<ArgsT>(cases)...}, pattern);
            } else {
                return _match_iteration(VariantEqOperator(pattern), std::forward<ArgsT>(cases)...);
            }
        } else {
            return _match_iteration(pattern, std::forward<ArgsT>(cases)...);
        }
    }
}

/// Same as [&]()
#define doo , [&]()

/// Same as comma
#define ret ,

/// Same as doo { return ... ; }
#define retlazy(...) doo { return __VA_ARGS__; }

/// Set condition with 'it' argument
#define condx(...) \
    nma::MarkAsCondition{([&]([[maybe_unused]] const auto& it) -> bool { return ( __VA_ARGS__); })}

/// Super-short condition form
#define condz(...) condx(it __VA_ARGS__)
