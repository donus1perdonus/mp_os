#include "../include/client_logger_builder.h"

inline std::string const client_logger_builder::get_absolute_path(
    std::string const& file_path)
{
    return std::filesystem::absolute(file_path).string();
}

logger_builder* client_logger_builder::set_format(std::string const& format)
{
    _format = format;

    return this;
}

client_logger_builder::client_logger_builder(
    client_logger_builder const &other) 
    : _files(other._files)
{
}

client_logger_builder &client_logger_builder::operator=(
    client_logger_builder const &other)
{
    if (&other != this)
    {
        _files = other._files;
    }

    return *this; 
}

client_logger_builder::client_logger_builder(
    client_logger_builder &&other) noexcept
    : _files(std::move(other._files))
{
}

client_logger_builder &client_logger_builder::operator=(
    client_logger_builder &&other) noexcept
{
    if (&other != this)
    {
        _files = std::move(other._files);
    }

    return *this; 
}

logger_builder *client_logger_builder::add_file_stream(
    std::string const &stream_file_path,
    logger::severity severity)
{
    std::string const file_path = get_absolute_path(stream_file_path);

    // установка битового поля для каждого severity
    _files[file_path] |= (1 << static_cast<int>(severity));
    
    return this;
}

logger_builder *client_logger_builder::add_console_stream(
    logger::severity severity)
{
    _files["cerr"] |= (1 << static_cast<int>(severity));
    
    return this;
}

logger_builder* client_logger_builder::transform_with_configuration(
    std::string const &configuration_file_path,
    std::string const &configuration_path)
{
    std::ifstream file(configuration_file_path);

    if(!file.is_open())
    throw std::runtime_error("File or directory does not exist: " + configuration_file_path);

    nlohmann::json data;
    file >> data;

    // Доступ к вложенным полям
    for (const auto& file_path : data[configuration_path]) 
    {
        std::string path = file_path["path"];
        auto severity = file_path["severity"];

        for (const std::string& s : severity) 
        {
            this->add_file_stream(path, string_to_severity(s));
        }
    }

    // Закрытие файла
    file.close();

    return this;
}

logger_builder *client_logger_builder::clear()
{
    _files.erase(_files.begin(), _files.end());

    return this;
}

logger *client_logger_builder::build() const
{
    return new client_logger(this->_files, this->_format);
}