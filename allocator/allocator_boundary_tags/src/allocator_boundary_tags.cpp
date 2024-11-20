#include <mutex>

#include <not_implemented.h>

#include "../include/allocator_boundary_tags.h"
#include "allocator_boundary_tags.h"

allocator_boundary_tags::~allocator_boundary_tags()
{
    throw not_implemented("allocator_boundary_tags::~allocator_boundary_tags()", "your code should be here...");
}

allocator_boundary_tags::allocator_boundary_tags(
    allocator_boundary_tags const &other)
{
    throw not_implemented("allocator_boundary_tags::allocator_boundary_tags(allocator_boundary_tags const &)", "your code should be here...");
}

allocator_boundary_tags &allocator_boundary_tags::operator=(
    allocator_boundary_tags const &other)
{
    throw not_implemented("allocator_boundary_tags &allocator_boundary_tags::operator=(allocator_boundary_tags const &)", "your code should be here...");
}

allocator_boundary_tags::allocator_boundary_tags(
    allocator_boundary_tags &&other) noexcept
{
    throw not_implemented("allocator_boundary_tags::allocator_boundary_tags(allocator_boundary_tags &&) noexcept", "your code should be here...");
}

allocator_boundary_tags &allocator_boundary_tags::operator=(
    allocator_boundary_tags &&other) noexcept
{
    throw not_implemented("allocator_boundary_tags &allocator_boundary_tags::operator=(allocator_boundary_tags &&) noexcept", "your code should be here...");
}

allocator_boundary_tags::allocator_boundary_tags(
    size_t space_size,
    allocator *parent_allocator,
    logger *logger,
    allocator_with_fit_mode::fit_mode allocate_fit_mode)
{
    throw not_implemented("allocator_boundary_tags::allocator_boundary_tags(size_t, allocator *, logger *, allocator_with_fit_mode::fit_mode)", "your code should be here...");
}

[[nodiscard]] void *allocator_boundary_tags::allocate(
    size_t value_size,
    size_t values_count)
{
    throw not_implemented("[[nodiscard]] void *allocator_boundary_tags::allocate(size_t, size_t)", "your code should be here...");
}

void allocator_boundary_tags::deallocate(
    void *at)
{
    throw not_implemented("void allocator_boundary_tags::deallocate(void *)", "your code should be here...");
}

inline void allocator_boundary_tags::set_fit_mode(
    allocator_with_fit_mode::fit_mode mode)
{
    throw not_implemented("inline void allocator_boundary_tags::set_fit_mode(allocator_with_fit_mode::fit_mode)", "your code should be here...");
}

inline allocator *allocator_boundary_tags::get_allocator() const
{
    throw not_implemented("inline allocator *allocator_boundary_tags::get_allocator() const", "your code should be here...");
}

std::vector<allocator_test_utils::block_info> allocator_boundary_tags::get_blocks_info() const noexcept
{
    throw not_implemented("std::vector<allocator_test_utils::block_info> allocator_boundary_tags::get_blocks_info() const noexcept", "your code should be here...");
}

inline logger *allocator_boundary_tags::get_logger() const
{
    throw not_implemented("inline logger *allocator_boundary_tags::get_logger() const", "your code should be here...");
}

inline std::string allocator_boundary_tags::get_typename() const noexcept
{
    throw not_implemented("inline std::string allocator_boundary_tags::get_typename() const noexcept", "your code should be here...");
}

size_t &allocator_boundary_tags::obtain_trusted_memory_size() const
{
    return *reinterpret_cast<size_t *>(&obtain_fit_mode() + 1);
}

size_t constexpr allocator_boundary_tags::common_metadata_size()
{
    return sizeof(allocator*) + sizeof(std::mutex) + sizeof(logger*) + sizeof(allocator_with_fit_mode::fit_mode) + sizeof(size_t) + sizeof(void*);
}

size_t constexpr allocator_boundary_tags::available_block_metadata_size()
{
    return sizeof(size_t) + sizeof(void*);
}

size_t constexpr allocator_boundary_tags::ancillary_block_metadata_size()
{
    // return sizeof(size_t) + sizeof(void *);
    return sizeof(block_size_t) + sizeof(void*);
}

std::mutex& allocator_boundary_tags::obtain_synchronizer() const
{
    return *reinterpret_cast<std::mutex*>(const_cast<unsigned char*>(reinterpret_cast<unsigned char const*>(_trusted_memory) + sizeof(allocator*) + sizeof(logger*)));
}

void*& allocator_boundary_tags::obtain_first_available_block_address_byref() const
{
    return *reinterpret_cast<void**>(reinterpret_cast<unsigned char*>(_trusted_memory)
                                     + sizeof(allocator*)
                                     + sizeof(logger*)
                                     + sizeof(std::mutex)
                                     + sizeof(allocator_with_fit_mode::fit_mode)
                                     + sizeof(size_t));
}

void** allocator_boundary_tags::obtain_first_available_block_address_byptr() const
{
    return reinterpret_cast<void**>(reinterpret_cast<unsigned char*>(_trusted_memory)
                                    + sizeof(allocator*)
                                    + sizeof(logger*)
                                    + sizeof(std::mutex)
                                    + sizeof(allocator_with_fit_mode::fit_mode)
                                    + sizeof(size_t));
}

void *& allocator_boundary_tags::obtain_allocator_trusted_memory_ancillary_block_owner(
    void *current_ancillary_block_address)
{
    return obtain_next_available_block_address(current_ancillary_block_address);
}

size_t &allocator_boundary_tags::obtain_ancillary_block_size(
    void *current_ancillary_block_address)
{
    return *reinterpret_cast<size_t *>(reinterpret_cast<void **>(current_ancillary_block_address) + 1);
}

void*& allocator_boundary_tags::obtain_next_available_block_address(
    void* current_available_block_address)
{
    return *reinterpret_cast<void**>(current_available_block_address);
}

size_t& allocator_boundary_tags::obtain_available_block_size(
    void* current_available_block_address)
{
    return *reinterpret_cast<size_t*>(&obtain_next_available_block_address(current_available_block_address) + 1);

    // return *reinterpret_cast<size_t *>(reinterpret_cast<void **>(current_available_block_address) + 1);

    // return *reinterpret_cast<size_t *>(reinterpret_cast<unsigned char *>(current_available_block_address) + sizeof(void *));
}

allocator_with_fit_mode::fit_mode& allocator_boundary_tags::obtain_fit_mode() const
{
    return *reinterpret_cast<allocator_with_fit_mode::fit_mode*>(reinterpret_cast<unsigned char*>(_trusted_memory) + sizeof(allocator*) + sizeof(logger*) + sizeof(std::mutex));
}

void allocator_boundary_tags::throw_if_allocator_instance_state_was_moved() const
{
    if (_trusted_memory == nullptr)
    {
        this->
            error_with_guard(get_typename() 
            + " Allocator instance state was moved :/");

        throw std::logic_error("Allocator instance state was moved :/");
    }
}