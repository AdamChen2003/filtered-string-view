#include "./filtered_string_view.h"
#include <algorithm>
#include <cstddef>
#include <ios>
#include <string>
#include <utility>
#include <algorithm>

auto fsv::filtered_string_view::operator=(const filtered_string_view &other) noexcept -> filtered_string_view {
    if (this != &other) {
        filtered_string_view(other).swap(*this);
    }
    return *this;
}

auto fsv::filtered_string_view::operator=(filtered_string_view &&other) noexcept -> filtered_string_view {
    if (this != &other) {
        other.swap(*this);
        other.data_ = nullptr;
        other.length_ = 0;
        other.predicate_ = default_predicate;
    }
    return *this;
}

auto fsv::filtered_string_view::swap(filtered_string_view &other) noexcept -> void {
    std::swap(data_, other.data_);
    std::swap(length_, other.length_);
    std::swap(predicate_, other.predicate_);
}

auto fsv::filtered_string_view::operator[](int n) const noexcept -> const char& {
    auto temp = data_;
    auto i = 0;
    while (*temp != '\0') {
        if (predicate_(*temp)) {
            if (i == n) {
                break;
            }
            ++i;
        }
        ++temp;
    }
    return *temp;
}

fsv::filtered_string_view::operator std::string() const {
    auto string = std::string{};
    for (const auto c : *this) {
        string += c;
    }
    return string;
}

auto fsv::filtered_string_view::at(int index) const -> const char & {
    if (index < 0 || index >= static_cast<int>(size())) {
        throw std::domain_error{"filtered_string_view::at(" + std::to_string(index) + "): invalid index"};
    }
    return (*this)[index];
}

auto fsv::filtered_string_view::size() const noexcept -> std::size_t {
    if (data_ == nullptr) {
        return 0;
    }
    auto size = std::size_t{0};
    for (auto i = 0u; i < length_; ++i) {
        if (predicate_(*(data_ + i))) {
            ++size;
        }
    }
    return size;
}

auto fsv::compose(const filtered_string_view &fsv, const std::vector<filter> &filts) noexcept -> filtered_string_view {
    // Construct a new filter which combines the logical outcome of the filters in filts
    const auto pred = [filts](const char &c) -> bool {
        for (const auto &f : filts) {
            if (!f(c)) {
                return false;
            }
        }
        return true;
    };
    return(fsv::filtered_string_view(fsv.data(), pred));
}

auto fsv::split(const filtered_string_view &fsv, const filtered_string_view &tok) -> std::vector<filtered_string_view> {
    // If the tok is empty, return a copy of fsv
    if (tok.size() == 0) {
        return std::vector<filtered_string_view>{fsv};
    }
    auto delimiter_pos = std::vector<int>{0};
    find_delimiter_positions(fsv, tok, delimiter_pos);
    auto split_strings = std::vector<filtered_string_view>{};
    auto delim_iter = delimiter_pos.begin();
    // Iterating through the delimiter positions to add the substrings between each occurance of a delimiter
    while (delim_iter != delimiter_pos.end()) {
        const auto l = *delim_iter;
        ++delim_iter;
        const auto r = *delim_iter;
        ++delim_iter;
        if (r - l == 0) {
            // If two delimiter occur consecutively, add an empty fsv
            split_strings.push_back(filtered_string_view(""));
        } else {
            split_strings.push_back(substr(fsv, l, r - l));
        }
    }
    return split_strings;
}

// Adds the indexes of the beginning and end of the delimiter apperances in fsv to the delimiter_pos vector
auto fsv::find_delimiter_positions(const filtered_string_view &fsv, const filtered_string_view &tok, std::vector<int> &delimiter_pos) -> void {
    auto start = 0;
    auto index = 0;
    auto tok_iter = tok.begin();
    auto delim = false;

    // Using string matching, find all occurences of tok inside fsv
    for (const auto &c : fsv) {
        if (c == *tok_iter) {
            if (!delim) {
                start = index;
                delim = true;
            }
            ++tok_iter;
            if (tok_iter == tok.end() && delim) {
                delim = false;
                delimiter_pos.push_back(start);
                delimiter_pos.push_back(index + 1);
                tok_iter = tok.begin();
            }
        } else {
            tok_iter = tok.begin();
        }
        ++index;
    }
    delimiter_pos.push_back(index);
}

auto fsv::substr(const filtered_string_view &fsv, int pos, int count) noexcept -> filtered_string_view {
    const auto rcount = count <= 0 ? static_cast<int>(fsv.size()) - pos : count;
    const auto start = &fsv[pos];
    const auto end = &fsv[pos + rcount - 1];

    // Construct a new filter which utilises the original fsv predicate as well as checking that the given
    // char c is within the specified range
    const auto pred = [fsv, rcount, start, end](const char &c) -> bool {
        return (rcount != 0 && (&c >= start) && (&c <= end) && fsv.predicate()(c));
    };
    return filtered_string_view(fsv.data(), pred);
}