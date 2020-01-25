#include <iostream>
#include "match_syntax.hpp"
#include "minitest.hpp"

using namespace std::literals;

using nma::match;

int main() {
    SECTION("Numbers") {
        REQUIRE("one hundred forty four"s == match(144,
            7   ret "seven",
            14  ret "fourteen",
            100 ret "one hundred",
            144 ret "one hundred forty four",
            228 ret "two hundred twenty two"
        ));

        REQUIRE("greater 0.5"s == match(0.55,
            condz(> 0.9) ret "greater 0.9",
            condz(> 0.8) ret "greater 0.8",
            condz(> 0.6) ret "greater 0.6",
            condz(> 0.5) ret "greater 0.5",
            condz(> 0.3) ret "greater 0.3"
        ));

        REQUIRE("less 0.4" == match(0.333,
            condz(< 0.3)   ret "less 0.3",
            condz(< 0.333) ret "less 0.333",
            condz(< 0.4)   ret "less 0.4"
        ));

        REQUIRE("less_eq 10" == match(10,
            condz(< 5)     ret "less 5",
            condz(<= 10.0) ret "less_eq 10"
        ));

        REQUIRE("greater_eq 15" == match(15.0,
            condz(>= 16.9) ret "greater_eq 16.9",
            condz(>= 15)   ret "greater_eq 15"
        ));

        REQUIRE("no_opt" == match(10,
            condz(== 5)  ret "equal 5",
            condz(>= 20) ret "greater_eq 20",
            condx(true)  ret "no_opt"
        ));

        REQUIRE("in_range 22.55 100" == match(22.55,
            condx(it >= 4     && it < 6)     ret "in_range 4 6",
            condx(it >= 20.5  && it < 22.55) ret "in_range 20.5 22.55",
            condx(it >= 22.55 && it < 100)   ret "in_range 22.55 100"
        ));
    }

    SECTION("With lambda") {
        std::string test;

        match(150,
            condz(< 100)  doo { test = "0, 100"; },
            condz(< 200)  doo { test = "hit!"; }
        );

        REQUIRE(test == "hit!");
    }

    SECTION("Inner match") {
        // Do not use this, inner matchs isn't lazy
        auto answ = match(228,
            4 ret match(20,
                21 ret "false",
                52 ret "false",
                condx(true) ret "false"
            ),
            228 ret match(20,
                condz(== 22) ret "false",
                condz(> 15)  ret "true",
                condx(true)  ret "false"
            )
        );

        REQUIRE("true"s == answ);

        // Lazy inner matchs via lambdas
        static std::string lazy_test;

        auto answ2 = match(228,
            4 doo {
                lazy_test += "first";
                return match(20,
                    21 ret "false",
                    52 ret "false",
                    condx(true) ret "false"
                );
            },
            228 doo {
                lazy_test += "second";
                return match(20,
                    condz(== 22) ret "false",
                    condz(> 15)  ret "true",
                    condx(true)  ret "false"
                );
            }
        );

        REQUIRE("true"s == answ2);
        REQUIRE("second"s == lazy_test);
    }

    SECTION("Strings") {
        int rc1 = match("str3"s,
            condz(== "str1") ret 1,
            condz(== "str2") doo { return 2; },
            "str3" ret 3,
            "str4" doo { return 4; }
        );

        REQUIRE(rc1 == 3);
    }

    SECTION("Variant") {
        std::variant<std::string, int, double> v;
        v = 3.8;

        REQUIRE(match(v,
            [](const std::string&) { return false; },
            [](int)    { return false; },
            [](float)  { return false; },
            [](double) { return true;  },
            [](auto)   { return false; }
        ));


        REQUIRE(match(v,
            "kek" ret false,
            3     ret false,
            3.8f  ret false, // Surprise, 3.8 != 3.8f (floating point jokes)
            3.8   ret true   // Valid
        ));


        std::variant<std::string, float, double> v2;
        v2 = 3.0f;
        REQUIRE(match(v2,
            "kek" ret false,
            3     ret true,   // Valid, first result (int not in variant types!)
            3.0   ret false,  // Also valid
            3.0f  ret false   // valid
        ));


        std::variant<std::string, const char*> v3;
        v3 = "kek";

        REQUIRE(match(v3, "kek"s ret true));
        REQUIRE(match(v3, 23 ret false, condx(true) ret true));
    }
}