#include <not_implemented.h>

#include "../include/allocator_global_heap.h"

allocator_global_heap::allocator_global_heap(
    logger *logger) : _logger(logger)
{
}

allocator_global_heap::~allocator_global_heap()
{
    delete _logger;
}

allocator_global_heap::allocator_global_heap(
    allocator_global_heap &&other) noexcept
    : _logger(std::move(other._logger))
{
    other._logger = nullptr;
}

allocator_global_heap &allocator_global_heap::operator=(
    allocator_global_heap &&other) noexcept
{
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
    void* memory = nullptr;

    try 
    {
        memory = ::operator new(value_size * values_count);

        if (_logger) 
        {
            _logger->log(get_typename() + " Memory allocated succesfully", logger::severity::debug);
        }
    } catch (const std::bad_alloc& e) {
        if (_logger) 
        {
            _logger->log(get_typename() + " Memory allocate error", logger::severity::debug);
        }
        throw;
    }

    return memory;
}

void allocator_global_heap::deallocate(
    void *at)
{
    if (at == nullptr) 
    {
        if (_logger) 
        {
            _logger->log(get_typename() + "Attempt to deallocate null pointer", logger::severity::debug);
        }

        throw std::logic_error("Attempt to deallocate null pointer");
    }

    // Проверка на принадлежность освобождаемого блока к текущему объекту аллокатора
    // В данном случае, мы предполагаем, что все выделенные блоки принадлежат текущему объекту аллокатора

    if (_logger) 
    {
        _logger->log(get_typename() + "Deallocate succesfully", logger::severity::debug);
    }

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