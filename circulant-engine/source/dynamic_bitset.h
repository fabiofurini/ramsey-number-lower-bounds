#ifndef HYBRID_GENCYC_DYNAMIC_BITSET_H
#define HYBRID_GENCYC_DYNAMIC_BITSET_H

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <vector>

class DynamicBitset {
public:
    explicit DynamicBitset(std::size_t nbits = 0)
        : nbits_(nbits), words_((nbits + 63U) / 64U, 0ULL) {}

    std::size_t nbits() const { return nbits_; }
    std::size_t nwords() const { return words_.size(); }

    std::uint64_t word(std::size_t index) const {
        assert(index < words_.size());
        return words_[index];
    }

    void clear() { std::fill(words_.begin(), words_.end(), 0ULL); }

    void set(std::size_t bit) {
        assert(bit < nbits_);
        words_[bit >> 6U] |= (1ULL << (bit & 63U));
    }

    void reset(std::size_t bit) {
        assert(bit < nbits_);
        words_[bit >> 6U] &= ~(1ULL << (bit & 63U));
    }

    bool test(std::size_t bit) const {
        assert(bit < nbits_);
        return (words_[bit >> 6U] & (1ULL << (bit & 63U))) != 0ULL;
    }

    bool empty() const {
        for (std::uint64_t word : words_) {
            if (word != 0ULL) return false;
        }
        return true;
    }

    std::size_t count() const {
        std::size_t total = 0;
        for (std::uint64_t word : words_) {
            total += static_cast<std::size_t>(__builtin_popcountll(word));
        }
        return total;
    }

    int first() const {
        for (std::size_t i = 0; i < words_.size(); ++i) {
            if (words_[i] != 0ULL) {
                return static_cast<int>(64U * i + static_cast<std::size_t>(__builtin_ctzll(words_[i])));
            }
        }
        return -1;
    }

    int pop_first() {
        for (std::size_t i = 0; i < words_.size(); ++i) {
            std::uint64_t &word = words_[i];
            if (word != 0ULL) {
                const unsigned offset = static_cast<unsigned>(__builtin_ctzll(word));
                word &= (word - 1ULL);
                return static_cast<int>(64U * i + offset);
            }
        }
        return -1;
    }

    void copy_from(const DynamicBitset &other) {
        assert_compatible(other);
        std::copy(other.words_.begin(), other.words_.end(), words_.begin());
    }

    void and_from(const DynamicBitset &left, const DynamicBitset &right) {
        assert_compatible(left);
        assert_compatible(right);
        for (std::size_t i = 0; i < words_.size(); ++i) words_[i] = left.words_[i] & right.words_[i];
    }

    void or_with(const DynamicBitset &other) {
        assert_compatible(other);
        for (std::size_t i = 0; i < words_.size(); ++i) words_[i] |= other.words_[i];
        mask_tail();
    }

    void shift_left_from(const DynamicBitset &source, std::size_t shift) {
        assert_compatible(source);
        clear();
        if (shift >= nbits_ || source.empty()) return;

        const std::size_t word_shift = shift >> 6U;
        const unsigned bit_shift = static_cast<unsigned>(shift & 63U);
        for (std::size_t i = 0; i < words_.size(); ++i) {
            const std::uint64_t value = source.words_[i];
            if (value == 0ULL) continue;
            const std::size_t destination = i + word_shift;
            if (destination < words_.size()) words_[destination] |= value << bit_shift;
            if (bit_shift != 0U && destination + 1U < words_.size()) {
                words_[destination + 1U] |= value >> (64U - bit_shift);
            }
        }
        mask_tail();
    }

    void shift_right_from(const DynamicBitset &source, std::size_t shift) {
        assert_compatible(source);
        clear();
        if (shift >= nbits_ || source.empty()) return;

        const std::size_t word_shift = shift >> 6U;
        const unsigned bit_shift = static_cast<unsigned>(shift & 63U);
        for (std::size_t i = word_shift; i < words_.size(); ++i) {
            const std::uint64_t value = source.words_[i];
            if (value == 0ULL) continue;
            const std::size_t destination = i - word_shift;
            words_[destination] |= value >> bit_shift;
            if (bit_shift != 0U && destination > 0U) {
                words_[destination - 1U] |= value << (64U - bit_shift);
            }
        }
        mask_tail();
    }

    // Rotate a logical nbits_-wide word to the left. scratch must be distinct from this/source.
    void rotate_left_from(const DynamicBitset &source, std::size_t shift, DynamicBitset &scratch) {
        assert_compatible(source);
        assert_compatible(scratch);
        if (nbits_ == 0U) return;
        shift %= nbits_;
        if (shift == 0U) {
            copy_from(source);
            return;
        }
        shift_left_from(source, shift);
        scratch.shift_right_from(source, nbits_ - shift);
        or_with(scratch);
    }

    std::vector<int> indices() const {
        DynamicBitset copy(nbits_);
        copy.copy_from(*this);
        std::vector<int> result;
        result.reserve(count());
        int bit = -1;
        while ((bit = copy.pop_first()) >= 0) result.push_back(bit);
        return result;
    }

private:
    void assert_compatible(const DynamicBitset &other) const {
        assert(nbits_ == other.nbits_);
        (void)other;
    }

    void mask_tail() {
        if (words_.empty() || (nbits_ & 63U) == 0U) return;
        words_.back() &= ((1ULL << (nbits_ & 63U)) - 1ULL);
    }

    std::size_t nbits_;
    std::vector<std::uint64_t> words_;
};

#endif
