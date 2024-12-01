#ifndef MATH_PRACTICE_AND_OPERATING_SYSTEMS_ALLOCATOR_ALLOCATOR_BOUNDARY_TAGS_H
#define MATH_PRACTICE_AND_OPERATING_SYSTEMS_ALLOCATOR_ALLOCATOR_BOUNDARY_TAGS_H

#include <allocator_guardant.h>
#include <allocator_test_utils.h>
#include <allocator_with_fit_mode.h>
#include <logger_guardant.h>
#include <typename_holder.h>

using byte = unsigned char;

class allocator_boundary_tags final:
    private allocator_guardant,
    public allocator_test_utils,
    public allocator_with_fit_mode,
    private logger_guardant,
    private typename_holder
{

private:
    
    void *_trusted_memory;

public:
    
    ~allocator_boundary_tags() override;
    
    allocator_boundary_tags(
        allocator_boundary_tags const &other) = delete;
    
    allocator_boundary_tags &operator=(
        allocator_boundary_tags const &other) = delete;
    
    allocator_boundary_tags(
        allocator_boundary_tags &&other) noexcept;
    
    allocator_boundary_tags &operator=(
        allocator_boundary_tags &&other) noexcept;

public:
    
    explicit allocator_boundary_tags(
        size_t space_size,
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

    static constexpr size_t obtain_global_metadata_size();
    static constexpr size_t obtain_block_metadata_size();

    static constexpr size_t get_trusted_memory_shift();
    static constexpr size_t get_logger_shift();
    static constexpr size_t get_allocator_shift();
    static constexpr size_t get_fitmode_shift();
    static constexpr size_t get_mutex_shift();
    static constexpr size_t get_first_block_shift();

    static constexpr size_t get_tm_ptr_shift();
    static constexpr size_t get_status_of_block_shift();
    static constexpr size_t get_size_of_block_shift();

    void initialize_block_metadata(byte *, size_t, bool);

    void *allocate_first_fit(size_t);
    void *allocate_best_fit(size_t);
    void *allocate_worst_fit(size_t);

    void *allocate_block(void *, size_t);
    void *allocate_full_block(void *);

    byte *find_first_free_block(size_t) const;
    
};

#endif //MATH_PRACTICE_AND_OPERATING_SYSTEMS_ALLOCATOR_ALLOCATOR_BOUNDARY_TAGS_H