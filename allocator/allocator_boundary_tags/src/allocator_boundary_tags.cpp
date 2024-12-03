#include <not_implemented.h>

#include "../include/allocator_boundary_tags.h"

#include <mutex>

allocator_boundary_tags::~allocator_boundary_tags()
{
    if(_trusted_memory == nullptr)
    {
        return;
    }

    this->
        debug_with_guard(get_typename()
        + " Destruct the object...");

    destruct(reinterpret_cast<std::mutex *>(reinterpret_cast<byte *>(_trusted_memory) + get_mutex_shift()));

    if(get_allocator() == nullptr)
    {
        ::operator delete(_trusted_memory);
    }
    else 
    {
        get_allocator()->deallocate(_trusted_memory);
    }

    _trusted_memory = nullptr;
}

allocator_boundary_tags::allocator_boundary_tags(
    allocator_boundary_tags &&other) noexcept :
    _trusted_memory(nullptr)
{
    if(other._trusted_memory == nullptr)
    {
        return;
    }

    std::lock_guard<std::mutex> locker(*reinterpret_cast<std::mutex *>(reinterpret_cast<byte *>(other._trusted_memory) + get_mutex_shift()));

    _trusted_memory = other._trusted_memory;
    other._trusted_memory = nullptr;
}

allocator_boundary_tags &allocator_boundary_tags::operator=(
    allocator_boundary_tags &&other) noexcept
{
    if(this != &other) 
    {
        std::lock_guard<std::mutex> locker(*reinterpret_cast<std::mutex *>(reinterpret_cast<byte *>(other._trusted_memory) + get_mutex_shift()));

        this->~allocator_boundary_tags();

        _trusted_memory = other._trusted_memory;

        other._trusted_memory = nullptr;
    }

    return *this;
}

allocator_boundary_tags::allocator_boundary_tags(
    size_t space_size,
    allocator *parent_allocator,
    logger *logger,
    allocator_with_fit_mode::fit_mode allocate_fit_mode) :
    _trusted_memory(nullptr)
{
    size_t full_size = obtain_global_metadata_size() + space_size + (2 * obtain_block_metadata_size());
    try 
    {
        _trusted_memory = 
            parent_allocator == nullptr 
            ? ::operator new(full_size) 
            : parent_allocator->allocate(1, full_size);
    }
    catch (std::bad_alloc const &e) 
    {
        throw e;
    }
    byte *ptr = reinterpret_cast<byte *>(_trusted_memory);
    *reinterpret_cast<size_t *>(ptr + get_trusted_memory_shift()) = full_size;
    *reinterpret_cast<class logger **>(ptr + get_logger_shift()) = logger;
    *reinterpret_cast<allocator **>(ptr + get_allocator_shift()) = parent_allocator;
    *reinterpret_cast<allocator_with_fit_mode::fit_mode *>(ptr + get_fitmode_shift()) = allocate_fit_mode;
    construct(reinterpret_cast<std::mutex *>(ptr + get_mutex_shift()));
    ptr += get_first_block_shift();
    initialize_block_metadata(ptr, full_size - obtain_global_metadata_size(), false);
    ptr += space_size + obtain_block_metadata_size();
    initialize_block_metadata(ptr, full_size - obtain_global_metadata_size(), false);

    this->
        debug_with_guard(get_typename()
        + " The object of allocator was created with "
        + std::to_string(full_size)
        + " bytes");
}

[[nodiscard]] void *allocator_boundary_tags::allocate(
    size_t value_size,
    size_t values_count)
{
    this->
        debug_with_guard(get_typename()
        + " Attempt to allocate block with size: "
        + std::to_string(value_size * values_count)
        + " bytes");

    if(_trusted_memory == nullptr) 
    {
        error_with_guard(" The trusted memory is nullptr!");
        throw std::bad_alloc();
    }
    this->
        trace_with_guard(get_typename()
        + " Lock the mutex...");
    std::lock_guard<std::mutex> locker(*reinterpret_cast<std::mutex *>(reinterpret_cast<byte *>(_trusted_memory) + get_mutex_shift()));

    auto size_of_new_block = (value_size * values_count) + (2 * obtain_block_metadata_size());
    auto fit_mode = reinterpret_cast<allocator_with_fit_mode::fit_mode *>(reinterpret_cast<byte *>(_trusted_memory) + get_fitmode_shift());
    void *res = nullptr;

    auto actual_blocks_state = this->get_blocks_info();
    std::string blocks_info(" ");
    for (const auto& value : actual_blocks_state) 
    {
        blocks_info += (value.is_block_occupied ? "Occupied " : "Free ") + std::to_string(value.block_size) + " bytes -> ";
    }
    this->
        information_with_guard(get_typename()
        + blocks_info);

    try 
    {
        switch(static_cast<int>(*fit_mode)) 
        {
            case 0:
                this->
                    information_with_guard(get_typename()
                    + " Allocate with first fit...");
                res = allocate_first_fit(size_of_new_block);
                break;
            case 1:
                this->
                    information_with_guard(get_typename()
                    + " Allocate with best fit...");
                res = allocate_best_fit(size_of_new_block);
                break;
            case 2:
                this->
                    information_with_guard(get_typename()
                    + " Allocate with worst fit...");
                res = allocate_worst_fit(size_of_new_block);
                break;
            default:
                this->
                    error_with_guard(get_typename()
                    + " Invalid fit type!");
                throw std::logic_error("Invalid fit type!");
        }
    }
    catch (std::bad_alloc const &e) 
    {
        this->
            error_with_guard(get_typename() 
            + e.what());
        throw e;
    }

    actual_blocks_state = this->get_blocks_info();
    blocks_info = " ";
    for (const auto& value : actual_blocks_state) 
    {
        blocks_info += (value.is_block_occupied ? "Occupied " : "Free ") + std::to_string(value.block_size) + " bytes -> ";
    }
    this->
        information_with_guard(get_typename()
        + blocks_info);

    this->
        debug_with_guard(get_typename()
        + " Success allocate the block with size "
        + std::to_string(size_of_new_block)
        + " bytes");

    return res;
}

void allocator_boundary_tags::deallocate(
    void *at)
{
    if(_trusted_memory == nullptr) 
    {
        throw std::logic_error("Allocator is empty");
    }

    this->
        debug_with_guard(get_typename()
        + " Call of deallocate...");

    if(at == nullptr) 
    {
        return;
    }
    if(at < reinterpret_cast<byte *>(_trusted_memory) + obtain_global_metadata_size() ||
        at > reinterpret_cast<byte *>(_trusted_memory) + *reinterpret_cast<size_t *>(reinterpret_cast<byte *>(_trusted_memory) + get_trusted_memory_shift()))
    {
        this->
            error_with_guard(get_typename()
            + " Pointer is not contained in trusted memory");
        throw std::logic_error("Pointer is not contained in trusted memory");
    }

    at = reinterpret_cast<byte *>(at) - obtain_block_metadata_size();

    if(*reinterpret_cast<void **>(reinterpret_cast<byte *>(at) + get_tm_ptr_shift()) != _trusted_memory) 
    {
        this->
            error_with_guard(get_typename()
            + " Pointer is not contained in that allocator");
        throw std::logic_error("Pointer is not contained in that allocator");
    }

    this->
        trace_with_guard(get_typename()
        + " Lock the mutex...");
    std::lock_guard<std::mutex> locker(*reinterpret_cast<std::mutex *>(reinterpret_cast<byte *>(_trusted_memory) + get_mutex_shift()));

    auto size_of_at = *reinterpret_cast<size_t *>(reinterpret_cast<byte *>(at) + get_size_of_block_shift());

    if(!*reinterpret_cast<bool *>(reinterpret_cast<byte *>(at) - obtain_block_metadata_size() + get_status_of_block_shift()) && at != reinterpret_cast<byte *>(_trusted_memory) + get_first_block_shift()) 
    {
        //Перед новым стоит свободный
        //Взяли размер предыдущего
        auto size_of_prev_block = *reinterpret_cast<size_t *>(reinterpret_cast<byte *>(at) - obtain_block_metadata_size() + get_size_of_block_shift());
        //Поставили at на предыдущий
        at = reinterpret_cast<byte *>(at) - size_of_prev_block;
        //К размеру предыдщего добавили наш
        *reinterpret_cast<size_t *>(reinterpret_cast<byte *>(at) + get_size_of_block_shift()) += size_of_at;
        size_of_at = *reinterpret_cast<size_t *>(reinterpret_cast<byte *>(at) + get_size_of_block_shift());
        //Пометили нашу страую мету свободной
        *reinterpret_cast<bool *>(reinterpret_cast<byte *>(at) + size_of_at - obtain_block_metadata_size() + get_status_of_block_shift()) = false;
        //Положили в старую мету новый размер
        *reinterpret_cast<size_t *>(reinterpret_cast<byte *>(at) + size_of_at - obtain_block_metadata_size() + get_size_of_block_shift()) = size_of_at;
    } 
    else 
    {
        //Если справа нет свободного блока
        //Пометили обе меты свободными
        *reinterpret_cast<bool *>(reinterpret_cast<byte *>(at) + get_status_of_block_shift()) = false;
        *reinterpret_cast<bool *>(reinterpret_cast<byte *>(at) + size_of_at - obtain_block_metadata_size() + get_status_of_block_shift()) = false;
    }
    if(!*reinterpret_cast<bool *>(reinterpret_cast<byte *>(at) + size_of_at + get_status_of_block_shift()) && (reinterpret_cast<byte *>(at) + size_of_at) != (reinterpret_cast<byte *>(_trusted_memory) + *reinterpret_cast<size_t *>(reinterpret_cast<byte *>(_trusted_memory) + get_trusted_memory_shift()))) {
        //За новым стоит совбодный
        //Взяли размер следующего блока
        auto sizeOfNextBlock = *reinterpret_cast<size_t *>(reinterpret_cast<byte *>(at) + size_of_at + get_size_of_block_shift());
        //К нашему размеру добавили размер следующего
        size_of_at = *reinterpret_cast<size_t *>(reinterpret_cast<byte *>(at) + get_size_of_block_shift()) += sizeOfNextBlock;
        //Кладём новый размер в правую мету
        *reinterpret_cast<size_t *>(reinterpret_cast<byte *>(at) + size_of_at - obtain_block_metadata_size() + get_size_of_block_shift()) = size_of_at;
    }
    //Тут else не нужен тк в предыдущем наш блок пометился свободным

    this->
        debug_with_guard(get_typename()
        + " Deallocate the block with size "
        + std::to_string(size_of_at)
        + " bytes");
}

inline void allocator_boundary_tags::set_fit_mode(
    allocator_with_fit_mode::fit_mode mode)
{
    if(_trusted_memory == nullptr) 
    {
        return;
    }

    this->
        trace_with_guard(get_typename()
        + " Set fit mode...");

    std::lock_guard<std::mutex> locker(*reinterpret_cast<std::mutex *>(reinterpret_cast<byte *>(_trusted_memory) + get_mutex_shift()));
    
    *reinterpret_cast<allocator_with_fit_mode::fit_mode *>(reinterpret_cast<byte *>(_trusted_memory) + get_fitmode_shift()) = mode;
}

inline allocator *allocator_boundary_tags::get_allocator() const
{
    if(_trusted_memory == nullptr)
    {
        return nullptr;
    }

    return *reinterpret_cast<allocator **>(
        reinterpret_cast<byte *>(_trusted_memory) + get_allocator_shift());
}

std::vector<allocator_test_utils::block_info> allocator_boundary_tags::get_blocks_info() const noexcept
{
    std::vector<allocator_test_utils::block_info> result;
    if(_trusted_memory == nullptr)
    { 
        return result;
    }

    byte *p_now = reinterpret_cast<byte *>(_trusted_memory) + get_first_block_shift(),
        *p_last = reinterpret_cast<byte *>(_trusted_memory) + *reinterpret_cast<size_t *>(reinterpret_cast<byte *>(_trusted_memory) + get_trusted_memory_shift());

    while(reinterpret_cast<void *>(p_now) < reinterpret_cast<void *>(p_last)) 
    {
        result.push_back(allocator_test_utils::block_info
            {
            *reinterpret_cast<size_t *>(p_now + get_size_of_block_shift()), 
            *reinterpret_cast<bool *>(p_now + get_status_of_block_shift())
            });

        p_now += *reinterpret_cast<size_t *>(p_now + get_size_of_block_shift());
    }

    return result;
}

inline logger *allocator_boundary_tags::get_logger() const
{
    if(_trusted_memory == nullptr)
    {
        return nullptr;
    }

    return *reinterpret_cast<logger **>(
        reinterpret_cast<byte *>(_trusted_memory) + get_logger_shift());
}

inline std::string allocator_boundary_tags::get_typename() const noexcept
{
    return ("[allocator_boundary_tags]");
}

//--------------------------------------------------------------------------

constexpr size_t allocator_boundary_tags::obtain_global_metadata_size() 
{
    return sizeof(size_t) + sizeof(logger *) + sizeof(allocator *) + sizeof(allocator_with_fit_mode::fit_mode) + sizeof(std::mutex);
}

constexpr size_t allocator_boundary_tags::obtain_block_metadata_size() 
{
    return sizeof(bool) + sizeof(size_t) + sizeof(void *);
}

constexpr size_t allocator_boundary_tags::get_trusted_memory_shift() 
{
    return 0;
}

constexpr size_t allocator_boundary_tags::get_logger_shift() 
{
    return get_trusted_memory_shift() + sizeof(size_t);
}

constexpr size_t allocator_boundary_tags::get_allocator_shift() 
{
    return get_logger_shift() + sizeof(logger *);
}

constexpr size_t allocator_boundary_tags::get_fitmode_shift() 
{
    return get_allocator_shift() + sizeof(allocator *);
}

constexpr size_t allocator_boundary_tags::get_mutex_shift() 
{
    return get_fitmode_shift() + sizeof(allocator_with_fit_mode::fit_mode);
}

constexpr size_t allocator_boundary_tags::get_first_block_shift() 
{
    return get_mutex_shift() + sizeof(std::mutex);
}

constexpr size_t allocator_boundary_tags::get_tm_ptr_shift() 
{
    return 0;
}

constexpr size_t allocator_boundary_tags::get_status_of_block_shift() 
{
    return sizeof(void *);
}

constexpr size_t allocator_boundary_tags::get_size_of_block_shift() 
{
    return get_status_of_block_shift() + sizeof(bool);
}

void allocator_boundary_tags::initialize_block_metadata(byte *ptr, size_t size_of_block, bool statusOfBlock) 
{
    *reinterpret_cast<void **>(ptr + get_tm_ptr_shift()) = _trusted_memory;
    *reinterpret_cast<bool *>(ptr + get_status_of_block_shift()) = statusOfBlock;
    *reinterpret_cast<size_t *>(ptr + get_size_of_block_shift()) = size_of_block;
}

void *allocator_boundary_tags::allocate_first_fit(size_t size_of_new_block) 
{
    byte *curent_block = reinterpret_cast<byte *>(_trusted_memory) + get_first_block_shift(),
         *last_ptr = reinterpret_cast<byte *>(_trusted_memory) + *reinterpret_cast<size_t *>(reinterpret_cast<byte *>(_trusted_memory) + get_trusted_memory_shift());
    
    while(curent_block < last_ptr) 
    {
        auto size_of_block = *reinterpret_cast<size_t *>(curent_block + get_size_of_block_shift());

        if(!*reinterpret_cast<bool *>(curent_block + get_status_of_block_shift())) 
        {
            if(size_of_block == size_of_new_block) return allocate_full_block(curent_block);
            if(size_of_block > size_of_new_block) return allocate_block(curent_block, size_of_new_block);
        }

        curent_block += size_of_block;
    }

    this->
        error_with_guard(get_typename()
        + " Can not find the available block");
    throw std::bad_alloc();
}

void *allocator_boundary_tags::allocate_best_fit(size_t size_of_block) 
{
    byte *best_block = find_first_free_block(size_of_block),
    *current_block = reinterpret_cast<byte *>(_trusted_memory) + get_first_block_shift(),
    *p_last = reinterpret_cast<byte *>(_trusted_memory) + *reinterpret_cast<size_t *>(reinterpret_cast<byte *>(_trusted_memory) + get_trusted_memory_shift());
    if(best_block == nullptr) 
    {
        this->
            error_with_guard(get_typename()
            + " Can not find the available block");
        throw std::bad_alloc();
    }
    if(*reinterpret_cast<size_t *>(best_block + get_size_of_block_shift()) == size_of_block) return allocate_full_block(best_block);

    while(current_block < p_last) 
    {
        if(!*reinterpret_cast<bool *>(current_block + get_status_of_block_shift())) 
        {
            if(*reinterpret_cast<size_t *>(best_block + get_size_of_block_shift()) > *reinterpret_cast<size_t *>(current_block + get_size_of_block_shift())) 
            {
                best_block = current_block;
            }
        }
        current_block += *reinterpret_cast<size_t *>(current_block + get_size_of_block_shift());
    }
    return allocate_block(best_block, size_of_block);
}

void *allocator_boundary_tags::allocate_worst_fit(size_t size_of_block) 
{
    byte *best_block = find_first_free_block(size_of_block),
    *current_block = reinterpret_cast<byte *>(_trusted_memory) + get_first_block_shift(),
    *p_last = reinterpret_cast<byte *>(_trusted_memory) + *reinterpret_cast<size_t *>(reinterpret_cast<byte *>(_trusted_memory) + get_trusted_memory_shift());
    if(best_block == nullptr) 
    {
        this->
            error_with_guard(get_typename()
            + " Can not find the available block");
        throw std::bad_alloc();
    }
    if(*reinterpret_cast<size_t *>(best_block + get_size_of_block_shift()) == size_of_block) 
    {
        return allocate_full_block(best_block);
    }
    while(current_block < p_last) 
    {
        if(!*reinterpret_cast<bool *>(current_block + get_status_of_block_shift())) 
        {
            if(*reinterpret_cast<size_t *>(best_block + get_size_of_block_shift()) < *reinterpret_cast<size_t *>(current_block + get_size_of_block_shift()))
            {
                best_block = current_block;
            }
        }
        current_block += *reinterpret_cast<size_t *>(current_block + get_size_of_block_shift());
    }
    return allocate_block(best_block, size_of_block);
}

void *allocator_boundary_tags::allocate_block(void *block, size_t size_of_new_block) 
{
    byte *p_block = reinterpret_cast<byte *>(block);
    auto minsize_of_block = 2 * obtain_block_metadata_size(),
    size_of_block = *reinterpret_cast<size_t *>(p_block + get_size_of_block_shift());
    if(size_of_block - size_of_new_block < minsize_of_block)
    {
        return allocate_full_block(block);
    }

    *reinterpret_cast<void **>(p_block + get_tm_ptr_shift()) = _trusted_memory;
    *reinterpret_cast<bool *>(p_block + get_status_of_block_shift()) = true;
    *reinterpret_cast<size_t *>(p_block + get_size_of_block_shift()) = size_of_new_block;
    auto resultPtr = p_block + obtain_block_metadata_size();

    p_block += size_of_new_block;

    *reinterpret_cast<void **>((p_block - obtain_block_metadata_size()) + get_tm_ptr_shift()) = _trusted_memory;
    *reinterpret_cast<bool *>((p_block - obtain_block_metadata_size()) + get_status_of_block_shift()) = true;
    *reinterpret_cast<size_t *>((p_block - obtain_block_metadata_size()) + get_size_of_block_shift()) = size_of_new_block;

    *reinterpret_cast<void **>(p_block + get_tm_ptr_shift()) = _trusted_memory;
    *reinterpret_cast<bool *>(p_block + get_status_of_block_shift()) = false;
    *reinterpret_cast<size_t *>(p_block + get_size_of_block_shift()) = size_of_block - size_of_new_block;

    p_block += size_of_block - size_of_new_block - obtain_block_metadata_size();

    *reinterpret_cast<void **>(p_block + get_tm_ptr_shift()) = _trusted_memory;
    *reinterpret_cast<bool *>(p_block + get_status_of_block_shift()) = false;
    *reinterpret_cast<size_t *>(p_block + get_size_of_block_shift()) = size_of_block - size_of_new_block;
    return resultPtr;
}

void *allocator_boundary_tags::allocate_full_block(void *block) 
{
    byte *p_block = reinterpret_cast<byte *>(block);
    *reinterpret_cast<bool *>(p_block + get_status_of_block_shift()) = true;
    auto secondMeta = p_block + *reinterpret_cast<size_t *>(p_block + get_size_of_block_shift()) - obtain_block_metadata_size();

    *reinterpret_cast<bool *>(secondMeta + get_status_of_block_shift()) = true;
    *reinterpret_cast<void **>(p_block + get_tm_ptr_shift()) = _trusted_memory;
    *reinterpret_cast<void **>(secondMeta + get_tm_ptr_shift()) = _trusted_memory;

    return p_block + obtain_block_metadata_size();
}

byte *allocator_boundary_tags::find_first_free_block(size_t size_of_block) const 
{
    byte *current_block = reinterpret_cast<byte *>(_trusted_memory) + get_first_block_shift(),
        *p_last = reinterpret_cast<byte *>(_trusted_memory) + *reinterpret_cast<size_t *>    (reinterpret_cast<byte *>(_trusted_memory) + get_trusted_memory_shift());

    while(current_block < p_last) 
    {
        if(!*reinterpret_cast<bool *>(current_block + get_status_of_block_shift())) 
        {
            if(*reinterpret_cast<size_t *>(current_block + get_size_of_block_shift()) >= size_of_block) return current_block;
        }

        current_block += *reinterpret_cast<size_t *>(current_block + get_size_of_block_shift());
    }

    return nullptr;
}