#ifndef COMP6771_ASS2_FSV_H
#define COMP6771_ASS2_FSV_H

#include <algorithm>
#include <cstddef>
#include <cstring>
#include <exception>
#include <iostream>
#include <iterator>
#include <string>
#include <vector>

namespace fsv {
    using filter = std::function<bool(const char &)>;
    class filtered_string_view {
        template <typename ValueType>
        class iter {
        friend filtered_string_view;
        public:
            using iterator_category = std::bidirectional_iterator_tag;
            using value_type = ValueType;
            using reference_type = const value_type&;
            using pointer_type = void;
            using difference_type = std::ptrdiff_t;

            iter() noexcept = default;

            auto operator*() const noexcept -> char {
                return *pointer_;
            }

            auto operator->() const noexcept -> char;

            auto operator++() noexcept -> iter& {
                ++pointer_;
                ++index_;
                // If the iterator is equal to end()
                if (index_ == size_) {
                    return *this;
                }
                while (!fsv_->predicate_(*pointer_)) {
                    ++pointer_;
                }
                return *this;
            }

            auto operator++(int) noexcept -> iter {
                auto self = *this;
                ++*this;
                return self;
            }

            auto operator--() noexcept -> iter& {
                pointer_--;
                while (!fsv_->predicate_(*pointer_)) {
                    pointer_--;
                }
                index_--;
                return *this;
            }

            auto operator--(int) noexcept -> iter {
                auto self = *this;
                --*this;
                return self;
            }

            friend auto operator==(const iter &lhs, const iter &rhs) noexcept -> bool {
                return (*(lhs.fsv_)).same_range(rhs.fsv_) && lhs.pointer_ == rhs.pointer_;
            }

            friend auto operator!=(const iter &lhs, const iter &rhs) noexcept -> bool {
                return !(lhs == rhs);
            }

        private:
            iter(const filtered_string_view *fsv, const char *pointer, const std::size_t size, const std::size_t index) noexcept:
            fsv_{fsv}, pointer_{pointer}, size_{size}, index_{index} {}

            const filtered_string_view *fsv_; // Pointer to the container being iterated over
            const char *pointer_; // Pointer to the current character during iteration
            std::size_t size_; // Size of the filtered string: stored to avoid recalculation
            std::size_t index_; // Index of the element being referenced by the iterator in the filtered string
        };

    public:
        using const_iterator = iter<const char>;
        using iterator = const_iterator;
        using reverse_iterator = std::reverse_iterator<iterator>;
        using const_reverse_iterator = std::reverse_iterator<const_iterator>;

        auto static default_predicate(const char &) noexcept -> bool {
            return true;
        }
        
        filtered_string_view() noexcept: data_{nullptr}, length_{0}, predicate_{default_predicate} {}
        
        filtered_string_view(const std::string &str, const filter &predicate = default_predicate) noexcept:
        data_{str.data()}, length_{str.size()}, predicate_{predicate} {}

        filtered_string_view(const char *str, const filter &predicate = default_predicate) noexcept:
        data_{str}, length_{strlen(str)}, predicate_{predicate} {};

        filtered_string_view(const filtered_string_view &other) noexcept = default;

        filtered_string_view(filtered_string_view &&other) noexcept : data_{std::exchange(other.data_, nullptr)}, 
        length_{std::exchange(other.length_, 0)}, predicate_{std::exchange(other.predicate_, default_predicate)} {}

        ~filtered_string_view() noexcept = default;

        auto operator=(const filtered_string_view &other) noexcept -> filtered_string_view;

        auto operator=(filtered_string_view &&other) noexcept -> filtered_string_view;

        auto operator[](int n) const noexcept -> const char&;

        // Is not noexcept because std::string constructor dynamically allocates memory
        explicit operator std::string() const;

        auto at(int index) const -> const char&;

        auto friend operator==(const filtered_string_view &lhs, const filtered_string_view &rhs) noexcept -> bool {
            if ((lhs.data_ == nullptr && rhs.data_ != nullptr && rhs.length_ == 0) ||
                (rhs.data_ == nullptr && lhs.data_ != nullptr && lhs.length_ == 0)) {
                return false;
            }
            return (lhs <=> rhs) == std::strong_ordering::equal;
        }

        auto friend operator<=>(const filtered_string_view &lhs, const filtered_string_view &rhs) noexcept -> std::strong_ordering {
            // Accounting for the comparison between fsv constructed by default and through empty filtered string
            if ((lhs.data_ == nullptr && rhs.data_ != nullptr && rhs.length_ == 0) ||
                (rhs.data_ == nullptr && lhs.data_ != nullptr && lhs.length_ == 0)) {
                return std::strong_ordering::equivalent;
            }

            // Iterating over the filtered string
            auto iter1 = lhs.begin();
            auto iter2 = rhs.begin();

            while (iter1 != lhs.end() && iter2 != rhs.end()) {
                if (*iter1 != *iter2) {
                    return (*iter1 <=> *iter2);
                }
                ++iter1;
                ++iter2;
            }

            // Comparing the lengths of the filtered strings if prior characters were equal
            if (iter1 == lhs.end() && iter2 != rhs.end()) {
                return std::strong_ordering::less;
            } else if (iter1 != lhs.end() && iter2 == rhs.end()) {
                return std::strong_ordering::greater;
            }

            return std::strong_ordering::equal;
        }

        auto friend operator<<(std::ostream &os, const filtered_string_view &fsv) noexcept -> std::ostream& {
            for (const auto c : fsv) {
                os << c;
            }
            return os;
        }

         auto data() const noexcept -> const char* {
            return data_;
        }

        auto size() const noexcept -> std::size_t;

        auto predicate() const noexcept -> const filter& {
            return predicate_;
        }

        auto empty() const noexcept -> bool {
            return size() == 0;
        }

        // A begin iterator points to the first character of the filtered string and has an index of 0
        auto begin() noexcept -> iterator {
            return size() == 0 ? iterator(this, nullptr, 0, 0) : iterator(this, &((*this)[0]), size(), 0);
        }

        // An end iterator points to one past the last character of the filtered string and has an index of size()
        auto end() noexcept -> iterator {
            return size() == 0 ? iterator(this, nullptr, 0, 0) : iterator(this, &((*this)[static_cast<int>(size()) - 1]) + 1, size(), size());
        }

        auto begin() const noexcept -> const_iterator {
            return size() == 0 ? const_iterator(this, nullptr, 0, 0) : const_iterator(this, &((*this)[0]), size(), 0);
        }

        auto end() const noexcept -> const_iterator {
            return size() == 0 ? const_iterator(this, nullptr, 0, 0) : const_iterator(this, &((*this)[static_cast<int>(size()) - 1]) + 1, size(), size());
        }

        auto cbegin() const noexcept -> const_iterator {
            return size() == 0 ? const_iterator(this, nullptr, 0, 0) : const_iterator(this, &((*this)[0]), size(), 0);
        }

        auto cend() const noexcept -> const_iterator {
            return size() == 0 ? const_iterator(this, nullptr, 0, 0) : const_iterator(this, &((*this)[static_cast<int>(size()) - 1]) + 1, size(), size());
        }

        auto rbegin() noexcept -> reverse_iterator {
            return reverse_iterator{end()};
        }

        auto rend() noexcept -> reverse_iterator {
            return reverse_iterator{begin()};
        }

        auto rbegin() const noexcept -> const_reverse_iterator {
            return const_reverse_iterator{end()};
        }

        auto rend() const noexcept -> const_reverse_iterator {
            return const_reverse_iterator{begin()};
        }

        auto crbegin() const noexcept -> const_reverse_iterator {
            return const_reverse_iterator{end()};
        }

        auto crend() const noexcept -> const_reverse_iterator {
            return const_reverse_iterator{begin()};
        }

        auto same_range(const filtered_string_view *other) const noexcept -> bool {
            return data_ == other->data_ && &predicate_ == &(other->predicate_);
        }

    private:
        const char *data_;
        std::size_t length_;
        filter predicate_;

        auto swap(filtered_string_view &other) noexcept -> void;
    };

    auto compose(const filtered_string_view &fsv, const std::vector<filter> &filts) noexcept -> filtered_string_view;

    // Split is not noexcept because it makes use of std::vector which allocates memory on the heap and also utilises 
    // push_back which can throw exceptions
    auto split(const filtered_string_view &fsv, const filtered_string_view &tok) -> std::vector<filtered_string_view>;
    auto find_delimiter_positions(const filtered_string_view &fsv, const filtered_string_view &tok, std::vector<int> &delimiter_pos) -> void;
    auto substr(const filtered_string_view &fsv, int pos = 0, int count = 0) noexcept -> filtered_string_view;
}

#endif // COMP6771_ASS2_FSV_H
