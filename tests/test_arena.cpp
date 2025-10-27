#include <gtest/gtest.h>
#include "quanta/Arena.hpp"

using namespace quanta;

// CONSTRUCTION AND DESTRUCTION

TEST(ArenaTest, Construction) {
    Arena arena(1024);
    
    EXPECT_EQ(arena.capacity(), 1024);
    EXPECT_EQ(arena.used(), 0);
    EXPECT_EQ(arena.available(), 1024);
}

// BASIC ALLOCATION

TEST(ArenaTest, BasicAllocation) {
    Arena arena(1024);
    
    void* p1 = arena.allocate(10, 1);
    ASSERT_NE(p1, nullptr);
    EXPECT_EQ(arena.used(), 10);
    
    void* p2 = arena.allocate(20, 1);
    ASSERT_NE(p2, nullptr);
    EXPECT_EQ(arena.used(), 30);
    
    EXPECT_NE(p1, p2);
}

TEST(ArenaTest, OutOfMemory) {
    Arena arena(100);
    
    void* p = arena.allocate(200, 1);  // too big
    EXPECT_EQ(p, nullptr);
    EXPECT_EQ(arena.used(), 0);
}

TEST(ArenaTest, ExactCapacityAllocation) {
    Arena arena(100);
    
    void* p = arena.allocate(100, 1);
    ASSERT_NE(p, nullptr);
    EXPECT_EQ(arena.used(), 100);
    EXPECT_EQ(arena.available(), 0);
    
    // No more space
    void* p2 = arena.allocate(1, 1);
    EXPECT_EQ(p2, nullptr);
}

// ALIGNMENT TESTS

TEST(ArenaTest, AlignmentCorrectness) {
    Arena arena(1024);
    
    // Test various alignments
    for (size_t align : {1, 2, 4, 8, 16, 32, 64}) {
        arena.reset();
        
        void* p = arena.allocate(1, align);
        ASSERT_NE(p, nullptr);
        
        uintptr_t addr = reinterpret_cast<uintptr_t>(p);
        EXPECT_EQ(addr % align, 0);  // must be aligned
    }
}

TEST(ArenaTest, AlignmentAfterMisalignedAllocation) {
    Arena arena(1024);
    
    // Allocate 1 byte (no special alignment)
    void* p1 = arena.allocate(1, 1);
    ASSERT_NE(p1, nullptr);
    
    // Now allocate with 8-byte alignment
    void* p2 = arena.allocate(8, 8);
    ASSERT_NE(p2, nullptr);
    
    uintptr_t addr = reinterpret_cast<uintptr_t>(p2);
    EXPECT_EQ(addr % 8, 0);  // Must be aligned despite previous allocation
}

TEST(ArenaTest, LargeAlignment) {
    Arena arena(2048);
    
    // Allocate with 512-byte alignment
    void* p = arena.allocate(1, 512);
    ASSERT_NE(p, nullptr);
    
    uintptr_t addr = reinterpret_cast<uintptr_t>(p);
    EXPECT_EQ(addr % 512, 0);
}

// RESET FUNCTIONALITY

TEST(ArenaTest, Reset) {
    Arena arena(1024);
    
    arena.allocate(100, 1);
    arena.allocate(200, 1);
    EXPECT_EQ(arena.used(), 300);
    
    arena.reset();
    EXPECT_EQ(arena.used(), 0);
    EXPECT_EQ(arena.available(), arena.capacity());
    
    // Can allocate again
    void* p = arena.allocate(100, 1);
    ASSERT_NE(p, nullptr);
    EXPECT_EQ(arena.used(), 100);
}

TEST(ArenaTest, MultipleResetCycles) {
    Arena arena(1024);
    
    for (int i = 0; i < 100; ++i) {
        arena.allocate(50, 1);
        arena.allocate(100, 1);
        EXPECT_EQ(arena.used(), 150);
        
        arena.reset();
        EXPECT_EQ(arena.used(), 0);
    }
}

// TYPED ALLOCATION

TEST(ArenaTest, TypedAllocation) {
    Arena arena(1024);
    
    int* i = arena.allocate<int>();
    ASSERT_NE(i, nullptr);
    EXPECT_EQ(reinterpret_cast<uintptr_t>(i) % alignof(int), 0);
    
    double* d = arena.allocate<double>();
    ASSERT_NE(d, nullptr);
    EXPECT_EQ(reinterpret_cast<uintptr_t>(d) % alignof(double), 0);
}

TEST(ArenaTest, ArrayAllocation) {
    Arena arena(1024);
    
    int* arr = arena.allocate<int>(10);
    ASSERT_NE(arr, nullptr);
    
    // Should be able to use as array
    for (int i = 0; i < 10; ++i) {
        arr[i] = i;
    }
    
    for (int i = 0; i < 10; ++i) {
        EXPECT_EQ(arr[i], i);
    }
}

TEST(ArenaTest, OverAlignedStruct) {
    struct alignas(128) BigAlign {
        int value;
    };
    
    Arena arena(1024);
    BigAlign* p = arena.allocate<BigAlign>();
    
    ASSERT_NE(p, nullptr);
    EXPECT_EQ(reinterpret_cast<uintptr_t>(p) % 128, 0);
}

// STRESS TESTS

TEST(ArenaTest, ManySmallAllocations) {
    Arena arena(10 * 1024 * 1024);  // 10 MB
    
    for (int i = 0; i < 100000; ++i) {
        void* p = arena.allocate(50, 8);
        ASSERT_NE(p, nullptr);
    }
}

TEST(ArenaTest, MixedSizeAllocations) {
    Arena arena(10 * 1024 * 1024);
    
    for (int i = 0; i < 10000; ++i) {
        size_t size = (i % 10 + 1) * 8;  // 8, 16, 24, ..., 80
        void* p = arena.allocate(size, 8);
        ASSERT_NE(p, nullptr);
    }
}

// MOVE SEMANTICS

TEST(ArenaTest, MoveConstruction) {
    Arena arena1(1024);
    arena1.allocate(100, 1);
    
    Arena arena2(std::move(arena1));
    
    EXPECT_EQ(arena2.capacity(), 1024);
    EXPECT_EQ(arena2.used(), 100);
    
    // arena1 should be empty
    EXPECT_EQ(arena1.capacity(), 0);
    EXPECT_EQ(arena1.used(), 0);
}

TEST(ArenaTest, MoveAssignment) {
    Arena arena1(1024);
    arena1.allocate(100, 1);
    
    Arena arena2(512);
    arena2 = std::move(arena1);
    
    EXPECT_EQ(arena2.capacity(), 1024);
    EXPECT_EQ(arena2.used(), 100);
}


// More future testing ideas:
// - Test alignment with structures of various sizes
// - Test behavior when allocation size + alignment > capacity
// - Test with custom types (classes)
// - Test thread-safety
// - Test with AddressSanitizer enabled
// - Benchmark allocation speed vs new/delete
