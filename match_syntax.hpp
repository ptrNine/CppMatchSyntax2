#pragma once
#include <array>
#include <exception>
#include <type_traits>
#include <functional>

namespace scl {
    template <typename T = void>
    struct Void { using type = void; };

    template <typename... ArgsT>
    struct arg_types : std::true_type {
        static constexpr size_t arg_count = sizeof...(ArgsT);
    };

    template <typename T, typename = void>
    struct is_function : std::false_type {};

    template <typename FunctorT>
    struct is_function<FunctorT, typename Void<decltype(&FunctorT::operator())>::type>
            : public is_function<decltype(&FunctorT::operator())> {};

    template <typename FunctorT, typename ReturnT, typename... ArgsT>
    struct is_function<ReturnT(FunctorT::*)(ArgsT...)> : arg_types<ArgsT...> {};

    template <typename FunctorT, typename ReturnT, typename... ArgsT>
    struct is_function<ReturnT(FunctorT::*)(ArgsT...) const> : arg_types<ArgsT...> {};

    template <typename ReturnT, typename... ArgsT>
    struct is_function<ReturnT(ArgsT...)> : arg_types<ArgsT...> {};

    template <typename ReturnT, typename... ArgsT>
    struct is_function<ReturnT(*)(ArgsT...)> : arg_types<ArgsT...> {};

    template <typename T>
    inline constexpr bool is_function_v = is_function<T>::value;

    template <typename T>
    inline constexpr size_t args_count_of_v = is_function<T>::arg_count;


    template <typename T, typename CondT>
    bool _condition_check(const T& pattern, const CondT& condition) {
        if constexpr (std::is_convertible_v<T, CondT>) {
            return pattern == condition;
        } else if constexpr (std::is_convertible_v<CondT, T>) {
            return condition == pattern;
        } else
            return condition(pattern);
    }

    template <typename T, typename CondT, typename CaseT, typename... ArgsT>
    auto _match_process(const T& pattern, const CondT& condition, const CaseT& case0, ArgsT&&... cases) {
        if (_condition_check(pattern, condition)) {
            if constexpr (is_function_v<CaseT>)
                return case0();
            else
                return case0;
        }
        else {
            if constexpr (sizeof...(ArgsT) < 2) {
                throw std::runtime_error("Unhandled");

                if constexpr (is_function_v<CaseT>)
                    return case0();
                else
                    return case0;
            }
            else
                return _match_process(pattern, std::forward<ArgsT>(cases)...);
        }
    }

    template <typename T, typename... ArgsT>
    auto match(const T& pattern, ArgsT&&... cases) {
        return _match_process(pattern, std::forward<ArgsT>(cases)...);
    }
}

#define doo , [&]()
#define ret ,

#define cond(...) \
    [&]([[maybe_unused]] const auto& it) -> bool { return ( __VA_ARGS__); }

#define condz(...) cond(it __VA_ARGS__)
