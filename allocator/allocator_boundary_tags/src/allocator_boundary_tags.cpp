#include <not_implemented.h>

#include "../include/allocator_boundary_tags.h"

#include <mutex>

allocator_boundary_tags::~allocator_boundary_tags()
{
    if(_trusted_memory == nullptr)
    {
        return;
    }
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
    size_t fullSize = obtain_global_metadata_size() + space_size + (2 * obtain_block_metadata_size());
    try 
    {
        _trusted_memory = 
            parent_allocator == nullptr 
            ? ::operator new(fullSize) 
            : parent_allocator->allocate(1, fullSize);
    }
    catch (std::bad_alloc const &e) 
    {
        throw e;
    }
    byte *ptr = reinterpret_cast<byte *>(_trusted_memory);
    *reinterpret_cast<size_t *>(ptr + get_trusted_memory_shift()) = fullSize;
    *reinterpret_cast<class logger **>(ptr + get_logger_shift()) = logger;
    *reinterpret_cast<allocator **>(ptr + get_allocator_shift()) = parent_allocator;
    *reinterpret_cast<allocator_with_fit_mode::fit_mode *>(ptr + get_fitmode_shift()) = allocate_fit_mode;
    construct(reinterpret_cast<std::mutex *>(ptr + get_mutex_shift()));
    ptr += get_first_block_shift();
    initialize_block_metadata(ptr, fullSize - obtain_global_metadata_size(), false);
    ptr += space_size + obtain_block_metadata_size();
    initialize_block_metadata(ptr, fullSize - obtain_global_metadata_size(), false);

    this->
        debug_with_guard(get_typename()
        + " The object of allocator was created with "
        + std::to_string(fullSize)
        + " bytes");
}

[[nodiscard]] void *allocator_boundary_tags::allocate(
    size_t value_size,
    size_t values_count)
{
    this->
        debug_with_guard(get_typename()
        + " Call of allocate...");

    if(_trusted_memory == nullptr) 
    {
        error_with_guard(" The trusted memory is nullptr!");
        throw std::bad_alloc();
    }
    this->
        trace_with_guard(get_typename()
        + " Lock the mutex...");
    std::lock_guard<std::mutex> locker(*reinterpret_cast<std::mutex *>(reinterpret_cast<byte *>(_trusted_memory) + get_mutex_shift()));

    auto sizeOfNewBlock = (value_size * values_count) + (2 * obtain_block_metadata_size());
    auto fitMode = reinterpret_cast<allocator_with_fit_mode::fit_mode *>(reinterpret_cast<byte *>(_trusted_memory) + get_fitmode_shift());
    void *res = nullptr;
    try 
    {
        switch(static_cast<int>(*fitMode)) 
        {
            case 0:
                this->
                    trace_with_guard(get_typename()
                    + " Allocate with first fit...");
                res = allocate_first_fit(sizeOfNewBlock);
                break;
            case 1:
                this->
                    trace_with_guard(get_typename()
                    + " Allocate with best fit...");
                res = allocate_best_fit(sizeOfNewBlock);
                break;
            case 2:
                this->
                    trace_with_guard(get_typename()
                    + " Allocate with worst fit...");
                res = allocate_worst_fit(sizeOfNewBlock);
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

    this->
        debug_with_guard(get_typename()
        + " Allocate the block with size "
        + std::to_string(sizeOfNewBlock)
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

    auto sizeOfAt = *reinterpret_cast<size_t *>(reinterpret_cast<byte *>(at) + get_size_of_block_shift());

    if(!*reinterpret_cast<bool *>(reinterpret_cast<byte *>(at) - obtain_block_metadata_size() + get_status_of_block_shift()) && at != reinterpret_cast<byte *>(_trusted_memory) + get_first_block_shift()) 
    {
        //Перед новым стоит свободный
        //Взяли размер предыдущего
        auto sizeOfPrevBlock = *reinterpret_cast<size_t *>(reinterpret_cast<byte *>(at) - obtain_block_metadata_size() + get_size_of_block_shift());
        //Поставили at на предыдущий
        at = reinterpret_cast<byte *>(at) - sizeOfPrevBlock;
        //К размеру предыдщего добавили наш
        *reinterpret_cast<size_t *>(reinterpret_cast<byte *>(at) + get_size_of_block_shift()) += sizeOfAt;
        sizeOfAt = *reinterpret_cast<size_t *>(reinterpret_cast<byte *>(at) + get_size_of_block_shift());
        //Пометили нашу страую мету свободной
        *reinterpret_cast<bool *>(reinterpret_cast<byte *>(at) + sizeOfAt - obtain_block_metadata_size() + get_status_of_block_shift()) = false;
        //Положили в старую мету новый размер
        *reinterpret_cast<size_t *>(reinterpret_cast<byte *>(at) + sizeOfAt - obtain_block_metadata_size() + get_size_of_block_shift()) = sizeOfAt;
    } 
    else 
    {
        //Если справа нет свободного блока
        //Пометили обе меты свободными
        *reinterpret_cast<bool *>(reinterpret_cast<byte *>(at) + get_status_of_block_shift()) = false;
        *reinterpret_cast<bool *>(reinterpret_cast<byte *>(at) + sizeOfAt - obtain_block_metadata_size() + get_status_of_block_shift()) = false;
    }
    if(!*reinterpret_cast<bool *>(reinterpret_cast<byte *>(at) + sizeOfAt + get_status_of_block_shift()) && (reinterpret_cast<byte *>(at) + sizeOfAt) != (reinterpret_cast<byte *>(_trusted_memory) + *reinterpret_cast<size_t *>(reinterpret_cast<byte *>(_trusted_memory) + get_trusted_memory_shift()))) {
        //За новым стоит совбодный
        //Взяли размер следующего блока
        auto sizeOfNextBlock = *reinterpret_cast<size_t *>(reinterpret_cast<byte *>(at) + sizeOfAt + get_size_of_block_shift());
        //К нашему размеру добавили размер следующего
        sizeOfAt = *reinterpret_cast<size_t *>(reinterpret_cast<byte *>(at) + get_size_of_block_shift()) += sizeOfNextBlock;
        //Кладём новый размер в правую мету
        *reinterpret_cast<size_t *>(reinterpret_cast<byte *>(at) + sizeOfAt - obtain_block_metadata_size() + get_size_of_block_shift()) = sizeOfAt;
    }
    //Тут else не нужен тк в предыдущем наш блок пометился свободным

    this->
        debug_with_guard(get_typename()
        + " Deallocate the block with size "
        + std::to_string(sizeOfAt)
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

    byte *pNow = reinterpret_cast<byte *>(_trusted_memory) + get_first_block_shift(),
        *pLast = reinterpret_cast<byte *>(_trusted_memory) + *reinterpret_cast<size_t *>(reinterpret_cast<byte *>(_trusted_memory) + get_trusted_memory_shift());

    while(reinterpret_cast<void *>(pNow) < reinterpret_cast<void *>(pLast)) 
    {
        result.push_back(allocator_test_utils::block_info
            {
            *reinterpret_cast<size_t *>(pNow + get_size_of_block_shift()), 
            *reinterpret_cast<bool *>(pNow + get_status_of_block_shift())
            });

        pNow += *reinterpret_cast<size_t *>(pNow + get_size_of_block_shift());
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

void allocator_boundary_tags::initialize_block_metadata(byte *ptr, size_t sizeOfBlock, bool statusOfBlock) 
{
    *reinterpret_cast<void **>(ptr + get_tm_ptr_shift()) = _trusted_memory;
    *reinterpret_cast<bool *>(ptr + get_status_of_block_shift()) = statusOfBlock;
    *reinterpret_cast<size_t *>(ptr + get_size_of_block_shift()) = sizeOfBlock;
}

void *allocator_boundary_tags::allocate_first_fit(size_t sizeOfNewBlock) 
{
    byte *curentBlock = reinterpret_cast<byte *>(_trusted_memory) + get_first_block_shift(),
         *lastPtr = reinterpret_cast<byte *>(_trusted_memory) + *reinterpret_cast<size_t *>(reinterpret_cast<byte *>(_trusted_memory) + get_trusted_memory_shift());
    
    while(curentBlock < lastPtr) 
    {
        auto sizeOfBlock = *reinterpret_cast<size_t *>(curentBlock + get_size_of_block_shift());

        if(!*reinterpret_cast<bool *>(curentBlock + get_status_of_block_shift())) 
        {
            if(sizeOfBlock == sizeOfNewBlock) return allocate_full_block(curentBlock);
            if(sizeOfBlock > sizeOfNewBlock) return allocate_block(curentBlock, sizeOfNewBlock);
        }

        curentBlock += sizeOfBlock;
    }

    this->
        error_with_guard(get_typename()
        + " Can not find the available block");
    throw std::bad_alloc();
}

void *allocator_boundary_tags::allocate_best_fit(size_t sizeOfBlock) 
{
    byte *bestBlock = find_first_free_block(sizeOfBlock),
    *currentBlock = reinterpret_cast<byte *>(_trusted_memory) + get_first_block_shift(),
    *pLast = reinterpret_cast<byte *>(_trusted_memory) + *reinterpret_cast<size_t *>(reinterpret_cast<byte *>(_trusted_memory) + get_trusted_memory_shift());
    if(bestBlock == nullptr) 
    {
        this->
            error_with_guard(get_typename()
            + " Can not find the available block");
        throw std::bad_alloc();
    }
    if(*reinterpret_cast<size_t *>(bestBlock + get_size_of_block_shift()) == sizeOfBlock) return allocate_full_block(bestBlock);

    while(currentBlock < pLast) 
    {
        if(!*reinterpret_cast<bool *>(currentBlock + get_status_of_block_shift())) 
        {
            if(*reinterpret_cast<size_t *>(bestBlock + get_size_of_block_shift()) > *reinterpret_cast<size_t *>(currentBlock + get_size_of_block_shift())) 
            {
                bestBlock = currentBlock;
            }
        }
        currentBlock += *reinterpret_cast<size_t *>(currentBlock + get_size_of_block_shift());
    }
    return allocate_block(bestBlock, sizeOfBlock);
}

void *allocator_boundary_tags::allocate_worst_fit(size_t sizeOfBlock) 
{
    byte *bestBlock = find_first_free_block(sizeOfBlock),
    *currentBlock = reinterpret_cast<byte *>(_trusted_memory) + get_first_block_shift(),
    *pLast = reinterpret_cast<byte *>(_trusted_memory) + *reinterpret_cast<size_t *>(reinterpret_cast<byte *>(_trusted_memory) + get_trusted_memory_shift());
    if(bestBlock == nullptr) 
    {
        this->
            error_with_guard(get_typename()
            + " Can not find the available block");
        throw std::bad_alloc();
    }
    if(*reinterpret_cast<size_t *>(bestBlock + get_size_of_block_shift()) == sizeOfBlock) 
    {
        return allocate_full_block(bestBlock);
    }
    while(currentBlock < pLast) 
    {
        if(!*reinterpret_cast<bool *>(currentBlock + get_status_of_block_shift())) 
        {
            if(*reinterpret_cast<size_t *>(bestBlock + get_size_of_block_shift()) < *reinterpret_cast<size_t *>(currentBlock + get_size_of_block_shift()))
            {
                bestBlock = currentBlock;
            }
        }
        currentBlock += *reinterpret_cast<size_t *>(currentBlock + get_size_of_block_shift());
    }
    return allocate_block(bestBlock, sizeOfBlock);
}

void *allocator_boundary_tags::allocate_block(void *block, size_t sizeOfNewBlock) 
{
    byte *pBlock = reinterpret_cast<byte *>(block);
    auto minSizeOfBlock = 2 * obtain_block_metadata_size(),
    sizeOfBlock = *reinterpret_cast<size_t *>(pBlock + get_size_of_block_shift());
    if(sizeOfBlock - sizeOfNewBlock < minSizeOfBlock)
    {
        return allocate_full_block(block);
    }

    *reinterpret_cast<void **>(pBlock + get_tm_ptr_shift()) = _trusted_memory;
    *reinterpret_cast<bool *>(pBlock + get_status_of_block_shift()) = true;
    *reinterpret_cast<size_t *>(pBlock + get_size_of_block_shift()) = sizeOfNewBlock;
    auto resultPtr = pBlock + obtain_block_metadata_size();

    pBlock += sizeOfNewBlock;

    *reinterpret_cast<void **>((pBlock - obtain_block_metadata_size()) + get_tm_ptr_shift()) = _trusted_memory;
    *reinterpret_cast<bool *>((pBlock - obtain_block_metadata_size()) + get_status_of_block_shift()) = true;
    *reinterpret_cast<size_t *>((pBlock - obtain_block_metadata_size()) + get_size_of_block_shift()) = sizeOfNewBlock;

    *reinterpret_cast<void **>(pBlock + get_tm_ptr_shift()) = _trusted_memory;
    *reinterpret_cast<bool *>(pBlock + get_status_of_block_shift()) = false;
    *reinterpret_cast<size_t *>(pBlock + get_size_of_block_shift()) = sizeOfBlock - sizeOfNewBlock;

    pBlock += sizeOfBlock - sizeOfNewBlock - obtain_block_metadata_size();

    *reinterpret_cast<void **>(pBlock + get_tm_ptr_shift()) = _trusted_memory;
    *reinterpret_cast<bool *>(pBlock + get_status_of_block_shift()) = false;
    *reinterpret_cast<size_t *>(pBlock + get_size_of_block_shift()) = sizeOfBlock - sizeOfNewBlock;
    return resultPtr;
}

void *allocator_boundary_tags::allocate_full_block(void *block) 
{
    byte *pBlock = reinterpret_cast<byte *>(block);
    *reinterpret_cast<bool *>(pBlock + get_status_of_block_shift()) = true;
    auto secondMeta = pBlock + *reinterpret_cast<size_t *>(pBlock + get_size_of_block_shift()) - obtain_block_metadata_size();

    *reinterpret_cast<bool *>(secondMeta + get_status_of_block_shift()) = true;
    *reinterpret_cast<void **>(pBlock + get_tm_ptr_shift()) = _trusted_memory;
    *reinterpret_cast<void **>(secondMeta + get_tm_ptr_shift()) = _trusted_memory;

    return pBlock + obtain_block_metadata_size();
}

byte *allocator_boundary_tags::find_first_free_block(size_t sizeOfBlock) const 
{
    byte *currentBlock = reinterpret_cast<byte *>(_trusted_memory) + get_first_block_shift(),
        *pLast = reinterpret_cast<byte *>(_trusted_memory) + *reinterpret_cast<size_t *>    (reinterpret_cast<byte *>(_trusted_memory) + get_trusted_memory_shift());

    while(currentBlock < pLast) 
    {
        if(!*reinterpret_cast<bool *>(currentBlock + get_status_of_block_shift())) 
        {
            if(*reinterpret_cast<size_t *>(currentBlock + get_size_of_block_shift()) >= sizeOfBlock) return currentBlock;
        }

        currentBlock += *reinterpret_cast<size_t *>(currentBlock + get_size_of_block_shift());
    }

    return nullptr;
}