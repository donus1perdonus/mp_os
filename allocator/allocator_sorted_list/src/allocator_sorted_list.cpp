#include <not_implemented.h>

#include "../include/allocator_sorted_list.h"

allocator_sorted_list::~allocator_sorted_list()
{
    log_with_guard(get_typename() + 
    " allocator_sorted_list::~allocator_sorted_list()", logger::severity::debug);
}

allocator_sorted_list::allocator_sorted_list(
    allocator_sorted_list const &other)
{
    log_with_guard(get_typename() + 
    " allocator_sorted_list::allocator_sorted_list(allocator_sorted_list const &)", logger::severity::debug);
}

allocator_sorted_list &allocator_sorted_list::operator=(
    allocator_sorted_list const &other)
{
    log_with_guard(get_typename() + 
    " allocator_sorted_list &allocator_sorted_list::operator=(allocator_sorted_list const &)", logger::severity::debug);
}

allocator_sorted_list::allocator_sorted_list(
    allocator_sorted_list &&other) noexcept
{
   log_with_guard(get_typename() + 
   " allocator_sorted_list::allocator_sorted_list(allocator_sorted_list &&) noexcept", logger::severity::debug);
}

allocator_sorted_list &allocator_sorted_list::operator=(
    allocator_sorted_list &&other) noexcept
{
   log_with_guard(get_typename() + 
   " allocator_sorted_list &allocator_sorted_list::operator=(allocator_sorted_list &&) noexcept", logger::severity::debug);
}

allocator_sorted_list::allocator_sorted_list(
    size_t space_size,
    allocator *parent_allocator,
    logger *logger,
    allocator_with_fit_mode::fit_mode allocate_fit_mode)
{
   log_with_guard(get_typename() + 
   " allocator_sorted_list::allocator_sorted_list(size_t, allocator *, logger *, allocator_with_fit_mode::fit_mode)", logger::severity::debug);
}

[[nodiscard]] void *allocator_sorted_list::allocate(
    size_t value_size,
    size_t values_count)
{
   log_with_guard(get_typename() + 
   " [[nodiscard]] void *allocator_sorted_list::allocate(size_t, size_t)", logger::severity::debug);
}

void allocator_sorted_list::deallocate(
    void *at)
{
   log_with_guard(get_typename() + 
   " void allocator_sorted_list::deallocate(void *)", logger::severity::debug);
}

inline void allocator_sorted_list::set_fit_mode(
    allocator_with_fit_mode::fit_mode mode)
{
   log_with_guard(get_typename() + 
   " inline void allocator_sorted_list::set_fit_mode(allocator_with_fit_mode::fit_mode)", logger::severity::debug);
}

inline allocator *allocator_sorted_list::get_allocator() const
{
   log_with_guard(get_typename() + 
   " inline allocator *allocator_sorted_list::get_allocator() const", logger::severity::debug);
}

std::vector<allocator_test_utils::block_info> allocator_sorted_list::get_blocks_info() const noexcept
{
   log_with_guard(get_typename() + 
   " std::vector<allocator_test_utils::block_info> allocator_sorted_list::get_blocks_info() const noexcept", logger::severity::debug);
}

inline logger *allocator_sorted_list::get_logger() const
{
   log_with_guard(get_typename() + 
   " inline logger *allocator_sorted_list::get_logger() const", logger::severity::debug);
}

inline std::string allocator_sorted_list::get_typename() const noexcept
{
    return "[allocator_sorted_list]";
}