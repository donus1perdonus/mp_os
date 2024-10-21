#include <not_implemented.h>

#include "../include/allocator_sorted_list.h"
#include "allocator_sorted_list.h"

allocator_sorted_list::~allocator_sorted_list() noexcept
{
    if (_trusted_memory == nullptr)
    {
        return;
    }

    debug_with_guard(get_typename() + 
    " allocator_sorted_list::~allocator_sorted_list()");

    allocator::destruct(&get_mutex());

    debug_with_guard("...");

    deallocate_with_guard(_trusted_memory);
}

allocator_sorted_list::allocator_sorted_list(
    allocator_sorted_list &&other) noexcept
    : _trusted_memory(other._trusted_memory)
{
    debug_with_guard(get_typename() + 
    " allocator_sorted_list::allocator_sorted_list(allocator_sorted_list &&) noexcept");

    other._trusted_memory = nullptr;
}

allocator_sorted_list &allocator_sorted_list::operator=(
    allocator_sorted_list &&other) noexcept
{
    debug_with_guard(get_typename() + 
    " allocator_sorted_list &allocator_sorted_list::operator=(allocator_sorted_list &&) noexcept");

    if (this != &other)
    {
        deallocate_with_guard(_trusted_memory);

        _trusted_memory = other._trusted_memory;

        other._trusted_memory = nullptr;
    }

    return *this;
}

allocator_sorted_list::allocator_sorted_list(
    size_t space_size,
    allocator *parent_allocator,
    logger *logger,
    allocator_with_fit_mode::fit_mode allocate_fit_mode)
{
    
    if (space_size < available_block_metadata_size())
    {
        error_with_guard(get_typename() + " Can not allocate memory size < 0");

        throw std::logic_error("Can not allocate memory size < 0");
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
        error_with_guard(get_typename() + " " + ex.what());

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

    *reinterpret_cast<void **>(*reinterpret_cast<void **>(placement)) = nullptr;
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

    std::lock_guard<std::mutex> lock(get_mutex());

    void *target_block = nullptr, *previous_to_target_block = nullptr;
    size_t requested_size = value_size * values_count + ancillary_block_metadata_size();
    size_t target_block_size;

    {
        void *current_block, *previous_block = nullptr;
        current_block = get_first_available_block_address();
        allocator_with_fit_mode::fit_mode fit_mode = get_fit_mode();

        while (current_block != nullptr)
        {
            size_t current_block_size = get_available_block_size(current_block);

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
            current_block = get_next_available_block_address(current_block);
        }
    }

    if (target_block == nullptr)
    {
        // TODO: логи типо такие: напишите нас плиз =)

        throw std::bad_alloc();
    }

    // TODO: You can do it! :)

    return reinterpret_cast<void *>(reinterpret_cast<unsigned char *>(target_block) + ancillary_block_metadata_size());
}

void allocator_sorted_list::deallocate(
    void *at)
{
    debug_with_guard(get_typename() + 
    " void allocator_sorted_list::deallocate(void *)");

    std::lock_guard<std::mutex> lock(get_mutex());

    // TODO: You also can do it :)
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
    return *(reinterpret_cast<logger **>(&get_mutex()) - 1);
}

inline std::string allocator_sorted_list::get_typename() const noexcept
{
    return "[allocator_sorted_list]";
}

size_t constexpr allocator_sorted_list::summ_size()
{
    return sizeof(allocator *) + sizeof(std::mutex) + sizeof(logger *) 
        + sizeof( allocator_with_fit_mode::fit_mode) + sizeof(void *);
}

size_t constexpr allocator_sorted_list::available_block_metadata_size()
{
    return sizeof(size_t) + sizeof(void *);
}

size_t constexpr allocator_sorted_list::ancillary_block_metadata_size()
{
    return sizeof(size_t) + sizeof(void *);
}

std::mutex& allocator_sorted_list::get_mutex() const
{
    return *reinterpret_cast<std::mutex*>
        (reinterpret_cast<unsigned char*>
        (_trusted_memory) + sizeof(allocator *) + sizeof(logger *));
}

void *& allocator_sorted_list::get_first_available_block_address() const
{
    return *reinterpret_cast<void **>(
        reinterpret_cast<unsigned char *>(
        (_trusted_memory) + sizeof(allocator *) + sizeof(logger *) + 
        sizeof(std::mutex) + sizeof(allocator_with_fit_mode::fit_mode) + sizeof(size_t)));
}
void *&allocator_sorted_list::get_next_available_block_address(
    void *current_avaliable_block_adress)
{
    return *reinterpret_cast<void **>(current_avaliable_block_adress);
}

size_t &allocator_sorted_list::get_available_block_size(
    void *current_available_block_address)
{
    return *reinterpret_cast<size_t *>(
        &get_next_available_block_address(current_available_block_address) + 1);
}

allocator_with_fit_mode::fit_mode &allocator_sorted_list::get_fit_mode() const
{
    return *reinterpret_cast<allocator_with_fit_mode::fit_mode *>(
        reinterpret_cast<unsigned char *>(_trusted_memory) 
        + sizeof(allocator *) + sizeof(logger *) + sizeof(std::mutex));
}