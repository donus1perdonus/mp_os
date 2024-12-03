#ifndef MATH_PRACTICE_AND_OPERATING_SYSTEMS_ALLOCATOR_ALLOCATOR_BUDDIES_SYSTEM_H
#define MATH_PRACTICE_AND_OPERATING_SYSTEMS_ALLOCATOR_ALLOCATOR_BUDDIES_SYSTEM_H

#include <allocator_guardant.h>
#include <allocator_test_utils.h>
#include <allocator_with_fit_mode.h>
#include <logger_guardant.h>
#include <typename_holder.h>

using byte = unsigned char;

class allocator_buddies_system final:
    private allocator_guardant,
    public allocator_test_utils,
    public allocator_with_fit_mode,
    private logger_guardant,
    private typename_holder
{

private:

    void *_trusted_memory;

public:
    
    ~allocator_buddies_system() override;
    
    allocator_buddies_system(
        allocator_buddies_system const &other) = delete;
    
    allocator_buddies_system &operator=(
        allocator_buddies_system const &other) = delete;
    
    allocator_buddies_system(
        allocator_buddies_system &&other) noexcept;
    
    allocator_buddies_system &operator=(
        allocator_buddies_system &&other) noexcept;

public:
    
    explicit allocator_buddies_system(
        size_t space_size_power_of_two,
        allocator *parent_allocator = nullptr,
        logger *logger = nullptr,
        allocator_with_fit_mode::fit_mode allocate_fit_mode = allocator_with_fit_mode::fit_mode::first_fit);

public:
    
    [[nodiscard]] void *allocate(
        size_t value_size,
        size_t values_count) override;
    
    void deallocate(
        void *at) override;

public:
    
    inline void set_fit_mode(
        allocator_with_fit_mode::fit_mode mode) override;

private:
    
    inline allocator *get_allocator() const override;

public:
    
    std::vector<allocator_test_utils::block_info> get_blocks_info() const noexcept override;

private:
    
    inline logger *get_logger() const override;

private:
    
    inline std::string get_typename() const noexcept override;

private:

    static constexpr size_t get_global_meta_size();
    static constexpr size_t get_free_block_meta_size();
    static constexpr size_t get_occupied_block_meta_size();

    static constexpr size_t get_size_of_trusted_memory_shift();
    static constexpr size_t get_allocator_shift();
    static constexpr size_t get_logger_shift();
    static constexpr size_t get_first_free_block_shift();
    static constexpr size_t get_fit_mode_shift();
    static constexpr size_t get_mutex_shift();

    static constexpr size_t get_p_prev_shift();
    static constexpr size_t get_p_next_shift();
    static constexpr size_t get_size_of_free_block_shift();

    static constexpr size_t get_tm_pointer_shift();
    static constexpr size_t get_size_of_occupied_block_shift();

    void *allocate_first_fit(byte);
    void *allocate_best_fit(byte);
    void *allocate_worst_fit(byte);

    void *allocate_block(void *);

    void *divide_block(void *, byte);
    void unite_block(void *);

    inline size_t get_relative_ptr(void *);

    static byte get_minimal_shift_of_block_size(size_t const &);
};

#endif //MATH_PRACTICE_AND_OPERATING_SYSTEMS_ALLOCATOR_ALLOCATOR_BUDDIES_SYSTEM_H