#include <mutex>

#include <not_implemented.h>
#include <exception>

#include "../include/allocator_sorted_list.h"

allocator_sorted_list::~allocator_sorted_list() noexcept
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

allocator_sorted_list::allocator_sorted_list(
    allocator_sorted_list&& other) noexcept
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

allocator_sorted_list& allocator_sorted_list::operator=(
    allocator_sorted_list&& other) noexcept
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
        allocator::destruct(&obtain_synchronizer());
        deallocate_with_guard(_trusted_memory);

        _trusted_memory = other._trusted_memory;

        other._trusted_memory = nullptr;
    }

    return *this;
}

allocator_sorted_list::allocator_sorted_list(
    size_t space_size,
    allocator* parent_allocator,
    logger* logger,
    allocator_with_fit_mode::fit_mode allocate_fit_mode)
{
    if (space_size < available_block_metadata_size())
    {
        this->
            error_with_guard(get_typename()
            + " Can not allocate memory size < 0");

        throw std::logic_error("Can't initialize allocator instance");
    }

    size_t memory_size = space_size + common_metadata_size();
    try
    {
        _trusted_memory = parent_allocator == nullptr
                          ? ::operator new (memory_size)
                          : parent_allocator->allocate(1, memory_size);
    }
    catch (std::bad_alloc const& ex)
    {
        this->
            error_with_guard(get_typename() + " " + ex.what());

        throw;
    }

    allocator** parent_allocator_placement = reinterpret_cast<allocator**>(_trusted_memory);
    *parent_allocator_placement = parent_allocator;

    class logger** logger_placement = reinterpret_cast<class logger**>(parent_allocator_placement + 1);
    *logger_placement = logger;

    std::mutex* synchronizer_placement = reinterpret_cast<std::mutex*>(logger_placement + 1);
    allocator::construct(synchronizer_placement);

    unsigned char* placement = reinterpret_cast<unsigned char*>(synchronizer_placement);

    placement += sizeof(std::mutex);
    *reinterpret_cast<allocator_with_fit_mode::fit_mode*>(placement) = allocate_fit_mode;

    placement += sizeof(allocator_with_fit_mode::fit_mode);
    *reinterpret_cast<size_t*>(placement) = space_size;

    placement += sizeof(size_t);
    *reinterpret_cast<void**>(placement) = placement + sizeof(void*);

    // BEFORE:
    //*reinterpret_cast<void **>(*reinterpret_cast<void **>(placement)) = nullptr;

    // AFTER:
    obtain_next_available_block_address(obtain_first_available_block_address_byref()) = nullptr;

    *reinterpret_cast<size_t*>(reinterpret_cast<void**>(*reinterpret_cast<void**>(placement)) + 1) 
        = space_size - available_block_metadata_size();

    this
        ->debug_with_guard(get_typename() 
        + " The object of allocator was created...");
}

[[nodiscard]] void* allocator_sorted_list::allocate(
    size_t value_size,
    size_t values_count)
{
    this->
        debug_with_guard(get_typename() 
        + " Call of the allocate...");

    std::lock_guard<std::mutex> lock(obtain_synchronizer());

    throw_if_allocator_instance_state_was_moved();

    void *target_block = nullptr, *previous_to_target_block = nullptr;
    size_t requested_size = value_size * values_count + ancillary_block_metadata_size();
    size_t target_block_size;

    {
        void *current_block, *previous_block = nullptr;
        current_block = obtain_first_available_block_address_byref();
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
            current_block = obtain_next_available_block_address(current_block);
        }
    }

    //Left the block, local variables are not needed
    //Implement block allocation
    //Logically: We take the block on the left, from the first of our block meta data fill, separate some piece of memory, and after this piece of memory put again the data meta

    if (target_block == nullptr)
    {
        this->
            error_with_guard(get_typename()
            + " The avaliable block was not found...");

        throw std::bad_alloc();
    }

    //Pointer to next free block
    void *next_block = obtain_next_available_block_address(target_block);
    bool remaining_part_left = target_block_size >= requested_size - ancillary_block_metadata_size();
    void *remaining_block;

    if (remaining_part_left)
    {
        remaining_block = reinterpret_cast<void *>(reinterpret_cast<unsigned char *>(target_block)
                                                   + requested_size
            /*+ available_block_metadata_size()*/);

        // TODO: this two instructions are the same
        //obtain_next_available_block_address(remaining_block) = next_block;
        *reinterpret_cast<void **>(remaining_block) = next_block;

        *reinterpret_cast<size_t *>(reinterpret_cast<unsigned char *>(remaining_block) + sizeof(void *)) =
            target_block_size - requested_size;
    }
    else
    {
        // TODO: user request was redefined
        requested_size = target_block_size;
    }

    *(previous_to_target_block != nullptr
        ? reinterpret_cast<void**>(previous_to_target_block)
        : &obtain_first_available_block_address_byref()) = (remaining_part_left
            ? remaining_block
            : next_block);

    // TODO: this two instructions are the same
    // obtain_allocator_trusted_memory_ancillary_block_owner(target_block) = _trusted_memory;
    *reinterpret_cast<void **>(target_block) = _trusted_memory;

    // TODO: this two instructions are the same
    // obtain_ancillary_block_size(target_block) = target_block_size;
    *reinterpret_cast<size_t *>(reinterpret_cast<unsigned char*>(target_block) + sizeof(void*)) = requested_size;

    this->
        debug_with_guard(get_typename()
        + " Memory was allocate succesfully...");

    return reinterpret_cast<void *>(reinterpret_cast<unsigned char*>(target_block) + ancillary_block_metadata_size());
}

void allocator_sorted_list::deallocate(
    void* at)
{
    this->
        debug_with_guard(get_typename()
        + " Call of the deallocate...");

    std::lock_guard<std::mutex> lock(obtain_synchronizer());

    throw_if_allocator_instance_state_was_moved();

    at = reinterpret_cast<void *>(reinterpret_cast<unsigned char *>(at) - ancillary_block_metadata_size());

    if (at == nullptr 
        || at < reinterpret_cast<unsigned char *>(_trusted_memory) + common_metadata_size() 
        || at > reinterpret_cast<unsigned char *>(_trusted_memory) + common_metadata_size() 
            + obtain_trusted_memory_size() - ancillary_block_metadata_size())
    {
        this->
            error_with_guard(get_typename() 
            + " Invalid block address...");

        throw std::logic_error("Invalid block address");
    }

    if (obtain_allocator_trusted_memory_ancillary_block_owner(at) != _trusted_memory)
    {
        this->
            error_with_guard(get_typename() 
            + " Attempt to deallocate block into wrong allocator instance");

        throw std::logic_error("Attempt to deallocate block into wrong allocator instance");
    }

    void *left_available_block = nullptr;
    void *right_available_block = obtain_first_available_block_address_byref();

    while (right_available_block != nullptr && (left_available_block == nullptr || left_available_block < at) && right_available_block > at)
    {
        // TODO: use it if next statement is too hard :/
        // left_available_block = right_available_block;

        right_available_block = obtain_next_available_block_address(left_available_block = right_available_block);
    }

    if (left_available_block == nullptr && right_available_block == nullptr)
    {
        obtain_next_available_block_address(obtain_first_available_block_address_byref() = at) = nullptr;
        // *reinterpret_cast<void **>(obtain_first_available_block_address_byref() = at) = nullptr;

        return;
    }

    // boost

    // chaining available blocks list items
    obtain_next_available_block_address(at) = right_available_block;
    (left_available_block == nullptr
        ? obtain_first_available_block_address_byref()
        : obtain_next_available_block_address(left_available_block)) = at;

    // merge with right block
    if (right_available_block != nullptr && (reinterpret_cast<unsigned char *>(at) + available_block_metadata_size() + obtain_available_block_size(at) == right_available_block))
    {
        obtain_available_block_size(at) += obtain_available_block_size(right_available_block) + available_block_metadata_size();
        obtain_next_available_block_address(at) = obtain_next_available_block_address(right_available_block);
    }

    // merge with left block
    if (left_available_block != nullptr && (reinterpret_cast<unsigned char *>(left_available_block) + available_block_metadata_size() + obtain_available_block_size(left_available_block) == at))
    {
        obtain_available_block_size(left_available_block) += obtain_available_block_size(at) + available_block_metadata_size();
        obtain_next_available_block_address(left_available_block) = obtain_next_available_block_address(at);
    }
}

inline void allocator_sorted_list::set_fit_mode(
    allocator_with_fit_mode::fit_mode mode)
{
    std::lock_guard<std::mutex> lock(obtain_synchronizer());

    obtain_fit_mode() = mode;
}

inline allocator* allocator_sorted_list::get_allocator() const
{
    return *reinterpret_cast<allocator**>(_trusted_memory);
}

std::vector<allocator_test_utils::block_info> allocator_sorted_list::get_blocks_info() const noexcept
{
    std::vector<allocator_test_utils::block_info> blocks_info;

    

    return blocks_info; 
}

inline logger* allocator_sorted_list::get_logger() const
{
    return *(reinterpret_cast<logger**>(&obtain_synchronizer()) - 1);
}

inline std::string allocator_sorted_list::get_typename() const noexcept
{
    return "[allocator_sorted_list]";
}

size_t &allocator_sorted_list::obtain_trusted_memory_size() const
{
    return *reinterpret_cast<size_t *>(&obtain_fit_mode() + 1);
}

size_t constexpr allocator_sorted_list::common_metadata_size()
{
    return sizeof(allocator*) + sizeof(std::mutex) + sizeof(logger*) + sizeof(allocator_with_fit_mode::fit_mode) + sizeof(size_t) + sizeof(void*);
}

size_t constexpr allocator_sorted_list::available_block_metadata_size()
{
    return sizeof(size_t) + sizeof(void*);
}

size_t constexpr allocator_sorted_list::ancillary_block_metadata_size()
{
    // return sizeof(size_t) + sizeof(void *);
    return sizeof(block_size_t) + sizeof(void*);
}

std::mutex& allocator_sorted_list::obtain_synchronizer() const
{
    return *reinterpret_cast<std::mutex*>(const_cast<unsigned char*>(reinterpret_cast<unsigned char const*>(_trusted_memory) + sizeof(allocator*) + sizeof(logger*)));
}

void*& allocator_sorted_list::obtain_first_available_block_address_byref() const
{
    return *reinterpret_cast<void**>(reinterpret_cast<unsigned char*>(_trusted_memory)
                                     + sizeof(allocator*)
                                     + sizeof(logger*)
                                     + sizeof(std::mutex)
                                     + sizeof(allocator_with_fit_mode::fit_mode)
                                     + sizeof(size_t));
}

void** allocator_sorted_list::obtain_first_available_block_address_byptr() const
{
    return reinterpret_cast<void**>(reinterpret_cast<unsigned char*>(_trusted_memory)
                                    + sizeof(allocator*)
                                    + sizeof(logger*)
                                    + sizeof(std::mutex)
                                    + sizeof(allocator_with_fit_mode::fit_mode)
                                    + sizeof(size_t));
}

void *& allocator_sorted_list::obtain_allocator_trusted_memory_ancillary_block_owner(
    void *current_ancillary_block_address)
{
    return obtain_next_available_block_address(current_ancillary_block_address);
}

size_t &allocator_sorted_list::obtain_ancillary_block_size(
    void *current_ancillary_block_address)
{
    return *reinterpret_cast<size_t *>(reinterpret_cast<void **>(current_ancillary_block_address) + 1);
}

void*& allocator_sorted_list::obtain_next_available_block_address(
    void* current_available_block_address)
{
    return *reinterpret_cast<void**>(current_available_block_address);
}

size_t& allocator_sorted_list::obtain_available_block_size(
    void* current_available_block_address)
{
    return *reinterpret_cast<size_t*>(&obtain_next_available_block_address(current_available_block_address) + 1);

    // return *reinterpret_cast<size_t *>(reinterpret_cast<void **>(current_available_block_address) + 1);

    // return *reinterpret_cast<size_t *>(reinterpret_cast<unsigned char *>(current_available_block_address) + sizeof(void *));
}

allocator_with_fit_mode::fit_mode& allocator_sorted_list::obtain_fit_mode() const
{
    return *reinterpret_cast<allocator_with_fit_mode::fit_mode*>(reinterpret_cast<unsigned char*>(_trusted_memory) + sizeof(allocator*) + sizeof(logger*) + sizeof(std::mutex));
}

void allocator_sorted_list::throw_if_allocator_instance_state_was_moved() const
{
    if (_trusted_memory == nullptr)
    {
        this->
            error_with_guard(get_typename() 
            + " Allocator instance state was moved :/");

        throw std::logic_error("Allocator instance state was moved :/");
    }
}