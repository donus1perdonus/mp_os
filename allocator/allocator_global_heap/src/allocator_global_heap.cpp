#include "../include/allocator_global_heap.h"

allocator_global_heap::allocator_global_heap(
    logger *logger) 
    : _logger(logger)
{
    debug_with_guard(get_typename() + 
        " Created the object...");
}

allocator_global_heap::~allocator_global_heap()
{
    debug_with_guard(get_typename() + 
        " Destruct the object...");

    delete _logger;
}

allocator_global_heap::allocator_global_heap(
    allocator_global_heap &&other) noexcept
    : _logger(std::move(other._logger))
{
    debug_with_guard(get_typename() + 
        " Call of overloaded move constructor...");

    other._logger = nullptr;
}

allocator_global_heap &allocator_global_heap::operator=(
    allocator_global_heap &&other) noexcept
{
    debug_with_guard(get_typename() + 
        " Call of overloaded move operator=...");

    if (this != &other)
    {
        _logger = std::move(other._logger);
        other._logger = nullptr;
    }

    return *this;
}

[[nodiscard]] void *allocator_global_heap::allocate(
    size_t value_size,
    size_t values_count)
{
    debug_with_guard(get_typename() + 
        " Call of the allocate...");

    void* memory = nullptr;

    try 
    {
        memory = ::operator new(value_size * values_count);
        
        debug_with_guard(get_typename() + 
            " Memory allocated succesfully");
    } catch (const std::bad_alloc& e) {
        
        error_with_guard(get_typename() + 
            " Memory allocate error");
        
        throw;
    }

    information_with_guard(get_typename()
        + " Size of allocated memory is "
        + std::to_string(value_size * values_count)
        + " bytes");

    return memory;
}

void allocator_global_heap::deallocate(
    void *at)
{
    debug_with_guard(get_typename() + 
        " Call of the deallocate");

    if (at == nullptr) 
    {
        error_with_guard(get_typename() + 
            " Attempt to deallocate null pointer");

        throw std::logic_error("Attempt to deallocate null pointer");
    }
    
    debug_with_guard(get_typename() + 
        " Deallocate succesfully");

    ::operator delete(at);
}

inline logger *allocator_global_heap::get_logger() const
{
    return _logger;
}

inline std::string allocator_global_heap::get_typename() const noexcept
{
    return "[allocator_global_heap]";
}