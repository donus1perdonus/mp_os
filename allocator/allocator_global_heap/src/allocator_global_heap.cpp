#include "../include/allocator_global_heap.h"

allocator_global_heap::allocator_global_heap(
    logger *logger) 
    : _logger(logger)
{
    debug_with_guard(get_typename() + 
        " allocator_global_heap::allocator_global_heap(logger *logger)");
}

allocator_global_heap::~allocator_global_heap()
{
    debug_with_guard(get_typename() + 
        " allocator_global_heap::~allocator_global_heap()");

    delete _logger;
}

allocator_global_heap::allocator_global_heap(
    allocator_global_heap &&other) noexcept
    : _logger(std::move(other._logger))
{
    debug_with_guard(get_typename() + 
        " allocator_global_heap::allocator_global_heap(allocator_global_heap &&other) noexcept");

    other._logger = nullptr;
}

allocator_global_heap &allocator_global_heap::operator=(
    allocator_global_heap &&other) noexcept
{
    debug_with_guard(get_typename() + 
        " allocator_global_heap &allocator_global_heap::operator=(allocator_global_heap &&other) noexcept");

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
        " [[nodiscard]] void *allocator_global_heap::allocate()");

    void* memory = nullptr;

    try 
    {
        memory = ::operator new(value_size * values_count);
        
        debug_with_guard(get_typename() + 
            " Memory allocated succesfully");
    
    } catch (const std::bad_alloc& e) {
        
        debug_with_guard(get_typename() + 
            " Memory allocate error");
        
        throw;
    }

    return memory;
}

void allocator_global_heap::deallocate(
    void *at)
{
    debug_with_guard(get_typename() + 
        " void allocator_global_heap::deallocate()");

    if (at == nullptr) 
    {
        error_with_guard(get_typename() + 
            "Attempt to deallocate null pointer");

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