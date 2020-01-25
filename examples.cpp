#include <iostream>
#include <string>
#include <cassert>
#include "match_syntax.hpp"

using namespace std::literals;

void match_distances() {
    int year;
    std::cout << "Enter you birth year: ";
    std::cin >> year;

    auto generation = nma::match(year,
        condz(< 1928)  ret "unnamed (too old :) )",
        condz(>= 1928) ret "silent",
        condz(>= 1946) ret "boomers",
        condz(>= 1965) ret "X",
        condz(>= 1981) ret "millennials",
        condz(>= 1997) ret "Z"
    );

    std::cout << "You are belong to " << generation << " generation" << std::endl;
}


void calculator() {
    // Is number function
    auto is_number = [](const std::string& str) {
        try {
            std::stod(str);
            return true;
        } catch (...) {
            return false;
        }
    };

    // Is sign function
    auto is_sign = [](const std::string& str) {
        return str == "+" || str == "-" || str == "*" || str == "/";
    };

    std::cout << "Expression examples: \n" <<
              "+ 1 3 5 .         -> 9 \n"
              "+ 1 3 5 * 2 .     -> 18 \n"
              "+ 6 2 * 2 - 1 4 . -> 11\n"
              "Write '.' for ending \n";
    std::cout << "Enter the expression: ";

    // Read all before .
    auto strs = std::vector<std::string>();
    while(strs.empty() || strs.back() != ".") {
        strs.emplace_back();
        std::cin >> strs.back();
    }
    strs.pop_back();

    double result = 0;
    std::string operation = "+";

    for (auto& str : strs) {
        nma::match(str,
            is_number doo {
                auto num = std::stod(str);

                nma::match(operation,
                    "+" doo { result += num; },
                    "-" doo { result -= num; },
                    "*" doo { result *= num; },
                    "/" doo { result /= num; }
                );
            },
            is_sign doo { operation = str; },
            condx(true) doo {} // Do nothing if no match
        );
    }

    std::cout << "Result = " << result << std::endl;
}

void lazify_example() {
    static std::string state;

    class ExpensiveConstructor {
    public:
        ExpensiveConstructor(int) {
            state += "EXPENSIVE ";
        }
    };

    auto rc = nma::match("operation3"s,
        "operation1" ret ExpensiveConstructor(1),
        "operation2" ret ExpensiveConstructor(2),
        "operation3" ret ExpensiveConstructor(3)
    );

    // All constructors have been called :/
    assert(state == "EXPENSIVE EXPENSIVE EXPENSIVE ");

    // Fix that with "retlazy"
    state = "";

    rc = nma::match("operation3"s,
        "operation1" retlazy (ExpensiveConstructor(1)),
        "operation2" retlazy (ExpensiveConstructor(2)),
        "operation3" doo { return ExpensiveConstructor(3); } // retlazy is equivalent to this
    );

    // Only one constructor has been called
    assert(state == "EXPENSIVE ");
}


void variant_test() {
    std::variant<std::string, int, float> val;
    val = "hello";

    // For unhandled use typ(decltype(val)) or [](auto)
    auto str = nma::match(val,
        [](std::string) { return "string"; },
        [](int)         { return "int"; }
    );

    std::cout << str << std::endl; // Prints "string"

    val = 2.f;

    // It's valid!
    auto rc = nma::match(val,
        condz(== "str") doo { std::cout << "string!"; return false; },
        condz(== 2.f)   doo { std::cout << "2.0f float"; return true; },
        condx(true) ret false
    );

    assert(rc);
}

int main() {
    match_distances();
    calculator();
    lazify_example();
    variant_test();
    return 0;
}