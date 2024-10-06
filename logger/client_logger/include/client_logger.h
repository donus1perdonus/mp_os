#ifndef MATH_PRACTICE_AND_OPERATING_SYSTEMS_CLIENT_LOGGER_H
#define MATH_PRACTICE_AND_OPERATING_SYSTEMS_CLIENT_LOGGER_H

#include <logger.h>
#include <nlohmann/json.hpp>
#include <filesystem> // C++ STANDART 17+
#include <fstream>
#include <map>

class client_logger final:
    public logger
{

private:

    std::map<std::string, unsigned char> _streams;

    std::string _format;

    static std::unordered_map<std::string, std::pair<std::ofstream*, int>> _all_streams;

    client_logger(std::map<std::string, unsigned char> files,
        std::string const& format);

    friend class client_logger_builder;

public:

    client_logger(
        client_logger const &other);

    client_logger &operator=(
        client_logger const &other);

    client_logger(
        client_logger &&other) noexcept;

    client_logger &operator=(
        client_logger &&other) noexcept;

    ~client_logger() noexcept final;

public:

    [[nodiscard]] logger const *log(
        const std::string &message,
        logger::severity severity) const noexcept override;
 
private:

    std::string string_with_format(
        std::string const & message,
        logger::severity severity) const;

};

#endif //MATH_PRACTICE_AND_OPERATING_SYSTEMS_CLIENT_LOGGER_H