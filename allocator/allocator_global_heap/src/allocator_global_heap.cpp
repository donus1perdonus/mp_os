#include "../include/allocator_global_heap.h"

allocator_global_heap::allocator_global_heap(
    logger *logger) 
    : _logger(logger)
{
    log_with_guard(get_typename() + 
        " allocator_global_heap::allocator_global_heap(logger *logger)",
        logger::severity::debug);
}

allocator_global_heap::~allocator_global_heap()
{
    log_with_guard(get_typename() + 
        " allocator_global_heap::~allocator_global_heap()",
        logger::severity::debug);

    delete _logger;
}

allocator_global_heap::allocator_global_heap(
    allocator_global_heap &&other) noexcept
    : _logger(std::move(other._logger))
{
    log_with_guard(get_typename() + 
        " allocator_global_heap::allocator_global_heap(allocator_global_heap &&other) noexcept",
        logger::severity::debug);

    other._logger = nullptr;
}

allocator_global_heap &allocator_global_heap::operator=(
    allocator_global_heap &&other) noexcept
{
    log_with_guard(get_typename() + 
        " allocator_global_heap &allocator_global_heap::operator=(allocator_global_heap &&other) noexcept",
        logger::severity::debug);

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
    log_with_guard(get_typename() + 
        " [[nodiscard]] void *allocator_global_heap::allocate()",
        logger::severity::debug);

    void* memory = nullptr;

    try 
    {
        memory = ::operator new(value_size * values_count);
        
        log_with_guard(get_typename() + 
            " Memory allocated succesfully", 
            logger::severity::debug);
    
    } catch (const std::bad_alloc& e) {
        
        log_with_guard(get_typename() + 
            " Memory allocate error",
            logger::severity::debug);
        
        throw;
    }

    return memory;
}

void allocator_global_heap::deallocate(
    void *at)
{
    log_with_guard(get_typename() + 
        " void allocator_global_heap::deallocate()",
        logger::severity::debug);

    if (at == nullptr) 
    {
        log_with_guard(get_typename() + 
            "Attempt to deallocate null pointer",
            logger::severity::debug);

        throw std::logic_error("Attempt to deallocate null pointer");
    }
    
    log_with_guard(get_typename() + 
        " Deallocate succesfully", 
        logger::severity::debug);

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