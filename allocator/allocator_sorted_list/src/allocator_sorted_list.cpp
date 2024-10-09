#include <not_implemented.h>

#include "../include/allocator_sorted_list.h"

allocator_sorted_list::~allocator_sorted_list() noexcept
{
    debug_with_guard(get_typename() + 
    " allocator_sorted_list::~allocator_sorted_list()");

    if (_trusted_memory == nullptr)
    {
        return;
    }

    allocator::destruct(&obtain_synchronizer());

    debug_with_guard("...");

    deallocate_with_guard(_trusted_memory);
}

allocator_sorted_list::allocator_sorted_list(
    allocator_sorted_list const &other)
{
    debug_with_guard(get_typename() + 
    " allocator_sorted_list::allocator_sorted_list(allocator_sorted_list const &)");
}

allocator_sorted_list &allocator_sorted_list::operator=(
    allocator_sorted_list const &other)
{
    debug_with_guard(get_typename() + 
    " allocator_sorted_list &allocator_sorted_list::operator=(allocator_sorted_list const &)");
}

allocator_sorted_list::allocator_sorted_list(
    allocator_sorted_list &&other) noexcept
{
    debug_with_guard(get_typename() + 
    " allocator_sorted_list::allocator_sorted_list(allocator_sorted_list &&) noexcept");
}

allocator_sorted_list &allocator_sorted_list::operator=(
    allocator_sorted_list &&other) noexcept
{
    debug_with_guard(get_typename() + 
    " allocator_sorted_list &allocator_sorted_list::operator=(allocator_sorted_list &&) noexcept");
}

allocator_sorted_list::allocator_sorted_list(
    size_t space_size,
    allocator *parent_allocator,
    logger *logger,
    allocator_with_fit_mode::fit_mode allocate_fit_mode)
{
    
    if (space_size < available_block_metadata_size())
    {
        //TODO
    }

    size_t memory_size = space_size + summ_size();
    try
    {
        _trusted_memory = parent_allocator == nullptr
                          ? ::operator new (memory_size)
                          : parent_allocator->allocate(1, memory_size);
    }
    catch (std::bad_alloc const &ex)
    {
        // TODO

        throw;
    }

    allocator **parent_allocator_placement = reinterpret_cast<allocator **>(_trusted_memory);
    *parent_allocator_placement = parent_allocator;

    class logger **logger_placement = reinterpret_cast<class logger **>(parent_allocator_placement + 1);
    *logger_placement = logger;

    std::mutex *synchronizer_placement = reinterpret_cast<std::mutex *>(logger_placement + 1);
    allocator::construct(synchronizer_placement);

    unsigned char *placement = reinterpret_cast<unsigned char *>
        (synchronizer_placement);

    placement += sizeof(std::mutex);
    *reinterpret_cast<allocator_with_fit_mode::fit_mode *>(placement) =
        allocate_fit_mode;

    placement += sizeof(allocator_with_fit_mode::fit_mode);
    *reinterpret_cast<size_t *>(placement) = space_size;

    placement += sizeof(size_t);
    *reinterpret_cast<void **>(placement) = placement + sizeof(void *);

    *reinterpret_cast<void **>(placement);

    *reinterpret_cast<void **>(*reinterpret_cast<void **>(placement));
    *reinterpret_cast<size_t *>(reinterpret_cast<void **>(*reinterpret_cast<void **>(placement)) + 1)
    = space_size - available_block_metadata_size();

    // TODO: написать логи для каждого шага
    debug_with_guard(get_typename() + 
    " allocator_sorted_list::allocator_sorted_list(size_t, allocator *, logger *, allocator_with_fit_mode::fit_mode)");
}

[[nodiscard]] void *allocator_sorted_list::allocate(
    size_t value_size,
    size_t values_count)
{
    debug_with_guard(get_typename() + 
    " [[nodiscard]] void *allocator_sorted_list::allocate(size_t, size_t)");
}

void allocator_sorted_list::deallocate(
    void *at)
{
    debug_with_guard(get_typename() + 
    " void allocator_sorted_list::deallocate(void *)");
}

inline void allocator_sorted_list::set_fit_mode(
    allocator_with_fit_mode::fit_mode mode)
{
    debug_with_guard(get_typename() + 
    " inline void allocator_sorted_list::set_fit_mode(allocator_with_fit_mode::fit_mode)");
}

inline allocator *allocator_sorted_list::get_allocator() const
{
    debug_with_guard(get_typename() + 
    " inline allocator *allocator_sorted_list::get_allocator() const");

    return *reinterpret_cast<allocator **>(_trusted_memory);
}

std::vector<allocator_test_utils::block_info> allocator_sorted_list::get_blocks_info() const noexcept
{
    debug_with_guard(get_typename() + 
    " std::vector<allocator_test_utils::block_info> allocator_sorted_list::get_blocks_info() const noexcept");
}

inline logger *allocator_sorted_list::get_logger() const
{
    debug_with_guard(get_typename() + 
    " inline logger *allocator_sorted_list::get_logger() const");

    return *(reinterpret_cast<logger **>(&obtain_synchronizer()) - 1);
}

inline std::string allocator_sorted_list::get_typename() const noexcept
{
    return "[allocator_sorted_list]";
}

size_t constexpr allocator_sorted_list::summ_size()
{
    return sizeof(allocator *) + sizeof(std::mutex) + sizeof(logger *) + sizeof( allocator_with_fit_mode::fit_mode) + sizeof(void *);
}

size_t constexpr allocator_sorted_list::available_block_metadata_size()
{
    return sizeof(size_t) + sizeof(void *);
}

size_t constexpr allocator_sorted_list::ancillary_block_metadata_size()
{
    return sizeof(size_t) + sizeof(void *);
}

std::mutex& allocator_sorted_list::obtain_synchronizer() const
{
    return *reinterpret_cast<std::mutex*>
        (reinterpret_cast<unsigned char*>
        (_trusted_memory) +
        sizeof(allocator *) + sizeof(logger *));
}