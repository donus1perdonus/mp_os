/*
Аллокатор запрашивает память вместе с метаданными, а не занимает память у блока.

Во всех функциях для работы с блоками на вход подаётся адрес на первый тег в метаданных (флаг занятости).
*/

#include <mutex>

#include <not_implemented.h>

#include "../include/allocator_boundary_tags.h"
#include "allocator_boundary_tags.h"

allocator_boundary_tags::~allocator_boundary_tags()
{
    if (_trusted_memory == nullptr)
    {
        return;
    }

    // obtain_synchronizer().~mutex();
    allocator::destruct(&obtain_synchronizer());

    this
        ->debug_with_guard(get_typename() 
        + " Destruct the object of mutex...");

    this
        //->information_with_guard("...")
        //->warning_with_guard("...")
        ->debug_with_guard(get_typename() 
        + " Destruct the object of allocator sorted list...");

    deallocate_with_guard(_trusted_memory);
}

allocator_boundary_tags::allocator_boundary_tags(
    allocator_boundary_tags &&other) noexcept
{
    this
        ->debug_with_guard(get_typename()
        + " Call of overloaded move constructor...");

    _trusted_memory = other._trusted_memory;

    this
        ->debug_with_guard(get_typename()
        + " Lock the object of synchronyzer...");

    std::lock_guard<std::mutex> lock(other.obtain_synchronizer());

    other._trusted_memory = nullptr;  
}

allocator_boundary_tags &allocator_boundary_tags::operator=(
    allocator_boundary_tags &&other) noexcept
{
    this
        ->debug_with_guard(get_typename()
        + " Call of overloaded move operator=...");

    this
        ->debug_with_guard(get_typename()
        + " Lock the object of synchronyzer...");

    std::lock_guard<std::mutex> lock(other.obtain_synchronizer());

    if (this != &other)
    {
        deallocate_with_guard(_trusted_memory);

        _trusted_memory = other._trusted_memory;

        other._trusted_memory = nullptr;
    }

    return *this;
}

allocator_boundary_tags::allocator_boundary_tags(
    size_t space_size,
    allocator *parent_allocator,
    logger *logger,
    allocator_with_fit_mode::fit_mode allocate_fit_mode)
{
    if (space_size < obtain_global_metadata_size() + obtain_block_metadata_size())
    {
        throw std::logic_error("Can't initialize allocator instance");
    }

    size_t memory_size = space_size + obtain_global_metadata_size() + obtain_block_metadata_size();

    try
    {
        _trusted_memory = parent_allocator == nullptr
                          ? ::operator new (memory_size)
                          : parent_allocator->allocate(1, memory_size);
    }
    catch (std::bad_alloc const& ex)
    {
        throw;
    }

    allocator** parent_allocator_placement = reinterpret_cast<allocator**>(_trusted_memory);
    *parent_allocator_placement = parent_allocator;
    std::cout << parent_allocator_placement << std::endl;

    class logger** logger_placement = reinterpret_cast<class logger**>(parent_allocator_placement + 1);
    *logger_placement = logger;
    std::cout << logger_placement << std::endl;

    std::mutex* synchronizer_placement = reinterpret_cast<std::mutex*>(logger_placement + 1);
    allocator::construct(synchronizer_placement);
    std::cout << synchronizer_placement << std::endl;

    unsigned char* placement = reinterpret_cast<unsigned char*>(synchronizer_placement);

    placement += sizeof(std::mutex);
    *reinterpret_cast<allocator_with_fit_mode::fit_mode*>(placement) = allocate_fit_mode;
    std::cout << sizeof(std::mutex) << " "<< reinterpret_cast<void*>(placement) << std::endl;

    placement += sizeof(allocator_with_fit_mode::fit_mode);
    *reinterpret_cast<size_t*>(placement) = space_size;
    std::cout << sizeof(allocator_with_fit_mode::fit_mode) << " " << reinterpret_cast<void*>(placement) << std::endl;

    placement += sizeof(size_t);

    set_block_metadata(placement, space_size, true);

    this
        ->debug_with_guard(get_typename() 
        + " The object of allocator was created with memory: "
        + std::to_string(memory_size)
        + " bytes");

    this
        ->information_with_guard(get_typename()
        + "The avaliable size of allocator memory is " 
        + std::to_string(space_size)
        + " bytes");
}

[[nodiscard]] void *allocator_boundary_tags::allocate(
    size_t value_size,
    size_t values_count)
{
    this->
        debug_with_guard(get_typename() 
        + " Call of the allocate...");

    this->
        information_with_guard(get_typename()
        + " Request the block with size: "
        + std::to_string(value_size * values_count)
        + " bytes");

    std::lock_guard<std::mutex> lock(obtain_synchronizer());

    this->
        trace_with_guard(get_typename()
        + " Lock the object of synchronyzer...");

    throw_if_allocator_instance_state_was_moved();

    void *target_block = nullptr, *previous_to_target_block = nullptr;
    size_t requested_size = (value_size * values_count) + obtain_block_metadata_size();
    size_t target_block_size;

    {
        void *current_block, *previous_block = nullptr;
        current_block = obtain_available_block_address();
        allocator_with_fit_mode::fit_mode fit_mode = obtain_fit_mode();

        while (current_block != nullptr)
        {
            size_t current_block_size = obtain_available_block_size(current_block);

            if (current_block_size >= requested_size && (
                fit_mode == allocator_with_fit_mode::fit_mode::first_fit ||
                (fit_mode == allocator_with_fit_mode::fit_mode::the_best_fit &&
                 (target_block == nullptr || current_block_size < target_block_size)) ||
                (fit_mode == allocator_with_fit_mode::fit_mode::the_worst_fit &&
                 (target_block == nullptr || current_block_size > target_block_size))))
            {
                target_block = current_block;
                previous_to_target_block = previous_block;
                target_block_size = current_block_size;

                if (fit_mode == allocator_with_fit_mode::fit_mode::first_fit)
                {
                    break;
                }
            }

            previous_block = current_block;
            current_block = obtain_available_block_address();
        }
    }
}

void allocator_boundary_tags::deallocate(
    void *at)
{
    throw not_implemented("void allocator_boundary_tags::deallocate(void *)", "your code should be here...");
}

inline void allocator_boundary_tags::set_fit_mode(
    allocator_with_fit_mode::fit_mode mode)
{
    std::lock_guard<std::mutex> lock(obtain_synchronizer());

    obtain_fit_mode() = mode;
}

inline allocator *allocator_boundary_tags::get_allocator() const
{
    return *reinterpret_cast<allocator**>(_trusted_memory);
}

std::vector<allocator_test_utils::block_info> allocator_boundary_tags::get_blocks_info() const noexcept
{
    throw not_implemented("std::vector<allocator_test_utils::block_info> allocator_boundary_tags::get_blocks_info() const noexcept", "your code should be here...");
}

inline logger *allocator_boundary_tags::get_logger() const
{
    return *(reinterpret_cast<logger**>(&obtain_synchronizer()) - 1);
}

inline std::string allocator_boundary_tags::get_typename() const noexcept
{
    return "[allocator_boundary_tags]";
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

void*& allocator_boundary_tags::obtain_available_block_address() const
{
    unsigned char* target_block = reinterpret_cast<unsigned char*>(_trusted_memory) + obtain_global_metadata_size(); // obtain first block metadata

    while (!(*reinterpret_cast<bool*>(target_block)))
    {
        target_block += 
            *(reinterpret_cast<size_t*>(reinterpret_cast<unsigned char*>(target_block) + sizeof(bool) + sizeof(void*))) + obtain_block_metadata_size();
    }

    return *reinterpret_cast<void**>(target_block);
}

size_t &allocator_boundary_tags::obtain_ancillary_block_size(
    void *current_ancillary_block_address)
{
    return *reinterpret_cast<size_t *>(reinterpret_cast<void **>(current_ancillary_block_address) + 1);
}

size_t& allocator_boundary_tags::obtain_available_block_size(
    void* current_available_block_address)
{
    return *reinterpret_cast<size_t*>(reinterpret_cast<unsigned char*>(current_available_block_address) + sizeof(bool) + sizeof(void*));
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

constexpr size_t allocator_boundary_tags::obtain_global_metadata_size() const
{
    return sizeof(allocator*) + sizeof(std::mutex) + sizeof(logger*) + sizeof(allocator_with_fit_mode::fit_mode) + sizeof(size_t);
}

constexpr size_t allocator_boundary_tags::obtain_block_metadata_size() const
{
    return (sizeof(size_t) + sizeof(bool) + sizeof(void*)) * 2;
}

void allocator_boundary_tags::set_block_metadata(unsigned char *placement, size_t space_size, bool is_free)
{
    *reinterpret_cast<bool*>(placement) = is_free;

    placement += sizeof(bool);
    *reinterpret_cast<void**>(placement) = _trusted_memory;

    placement += sizeof(void*);
    *reinterpret_cast<size_t*>(placement) = space_size;

    placement += *reinterpret_cast<size_t*>(placement); // move to end of block to place metadata

    *reinterpret_cast<bool*>(placement) = is_free;

    placement += sizeof(bool);
    *reinterpret_cast<void**>(placement) = _trusted_memory;

    placement += sizeof(void*);
    *reinterpret_cast<size_t*>(placement) = space_size;
}
