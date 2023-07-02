#include "./filtered_string_view.h"

#include <catch2/catch.hpp>
#include <cctype>
#include <compare>
#include <cstddef>
#include <set>
#include <stdexcept>
#include <string>
#include <vector>
#include <iterator>
#include <bits/stdc++.h>

TEST_CASE("default_predicate always returns true") {
  for (char c = std::numeric_limits<char>::min(); c != std::numeric_limits<char>::max(); c++) {
    CHECK(fsv::filtered_string_view::default_predicate(c));
  }
}

TEST_CASE("Default Constructor") {
  const auto sv = fsv::filtered_string_view{};
  CHECK(sv.size() == 0);
  CHECK(sv.data() == nullptr);
}

TEST_CASE("Implicit String Constructor") {
  const auto s = std::string{"cat"};
  const auto sv = fsv::filtered_string_view{s};
  CHECK(sv.size() == 3);
}

TEST_CASE("String Constructor with Predicate") {
  const auto s = std::string{"cat"};
  const auto pred = [](const char& c) { return c == 'a'; };
  const auto sv = fsv::filtered_string_view{s, pred};
  CHECK(sv.size() == 1);
}

TEST_CASE("Implicit Null-Terminated String Constructor") {
  const auto sv = fsv::filtered_string_view{"cat"};
  CHECK(sv.size() == 3);
}

TEST_CASE("Null-Terminated String with Predicate Constructor") {
  const auto pred = [](const char &c) { return c == 'a'; };
  const auto sv = fsv::filtered_string_view{"cat", pred};
  CHECK(sv.size() == 1);
}

TEST_CASE("Copy constructor") {
  const auto sv = fsv::filtered_string_view{"bulldog"};
  const auto copy = sv;
  CHECK(copy == sv);
}

TEST_CASE("Move constructor") {
  auto sv = fsv::filtered_string_view{"bulldog"};
  const auto data = sv.data();
  const auto size = sv.size();
  const auto move = std::move(sv);
  CHECK(move.data() == data);
  CHECK(move.size() == size);
  CHECK(sv.data() == nullptr);
  CHECK(sv.size() == 0);
}

TEST_CASE("Copy Assignment") {
  const auto pred = [](const char &c) { return c == '4' || c == '2'; };
  const auto fsv1 = fsv::filtered_string_view{"42 bro", pred};
  auto fsv2 = fsv::filtered_string_view{""};
  fsv2 = fsv1;
  CHECK(fsv1 == fsv2);

  // Checking that self copy doesn't change fsv2
  fsv2 = fsv2;
  CHECK(fsv1 == fsv2);
}

TEST_CASE("Move Assignment") {
  const auto pred = [](const char &c) { return c == '8' || c == '9'; };
  auto fsv1 = fsv::filtered_string_view{"'89 baby", pred};
  auto fsv2 = fsv::filtered_string_view{"hello"};
  fsv2 = std::move(fsv1);
  CHECK(fsv2 == "89");
  const auto temp = fsv2;
  CHECK(fsv1.size() == 0);
  CHECK(fsv1.data() == nullptr);

  // Checking that self move doesn't change fsv2
  fsv2 = std::move(fsv2);
  CHECK(fsv2 == temp);

}

TEST_CASE("Subscript operator") {
  const auto pred = [](const char &c) { return c == '9' || c == '0' || c == 'o'; };
  const auto fsv1 = fsv::filtered_string_view{"only 90s kids understand", pred};  
  CHECK(fsv1[0] == 'o');
  CHECK(fsv1[1] == '9');
  CHECK(fsv1[2] == '0');
}

TEST_CASE("Equality with different initializations") {
  const auto pred = [](const char &c) { return c == '9' || c == '0' || c == 'o'; };
  const auto str = std::string{"only 90s kids understand"};
  const auto fsv1 = fsv::filtered_string_view{str, pred};
  const auto fsv2 = fsv::filtered_string_view{"only 90s kids understand", pred};
  const auto fsv3 = fsv::filtered_string_view{"only 90s kids understand 9", pred};
  CHECK(fsv1 == fsv2);
  CHECK(fsv1 == fsv1);
  CHECK(fsv1 != fsv3);
  CHECK(fsv2 != fsv3);
}

TEST_CASE("String type Conversion") {
  const auto sv = fsv::filtered_string_view("vizsla");
  const auto s = static_cast<std::string>(sv);
  CHECK(s == "vizsla");
  CHECK(sv.data() != s.data());
}

TEST_CASE("String type Conversion with predicate") {
  const auto pred = [](const char &c) { return c == 'a' || c == 'z' || c == 'v'; };
  const auto sv = fsv::filtered_string_view("vizsla", pred);
  const auto s = static_cast<std::string>(sv);
  CHECK(s == "vza");
  CHECK(sv.data() != s.data());
}

TEST_CASE("at() member function") {
  const auto vowels = std::set<char>{'a', 'A', 'e', 'E', 'i', 'I', 'o', 'O', 'u', 'U'};
  const auto is_vowel = [&vowels](const char &c){ return vowels.contains(c); };
  const auto sv = fsv::filtered_string_view{"Malamute", is_vowel};
  CHECK(sv.at(0) == 'a');
  CHECK(sv.at(1) == 'a');
  CHECK(sv.at(2) == 'u');
  CHECK(sv.at(3) == 'e');
  CHECK_THROWS_AS(sv.at(-1), std::domain_error);
  CHECK_THROWS_AS(sv.at(4), std::domain_error);
}

TEST_CASE("empty() member function without filter") {
  const auto sv = fsv::filtered_string_view{"Australian Shephard"};
  const auto empty_sv = fsv::filtered_string_view{};
  CHECK(!sv.empty());
  CHECK(empty_sv.empty());
  const auto empty_sv_with_pred = fsv::filtered_string_view{"Dogs", [](const char &c){ (void)c; return false; }};
  CHECK(empty_sv_with_pred.empty());
}

TEST_CASE("empty() member function with filter") {
  const auto empty_sv_with_pred = fsv::filtered_string_view{"Dogs", [](const char &c){ (void)c; return false; }};
  CHECK(empty_sv_with_pred.empty());
}

TEST_CASE("data() member function") {
  const auto s = "Sum";
  const auto sv = fsv::filtered_string_view{s, [](const char &c){ (void)c; return false; }};
  auto ptr = sv.data();
  CHECK(*ptr == 'S');
  ++ptr;
  CHECK(*ptr == 'u');
  ++ptr;
  CHECK(*ptr == 'm');
  ++ptr;
  CHECK(*ptr == '\0');
}

TEST_CASE("predicate() member function") {
  const auto pred = [](const char &c) {
    return c == 'o';
  };
  const auto sv1 = fsv::filtered_string_view{"dog", pred};
  const auto predicate1 = sv1.predicate();
  CHECK(predicate1(char{'o'}));
  CHECK(!predicate1(char{'f'}));

  const auto sv2 = sv1;
  const auto predicate2 = sv1.predicate();
  CHECK(predicate2(char{'o'}));
  CHECK(!predicate2(char{'f'}));
}

TEST_CASE("Relational operations: fsv are of same size") {
  const auto lo = fsv::filtered_string_view{"aaa"};
  const auto hi = fsv::filtered_string_view{"zzz"};

  CHECK(lo < hi);
  CHECK(lo <= hi);
  CHECK(!(lo > hi));
  CHECK(!(lo >= hi));
  CHECK(lo <=> hi == std::strong_ordering::less);
}

TEST_CASE("Relational operations: fsv are of different size and lo is prefix of hi") {
  const auto lo = fsv::filtered_string_view{"aaa"};
  const auto hi = fsv::filtered_string_view{"aaaa"};

  CHECK(lo < hi);
  CHECK(lo <= hi);
  CHECK(!(lo > hi));
  CHECK(!(lo >= hi));
  CHECK(lo <=> hi == std::strong_ordering::less);
}

TEST_CASE("Relational operations: fsv are of different size") {
  const auto lo = fsv::filtered_string_view{"aaa"};
  const auto hi = fsv::filtered_string_view{"z"};

  CHECK(lo < hi);
  CHECK(lo <= hi);
  CHECK(!(lo > hi));
  CHECK(!(lo >= hi));
  CHECK(lo <=> hi == std::strong_ordering::less);
}

TEST_CASE("Relational operations with nullptr") {
  const auto lo = fsv::filtered_string_view{};
  const auto hi = fsv::filtered_string_view{"adam"};

  CHECK(!(lo == hi));
  CHECK(lo != hi);
  CHECK(lo < hi);
  CHECK(lo <= hi);
  CHECK(!(lo > hi));
  CHECK(!(lo >= hi));
  CHECK(lo <=> hi == std::strong_ordering::less);
}

TEST_CASE("Relational operations with nullptr and empty string") {
  const auto lhs = fsv::filtered_string_view{};
  const auto rhs = fsv::filtered_string_view{""};
  CHECK(lhs != rhs);
  CHECK(!(lhs < rhs));
  CHECK(lhs <= rhs);
   CHECK(lhs <=> rhs == std::strong_ordering::equivalent);
}

TEST_CASE("output stream") {
  const auto fsv = fsv::filtered_string_view{"c++ > rust > java", [](const char &c){ return c == 'c' || c == '+'; }};
  std::stringstream ss;
  ss << fsv;
  CHECK(ss.str() == "c++");
}

TEST_CASE("compose function: fsv has default predicate") {
  const auto fact = fsv::filtered_string_view{"Adam Chen is cool"};
  const auto vf = std::vector<fsv::filter>{
    [](const char &c){ return c == 'A' || c == 'd' || c == 'C'; },
    [](const char &c){ return c == 'd' || c == 'c'; },
    [](const char &c){ return std::islower(c); }
  };
  const auto sv = fsv::compose(fact, vf);
  const auto expected = fsv::filtered_string_view{"d"};
  CHECK(sv == expected);
}

TEST_CASE("compose function: fsv has default predicate and filters vector is empty") {
  const auto fact = fsv::filtered_string_view{"Adam Chen is cool"};
  const auto vf = std::vector<fsv::filter>{};
  const auto sv = fsv::compose(fact, vf);
  const auto expected = fsv::filtered_string_view{"Adam Chen is cool"};
  CHECK(sv == expected);
}

TEST_CASE("compose function: fsv has custom predicate") {
  const auto is_lower = [](const char &c) { return std::islower(static_cast<unsigned char>(c));};
  const auto best_languages = fsv::filtered_string_view{"CD / c++", is_lower};
  const auto vf = std::vector<fsv::filter>{
    [](const char &c){ return c == 'c' || c == '+' || c == '/'; },
    [](const char &c){ return c > ' '; }
  };
  const auto expected = fsv::filtered_string_view{"/c++"};
  CHECK(fsv::compose(best_languages, vf) == expected);
}

TEST_CASE("compose function: fsv has custom predicate and filters vector is empty") {
  const auto is_lower = [](const char &c) { return std::islower(static_cast<unsigned char>(c));};
  const auto best_languages = fsv::filtered_string_view{"CD / c++", is_lower};
  const auto vf = std::vector<fsv::filter>{};
  const auto expected = fsv::filtered_string_view{"CD / c++"};
  CHECK(fsv::compose(best_languages, vf) == expected);
}

TEST_CASE("substr function on fsv with default predicate") {
  const auto sv = fsv::filtered_string_view{"Adam Chen"};
  CHECK(fsv::substr(sv, 5) == "Chen");
}

TEST_CASE("substr function on fsv with custom predicate") {
  const auto is_upper = [](const char &c) { return std::isupper(static_cast<unsigned char>(c));};
  const auto sv1 = fsv::filtered_string_view{"Sled Dog Do No Wrong", is_upper};
  CHECK(fsv::substr(sv1, 0, 2) == "SD");
  CHECK(fsv::substr(sv1, 1, 2) == "DD");

  const auto interest = std::set<char>{'a', 'A', 'b', 'B', 'c', 'C', 'd', 'D', 'e', 'E', 'f', 'F', ' ', '/'};
  const auto sv2 = fsv::filtered_string_view{"0xDEADBEEF / 0xdeadbeef", [&interest](const char &c){ return interest.contains(c); }};
  CHECK(fsv::substr(sv2, 0, 8) == "DEADBEEF");
  CHECK(fsv::substr(sv2, 11, 19) == "deadbeef");
}

TEST_CASE("substr function producing empty string") {
  const auto is_upper = [](const char &c) { return std::isupper(static_cast<unsigned char>(c));};
  const auto sv = fsv::filtered_string_view{"abcdefgh", is_upper};
  const auto expected = fsv::filtered_string_view{""};
  CHECK(fsv::substr(sv, 0, 0) == expected);
}

TEST_CASE("substr function returns single char") {
  const auto sv = fsv::filtered_string_view{"xax"};
  const auto expected = fsv::filtered_string_view{"a"};
  CHECK(fsv::substr(sv, 1, 1) == expected);
}

TEST_CASE("substr function returns entire string view") {
  const auto sv = fsv::filtered_string_view{"adam"};
  const auto expected = fsv::filtered_string_view{"adam"};
  CHECK(fsv::substr(sv, 0, 4) == expected);
}

TEST_CASE("split function with delimiter of size 0") {
  const auto interest = std::set<char>{'a', 'A', 'b', 'B', 'c', 'C', 'd', 'D', 'e', 'E', 'f', 'F', ' ', '/'};
  const auto sv = fsv::filtered_string_view{"0xDEADBEEF/0xdeadbeef", [&interest](const char &c){ return interest.contains(c); }};
  const auto tok = fsv::filtered_string_view{""};
  const auto v = fsv::split(sv, tok);
  const auto expected = std::vector<fsv::filtered_string_view>{"DEADBEEF/deadbeef"};
  CHECK(v == expected);
}

TEST_CASE("split function with delimiter of size 1") {
  const auto interest = std::set<char>{'a', 'A', 'b', 'B', 'c', 'C', 'd', 'D', 'e', 'E', 'f', 'F', ' ', '/'};
  const auto sv = fsv::filtered_string_view{"0xDEADBEEF/0xdeadbeef", [&interest](const char &c){ return interest.contains(c); }};
  const auto tok = fsv::filtered_string_view{"/"};
  const auto v = fsv::split(sv, tok);
  const auto expected = std::vector<fsv::filtered_string_view>{"DEADBEEF", "deadbeef"};
  CHECK(v == expected);
}

TEST_CASE("split function with delimiter of size > 1") {
  const auto interest = std::set<char>{'a', 'A', 'b', 'B', 'c', 'C', 'd', 'D', 'e', 'E', 'f', 'F', ' ', '/'};
  const auto sv = fsv::filtered_string_view{"0xDEADBEEF / 0xdeadbeef", [&interest](const char &c){ return interest.contains(c); }};
  const auto tok = fsv::filtered_string_view{" / "};
  const auto v = fsv::split(sv, tok);
  const auto expected = std::vector<fsv::filtered_string_view>{"DEADBEEF", "deadbeef"};
  CHECK(v == expected);
}

TEST_CASE("split function with multiple instances of delimiter in fsv") {
  const auto interest = std::set<char>{'a', 'A', 'b', 'B', 'c', 'C', 'd', 'D', 'e', 'E', 'f', 'F', ' ', '/'};
  const auto sv = fsv::filtered_string_view{"0xDEA / DBEEF / 0xde / adbeef", [&interest](const char &c){ return interest.contains(c); }};
  const auto tok = fsv::filtered_string_view{" / "};
  const auto v = fsv::split(sv, tok);
  const auto expected = std::vector<fsv::filtered_string_view>{"DEA", "DBEEF", "de", "adbeef"};
  CHECK(v == expected);
}

TEST_CASE("split function returns empty strings") {
  const auto sv = fsv::filtered_string_view{"xax"};
  const auto tok  = fsv::filtered_string_view{"x"};
  const auto v = fsv::split(sv, tok);
  const auto expected = std::vector<fsv::filtered_string_view>{"", "a", ""};
  CHECK(v == expected);
}

TEST_CASE("split function returns all empty strings") {
  const auto sv = fsv::filtered_string_view{"xxx"};
  const auto tok  = fsv::filtered_string_view{"x"};
  const auto v = fsv::split(sv, tok);
  const auto expected = std::vector<fsv::filtered_string_view>{"", "", "", ""};
  CHECK(v == expected);
}

TEST_CASE("split function when delimiter doesn't exist") {
  const auto sv = fsv::filtered_string_view{"xax"};
  const auto tok  = fsv::filtered_string_view{"hello"};
  const auto v = fsv::split(sv, tok);
  const auto expected = std::vector<fsv::filtered_string_view>{"xax"};
  CHECK(v == expected);
}

TEST_CASE("split function with empty fsv") {
  const auto sv = fsv::filtered_string_view{""};
  const auto tok = fsv::filtered_string_view{"adam"};
  const auto v = fsv::split(sv, tok);
  const auto expected = std::vector<fsv::filtered_string_view>{""};
  CHECK(v == expected);
}

TEST_CASE("Iterators satisfy bidirectional properties") {
  CHECK(std::bidirectional_iterator<fsv::filtered_string_view::iterator>);
  CHECK(std::bidirectional_iterator<fsv::filtered_string_view::const_iterator>);
}

TEST_CASE("Iterator with empty string view") {
  const auto fsv1 = fsv::filtered_string_view{""};
  CHECK(fsv1.begin() == fsv1.end());
  const auto fsv2 = fsv::filtered_string_view{};
  CHECK(fsv2.begin() == fsv2.end());
  const auto is_upper = [](const char &c) { return std::isupper(static_cast<unsigned char>(c));};
  const auto fsv3 = fsv::filtered_string_view{"abcdefgh", is_upper};
  CHECK(fsv3.begin() == fsv3.end());
}

TEST_CASE("Iterator with default predicate") {
  // Forwards iteration from begin()
  const auto fsv = fsv::filtered_string_view{"adam"};
  auto iter = fsv.begin();
  CHECK(*iter == 'a');
  iter++;
  CHECK(*iter == 'd');
  iter++;
  CHECK(*iter == 'a');
  iter++;
  CHECK(*iter == 'm');
  iter++;
  CHECK(iter == fsv.end());
  iter--;
  CHECK(*iter == 'm');
  iter--;
  CHECK(*iter == 'a');
  iter--;
  CHECK(*iter == 'd');
  iter--;
  CHECK(*iter == 'a');
  CHECK(iter == fsv.begin());
}

TEST_CASE("Iterator with custom predicate") {
  const auto fsv = fsv::filtered_string_view{"asamoyed", [](const char &c) {
  return !(c == 's' || c == 'a' || c == 'e' || c == 'i' || c == 'o' || c == 'u');
  }};
  auto iter = fsv.begin();
  CHECK(*iter == 'm');
  iter++;
  CHECK(*iter == 'y');
  iter++;
  CHECK(*iter == 'd');
  iter++;
  CHECK(iter == fsv.end());
  iter--;
  CHECK(*iter == 'd');
  iter--;
  CHECK(*iter == 'y');
  iter--;
  CHECK(*iter == 'm');
  CHECK(iter == fsv.begin());
}

TEST_CASE("Iterator with custom predicate: filtered string contains first and last characters of undelying string") {
  const auto fsv = fsv::filtered_string_view{"asmoyed", [](const char &c) {
  return !(c == 's' || c == 'e' || c == 'i' || c == 'o' || c == 'u');
  }};
  auto iter = fsv.begin();
  CHECK(*iter == 'a');
  iter++;
  CHECK(*iter == 'm');
  iter++;
  CHECK(*iter == 'y');
  iter++;
  CHECK(*iter == 'd');
  iter++;
  CHECK(iter == fsv.end());
  iter--;
  CHECK(*iter == 'd');
  iter--;
  CHECK(*iter == 'y');
  iter--;
  CHECK(*iter == 'm');
  iter--;
  CHECK(*iter == 'a');
  CHECK(iter == fsv.begin());
}

TEST_CASE("Reverse Iterator with default predicate") {
  const auto fsv = fsv::filtered_string_view{"adam"};
  auto iter = fsv.rbegin();
  CHECK(*iter == 'm');
  iter++;
  CHECK(*iter == 'a');
  iter++;
  CHECK(*iter == 'd');
  iter++;
  CHECK(*iter == 'a');
  iter++;
  CHECK(iter == fsv.rend());
  iter--;
  CHECK(*iter == 'a');
  iter--;
  CHECK(*iter == 'd');
  iter--;
  CHECK(*iter == 'a');
  iter--;
  CHECK(*iter == 'm');
  CHECK(iter == fsv.rbegin());
}

TEST_CASE("Reverse Iterator with custom predicate") {
  const auto fsv = fsv::filtered_string_view{"asamoyed", [](const char &c) {
  return !(c == 's' || c == 'a' || c == 'e' || c == 'i' || c == 'o' || c == 'u');
  }};
  auto iter = fsv.rbegin();
  CHECK(*iter == 'd');
  iter++;
  CHECK(*iter == 'y');
  iter++;
  CHECK(*iter == 'm');
  iter++;
  CHECK(iter == fsv.rend());
  iter--;
  CHECK(*iter == 'm');
  iter--;
  CHECK(*iter == 'y');
  iter--;
  CHECK(*iter == 'd');
  CHECK(iter == fsv.rbegin());
}

TEST_CASE("Reverse Iterator with custom predicate: filtered string contains first and last characters of undelying string") {
  const auto fsv = fsv::filtered_string_view{"asmoyed", [](const char &c) {
  return !(c == 's' || c == 'e' || c == 'i' || c == 'o' || c == 'u');
  }};
  auto iter = fsv.rbegin();
  CHECK(*iter == 'd');
  iter++;
  CHECK(*iter == 'y');
  iter++;
  CHECK(*iter == 'm');
  iter++;
  CHECK(*iter == 'a');
  iter++;
  CHECK(iter == fsv.rend());
  iter--;
  CHECK(*iter == 'a');
  iter--;
  CHECK(*iter == 'm');
  iter--;
  CHECK(*iter == 'y');
  iter--;
  CHECK(*iter == 'd');
  CHECK(iter == fsv.rbegin());
}
