#include <not_implemented.h>

#include "../include/allocator_buddies_system.h"
#include <cmath>
#include <mutex>

allocator_buddies_system::~allocator_buddies_system()
{
    if(_trusted_memory == nullptr) 
    {
        return;
    }

    destruct(reinterpret_cast<std::mutex *>(reinterpret_cast<byte *>(_trusted_memory) + get_mutex_shift()));

    deallocate_with_guard(_trusted_memory);

    _trusted_memory = nullptr;
}

allocator_buddies_system::allocator_buddies_system(
    allocator_buddies_system &&other) noexcept :
    _trusted_memory(nullptr)
{
    if(other._trusted_memory == nullptr) 
    {
        return;
    }

    std::lock_guard<std::mutex> locker(*reinterpret_cast<std::mutex *>(reinterpret_cast<byte *>(other._trusted_memory)));

    _trusted_memory = other._trusted_memory;
    other._trusted_memory = nullptr;
}

allocator_buddies_system &allocator_buddies_system::operator=(
    allocator_buddies_system &&other) noexcept
{
    if(this != &other) 
    {
        std::lock_guard<std::mutex> locker(*reinterpret_cast<std::mutex *>(reinterpret_cast<byte *>(other._trusted_memory)));
        
        this->~allocator_buddies_system();

        _trusted_memory = other._trusted_memory;
        other._trusted_memory = nullptr;
    }

    return *this;
}

allocator_buddies_system::allocator_buddies_system(
    size_t space_size,
    allocator *parent_allocator,
    logger *logger,
    allocator_with_fit_mode::fit_mode allocate_fit_mode) :
    _trusted_memory(nullptr)
{
    if(space_size > UCHAR_MAX) 
    {
        throw std::runtime_error("Не придумал :3");
    }
    size_t size_of_trusted_memory = get_global_meta_size() + (1 << space_size);
    if(1 << space_size < get_free_block_meta_size()) 
    {
        throw std::logic_error("Space size is small");
    }
    try 
    {
        _trusted_memory = (parent_allocator == nullptr ?
        ::operator new(size_of_trusted_memory) :
        parent_allocator->allocate(1, size_of_trusted_memory));
    }
    catch(std::bad_alloc const &e) 
    {
        std::cerr << e.what() << std::endl;
        throw e;
    }

    byte *tm = reinterpret_cast<byte *>(_trusted_memory), *first_block = tm + get_global_meta_size();
    *reinterpret_cast<size_t *>(tm + get_size_of_trusted_memory_shift()) = size_of_trusted_memory;
    *reinterpret_cast<allocator **>(tm + get_allocator_shift()) = parent_allocator;
    *reinterpret_cast<class logger **>(tm + get_logger_shift()) = logger;
    *reinterpret_cast<void **>(tm + get_first_free_block_shift()) = reinterpret_cast<void *>(first_block);
    *reinterpret_cast<allocator_with_fit_mode::fit_mode *>(tm + get_fit_mode_shift()) = allocate_fit_mode;
    construct(reinterpret_cast<std::mutex *>(tm + get_mutex_shift()));

    *reinterpret_cast<void **>(first_block + get_p_prev_shift()) = nullptr;
    *reinterpret_cast<void **>(first_block + get_p_next_shift()) = nullptr;
    *(first_block + get_size_of_free_block_shift()) = static_cast<byte>(space_size);
}

[[nodiscard]] void *allocator_buddies_system::allocate(
    size_t value_size,
    size_t values_count)
{
    if(_trusted_memory == nullptr) 
    {
        throw std::bad_alloc();
    }

    if(*reinterpret_cast<void **>(reinterpret_cast<byte *>(_trusted_memory) + get_first_free_block_shift()) == nullptr) 
    {
        throw std::bad_alloc();
    }

    std::lock_guard<std::mutex> locker(*reinterpret_cast<std::mutex *>(reinterpret_cast<byte *>(_trusted_memory) + get_mutex_shift()));
    auto fit = *reinterpret_cast<allocator_with_fit_mode::fit_mode *>(reinterpret_cast<byte *>(_trusted_memory) + get_fit_mode_shift());
    byte size_of_new_blockShift = get_minimal_shift_of_block_size(get_occupied_block_meta_size() + (value_size * values_count));

    switch (fit) 
    {
        case allocator_with_fit_mode::fit_mode::first_fit:
            return allocate_first_fit(size_of_new_blockShift);
            break;
        case allocator_with_fit_mode::fit_mode::the_best_fit:
            return allocate_best_fit(size_of_new_blockShift);
            break;
        case allocator_with_fit_mode::fit_mode::the_worst_fit:
            return allocate_worst_fit(size_of_new_blockShift);
            break;
        default:
            throw std::runtime_error("Unknown fitMode");
    }
}

void allocator_buddies_system::deallocate(
    void *at)
{
    if(at == nullptr) 
    {
        return;
    }

    if(_trusted_memory == nullptr) 
    {
        throw std::logic_error("Allocator is empty");
    }

    at = reinterpret_cast<byte *>(at) - get_occupied_block_meta_size();
    void *p_first = reinterpret_cast<byte *>(_trusted_memory) + get_global_meta_size(),
        *p_last = reinterpret_cast<byte *>(_trusted_memory) + *reinterpret_cast<size_t *>(reinterpret_cast<byte *>(_trusted_memory) + get_size_of_trusted_memory_shift());
    
    if(at < p_first || at > p_last)
    {
        throw std::logic_error("Ptr is non consists to that allocator");
    }

    if(*reinterpret_cast<void **>(reinterpret_cast<byte *>(at) + get_tm_pointer_shift()) != _trusted_memory) 
    {
        throw std::logic_error("Ptr not belong to that allocator");
    }

    void *p_right = *reinterpret_cast<void **>(reinterpret_cast<byte *>(_trusted_memory) + get_first_free_block_shift());
    byte size_of_block = *(reinterpret_cast<byte *>(at) + get_size_of_occupied_block_shift());

    if(p_right == nullptr) 
    {
        //У нас нет ни одного блока :(
        *reinterpret_cast<void **>(reinterpret_cast<byte *>(_trusted_memory) + get_first_free_block_shift()) = at;
        *reinterpret_cast<void **>(reinterpret_cast<byte *>(at) + get_p_prev_shift()) = nullptr;
        *reinterpret_cast<void **>(reinterpret_cast<byte *>(at) + get_p_next_shift()) = nullptr;
        *(reinterpret_cast<byte *>(at) + get_size_of_free_block_shift()) = size_of_block;
        return;
    }
    while(true) 
    {
        if(at < p_right) 
        {
            //Нашли перед каким блоком стоит наш at
            void *pLeft = *reinterpret_cast<void **>(reinterpret_cast<byte *>(p_right) + get_p_prev_shift());
            *reinterpret_cast<void **>(reinterpret_cast<byte *>(p_right) + get_p_prev_shift()) = at;
            *reinterpret_cast<void **>(reinterpret_cast<byte *>(at) + get_p_next_shift()) = p_right;
            *reinterpret_cast<void **>(reinterpret_cast<byte *>(at) + get_p_prev_shift()) = pLeft;
            if(pLeft == nullptr)
            {
                *reinterpret_cast<void **>(reinterpret_cast<byte *>(_trusted_memory) + get_first_free_block_shift()) = at;
            }
            else
            {
                *reinterpret_cast<void **>(reinterpret_cast<byte *>(pLeft) + get_p_next_shift()) = at;
            }
            *(reinterpret_cast<byte *>(at) + get_size_of_free_block_shift()) = size_of_block;
            unite_block(at);
            return;
        }
        if(*reinterpret_cast<void **>(reinterpret_cast<byte *>(p_right) + get_p_next_shift()) == nullptr) break;
        p_right = *reinterpret_cast<void **>(reinterpret_cast<byte *>(p_right) + get_p_next_shift());
    }

    //Если наш блок будет последним
    *reinterpret_cast<void **>(reinterpret_cast<byte *>(p_right) + get_p_next_shift()) = at;
    *reinterpret_cast<void **>(reinterpret_cast<byte *>(at) + get_p_prev_shift()) = p_right;
    *reinterpret_cast<void **>(reinterpret_cast<byte *>(at) + get_p_next_shift()) = nullptr;
    *(reinterpret_cast<byte *>(at) + get_size_of_free_block_shift()) = size_of_block;
    unite_block(at);
}

inline void allocator_buddies_system::set_fit_mode(
    allocator_with_fit_mode::fit_mode mode)
{
    if(_trusted_memory == nullptr) 
    {
        return;
    }

    std::lock_guard<std::mutex> locker(*reinterpret_cast<std::mutex *>(reinterpret_cast<byte *>(_trusted_memory)));
    *reinterpret_cast<allocator_with_fit_mode::fit_mode *>(reinterpret_cast<byte *>(_trusted_memory) + get_fit_mode_shift()) = mode;
}

inline allocator *allocator_buddies_system::get_allocator() const
{
    if(_trusted_memory == nullptr) 
    {
        return nullptr;
    }

    return *reinterpret_cast<allocator **>(reinterpret_cast<byte *>(_trusted_memory) + get_allocator_shift());
}

std::vector<allocator_test_utils::block_info> allocator_buddies_system::get_blocks_info() const noexcept
{
    std::vector<allocator_test_utils::block_info> result;
    if(_trusted_memory == nullptr) 
    {
        return result;
    }

    auto size_of_trusted_memory =  *reinterpret_cast<size_t *>(reinterpret_cast<byte *>(_trusted_memory) + get_size_of_trusted_memory_shift());
    byte *p_now = reinterpret_cast<byte *>(_trusted_memory) + get_global_meta_size(),
        *p_last = reinterpret_cast<byte *>(_trusted_memory) + size_of_trusted_memory;

    while(p_now < p_last) 
    {
        bool is_occupated = (*reinterpret_cast<void **>(p_now) == _trusted_memory);
        byte shift = *(p_now + (is_occupated ? get_size_of_occupied_block_shift() : get_size_of_free_block_shift()));
        size_t size_of_block = 1 << shift;
        result.push_back(allocator_test_utils::block_info
            {
            size_of_block, 
            is_occupated
            });
        p_now = reinterpret_cast<byte *>(p_now) + size_of_block;
    }

    return result;
}

inline logger *allocator_buddies_system::get_logger() const
{
    if(_trusted_memory == nullptr)
    {
        return nullptr;
    }

    return *reinterpret_cast<logger **>(reinterpret_cast<byte *>(_trusted_memory) + get_logger_shift());
}

inline std::string allocator_buddies_system::get_typename() const noexcept
{
    return ("[allocator_buddies_system]");
}

//----------------------------------------------------------------------------

constexpr size_t allocator_buddies_system::get_global_meta_size() 
{
    return sizeof(size_t) + sizeof(allocator *) + sizeof(logger *) + sizeof(void *) + sizeof(allocator_with_fit_mode::fit_mode) + sizeof(std::mutex);
}

constexpr size_t allocator_buddies_system::get_free_block_meta_size() 
{
    return 2 * sizeof(void *) + sizeof(byte);
}

constexpr size_t allocator_buddies_system::get_occupied_block_meta_size() 
{
    return sizeof(byte) + sizeof(void *);
}

constexpr size_t allocator_buddies_system::get_size_of_trusted_memory_shift() 
{
    return 0;
}

constexpr size_t allocator_buddies_system::get_allocator_shift() 
{
    return sizeof(size_t);
}

constexpr size_t allocator_buddies_system::get_logger_shift() 
{
    return get_allocator_shift() + sizeof(allocator *);
}

constexpr size_t allocator_buddies_system::get_first_free_block_shift() 
{
    return get_logger_shift() + sizeof(logger *);
}

constexpr size_t allocator_buddies_system::get_fit_mode_shift() 
{
    return get_first_free_block_shift() + sizeof(void *);
}

constexpr size_t allocator_buddies_system::get_mutex_shift() 
{
    return get_fit_mode_shift() + sizeof(allocator_with_fit_mode::fit_mode);
}

constexpr size_t allocator_buddies_system::get_p_prev_shift() 
{
    return 0;
}

constexpr size_t allocator_buddies_system::get_p_next_shift() 
{
    return sizeof(void *);
}

constexpr size_t allocator_buddies_system::get_size_of_free_block_shift() 
{
    return get_p_next_shift() + sizeof(void *);
}

constexpr size_t allocator_buddies_system::get_tm_pointer_shift() 
{
    return 0;
}

constexpr size_t allocator_buddies_system::get_size_of_occupied_block_shift() 
{
    return sizeof(void *);
}

void *allocator_buddies_system::allocate_first_fit(byte size_of_new_block)
{
    void *cur_block = *reinterpret_cast<void **>(reinterpret_cast<byte *>(_trusted_memory) + get_first_free_block_shift());

    while(cur_block != nullptr) 
    {
        byte size_of_cur_block = *(reinterpret_cast<byte *>(cur_block) + get_size_of_free_block_shift());
        if(size_of_cur_block == size_of_new_block) 
        {
            return allocate_block(cur_block);
        }

        if(size_of_cur_block > size_of_new_block) 
        {
            return divide_block(cur_block, size_of_new_block);
        }

        cur_block = *reinterpret_cast<void **>(reinterpret_cast<byte *>(cur_block) + get_p_next_shift());
    }

    throw std::bad_alloc();
}

void *allocator_buddies_system::allocate_best_fit(byte size_of_new_block) 
{
    void *best_block = nullptr,
    *cur_block = *reinterpret_cast<void **>(reinterpret_cast<byte *>(_trusted_memory) + get_first_free_block_shift());
    byte size_of_best_block = 0;

    while(cur_block != nullptr) 
    {
        byte size_of_cur_block = *(reinterpret_cast<byte *>(cur_block) + get_size_of_free_block_shift());

        if(size_of_cur_block >= size_of_new_block) 
        {
            if(size_of_cur_block == size_of_new_block) 
            {
                return allocate_block(cur_block);
            }
            if(best_block == nullptr || size_of_best_block > size_of_cur_block) 
            {
                best_block = cur_block;
                size_of_best_block = size_of_cur_block;
            }
        }
        cur_block = *reinterpret_cast<void **>(reinterpret_cast<byte *>(cur_block) + get_p_next_shift());
    }

    if(best_block == nullptr) 
    {
        throw std::bad_alloc();
    }

    return divide_block(best_block, size_of_new_block);
}

void *allocator_buddies_system::allocate_worst_fit(byte size_of_new_block) 
{
    void *worst_block = nullptr,
    *cur_block = *reinterpret_cast<void **>(reinterpret_cast<byte *>(_trusted_memory) + get_first_free_block_shift());
    byte size_of_worst_block = 0;
    while(cur_block != nullptr) 
    {
        byte size_of_cur_block = *(reinterpret_cast<byte *>(cur_block) + get_size_of_free_block_shift());
        if(size_of_cur_block >= size_of_new_block) 
        {
            if(worst_block == nullptr || size_of_worst_block < size_of_cur_block) 
            {
                worst_block = cur_block;
                size_of_worst_block = size_of_cur_block;
            }
        }
        cur_block = *reinterpret_cast<void **>(reinterpret_cast<byte *>(cur_block) + get_p_next_shift());
    }
    if(worst_block == nullptr) 
    {
        throw std::bad_alloc();
    }
    if(size_of_worst_block == size_of_new_block) 
    {
        return allocate_block(cur_block);
    }

    return divide_block(worst_block, size_of_new_block);
}

void *allocator_buddies_system::allocate_block(void *block) 
{
    void *previous_block = *reinterpret_cast<void **>(reinterpret_cast<byte *>(block) + get_p_prev_shift()),
    *next_block = *reinterpret_cast<void **>(reinterpret_cast<byte *>(block) + get_p_next_shift());
    byte size_of_block = *(reinterpret_cast<byte *>(block) + get_size_of_free_block_shift());

    if(previous_block != nullptr)
    {
        *reinterpret_cast<void **>(reinterpret_cast<byte *>(previous_block) + get_p_next_shift()) = next_block;
    }
    else
    {
        *reinterpret_cast<void **>(reinterpret_cast<byte *>(_trusted_memory) + get_first_free_block_shift()) = next_block;
    }

    if(next_block != nullptr)
    {
        *reinterpret_cast<void **>(reinterpret_cast<byte *>(next_block) + get_p_prev_shift()) = previous_block;
    }

    *reinterpret_cast<void **>(reinterpret_cast<byte *>(block) + get_tm_pointer_shift()) = _trusted_memory;
    *(reinterpret_cast<byte *>(block) + get_size_of_occupied_block_shift()) = size_of_block;

    return reinterpret_cast<byte *>(block) + get_occupied_block_meta_size();
}

void *allocator_buddies_system::divide_block(void *block, byte requestedSize) 
{
    byte size_of_block = *(reinterpret_cast<byte *>(block) + get_size_of_free_block_shift());

    while(size_of_block > requestedSize) 
    {
        void *next_block = *reinterpret_cast<void **>(reinterpret_cast<byte *>(block) + get_p_next_shift());
        --size_of_block;

        //заполним мету двойника
        void *buddy = reinterpret_cast<byte *>(block) + (1 << size_of_block);
        *reinterpret_cast<void **>(reinterpret_cast<byte *>(buddy) + get_p_prev_shift()) = block;
        *reinterpret_cast<void **>(reinterpret_cast<byte *>(buddy) + get_p_next_shift()) = next_block;

        if(next_block != nullptr) 
        {
            *reinterpret_cast<void **>(reinterpret_cast<byte *>(next_block) + get_p_prev_shift()) = buddy;
        }

        *(reinterpret_cast<byte *>(buddy) + get_size_of_free_block_shift()) = size_of_block;

        //Заполним нашу мету
        *reinterpret_cast<void **>(reinterpret_cast<byte *>(block) + get_p_next_shift()) = buddy;
        *(reinterpret_cast<byte *>(block) + get_size_of_free_block_shift()) = size_of_block;
    }
    return allocate_block(block);
}

void allocator_buddies_system::unite_block(void *at) {
    if(at == nullptr) 
    {
        throw;
    }

    void *p_prev = *reinterpret_cast<void **>(reinterpret_cast<byte *>(at) + get_p_prev_shift()),
         *p_next = *reinterpret_cast<void **>(reinterpret_cast<byte *>(at) + get_p_next_shift());

    if(p_prev == nullptr && p_next == nullptr) 
    {
        return;
    }

    byte sizeOfAt = *(reinterpret_cast<byte *>(at) + get_size_of_free_block_shift());
    size_t relative_ptr_of_at = get_relative_ptr(at),
    relative_ptr_of_buddy = relative_ptr_of_at ^ (1 << sizeOfAt);
    void *buddy = reinterpret_cast<byte *>(_trusted_memory) + get_global_meta_size() + relative_ptr_of_buddy;
    bool buddyIsOcupied = *reinterpret_cast<void **>(reinterpret_cast<byte *>(buddy) + get_tm_pointer_shift()) == _trusted_memory;

    if(buddyIsOcupied) 
    {
        return;
    }

    byte sizeOfBuddy = *(reinterpret_cast<byte *>(buddy) + get_size_of_free_block_shift());

    if(sizeOfAt != sizeOfBuddy) 
    {
        return;
    }

    if(at < buddy) 
    {
        std::swap(at, buddy);
    }
    ++*(reinterpret_cast<byte *>(buddy) + get_size_of_free_block_shift());
    p_next = *reinterpret_cast<void **>(reinterpret_cast<byte *>(at) + get_p_next_shift());
    *reinterpret_cast<void **>(reinterpret_cast<byte *>(buddy) + get_p_next_shift()) = p_next;

    if(p_next != nullptr)
    {
        *reinterpret_cast<void **>(reinterpret_cast<byte *>(p_next) + get_p_prev_shift()) = buddy;
    }
    unite_block(buddy);
}

size_t allocator_buddies_system::get_relative_ptr(void *at) 
{
    byte *p_first = reinterpret_cast<byte *>(_trusted_memory) + get_global_meta_size();

    return reinterpret_cast<byte *>(at) - p_first;
}


byte allocator_buddies_system::get_minimal_shift_of_block_size(size_t const &size_of_block) 
{
    //for(shift = 0; size_of_block < (1 << shift); ++shift){}
    byte minimal_shift = static_cast<byte>(std::log2(get_free_block_meta_size()));

    if(size_of_block <= get_free_block_meta_size()) 
    {
        return minimal_shift + 1;
    }
    byte shift = 0;

    while(size_of_block > (1 << shift++)){}

    return shift - 1;
}