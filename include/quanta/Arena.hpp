#pragma once

#include "Common.hpp"
#include <cstddef>
#include <cstdint>
#include <utility>

namespace quanta {

/**
 * @brief Fast bump-pointer allocator (linear/region allocator)
 */
class Arena {
private:
    char* buffer_;
    size_t capacity_;
    size_t pos_;

public:

    /**
     * @brief Construct an empty arena
     */
    Arena() noexcept : buffer_(nullptr), capacity_(0), pos_(0) {}

    /**
     * @brief Construct an arena with the given capacity
     * 
     * @param capacity Size of the arena in bytes
     */
    explicit Arena(size_t capacity)
        : capacity_(capacity),
          pos_ (0)
    {
        buffer_ = reinterpret_cast<char*>( ::operator new(capacity) );
    }

    /**
     * @brief Destructor - frees the arena's memory
     */
    ~Arena() {
        if (buffer_ != nullptr) {
            ::operator delete(buffer_);
            buffer_ = nullptr;
            pos_ = 0;
            capacity_ = 0;
        }
    }

    // Arenas should not be copied
    Arena(const Arena&) = delete;
    Arena& operator=(const Arena&) = delete;

    // Arenas can be moved

    Arena(Arena&& other) noexcept 
        : buffer_(std::exchange(other.buffer_, nullptr)),
          capacity_(std::exchange(other.capacity_, 0)),
          pos_(std::exchange(other.pos_, 0))
    {    }

    Arena& operator=(Arena&& other) noexcept {
        if (this != &other) {
            if (buffer_) {
                ::operator delete(buffer_);
            }
            
            buffer_ = std::exchange(other.buffer_, nullptr);
            capacity_ = std::exchange(other.capacity_, 0);
            pos_ = std::exchange(other.pos_, 0);
        }
        return *this;
    }

    /**
     * @brief Allocate memory with the given size and alignment
     * 
     * @param size Number of bytes to allocate
     * @param alignment Alignment requirement (must be power of 2)
     * @return Pointer to allocated memory or nullptr if out of memory
     */
    void* allocate(size_t size, size_t alignment) noexcept {
        if (!is_power_of_2(alignment) || size == 0)
            return nullptr;

        // Align position
        size_t aligned_pos = align_up(pos_, alignment);

        // Check for overflow and out of memory
        if (aligned_pos + size < aligned_pos || aligned_pos + size > capacity_) [[unlikely]]
            return nullptr;

        pos_ = aligned_pos + size;

        return static_cast<void*>(buffer_ + aligned_pos);
    }

    /**
     * @brief Type-safe allocation for objects of type T
     * 
     * @tparam T Type to allocate
     * @param count Number of objects to allocate (default 1)
     * @return Pointer to allocated objects or nullptr if out of memory
     */
    template<typename T>
    T* allocate(size_t count = 1) noexcept {
        if (count > SIZE_MAX / sizeof(T))
            return nullptr;
        
        size_t total_size = sizeof(T) * count;

        return static_cast<T*>(allocate(total_size, alignof(T))); 
    }

    /**
     * @brief Reset the arena, making all allocated memory available for reuse
     */
    void reset() noexcept {
        pos_ = 0;
    }

    /**
     * @brief Get the number of bytes allocated
     */
    size_t used() const noexcept {
        return pos_;
    }

    /**
     * @brief Get the total capacity of the arena
     */
    size_t capacity() const noexcept {
        return capacity_;
    }

    /**
     * @brief Get the number of bytes available for allocation
     */
    size_t available() const noexcept {
        return (capacity_ - pos_);
    }

    /**
     * @brief Check if ptr belongs to arena
     * 
     * @param ptr Pointer to check
     */
    bool owns(void* ptr) const noexcept {
        return ptr >= buffer_ && ptr < buffer_ + capacity_;
    }
};

} // namespace quanta
