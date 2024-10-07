#include "../include/client_logger.h"

std::unordered_map<std::string, 
    std::pair<std::ofstream*, int>> 
    client_logger::_all_streams;

client_logger::client_logger(
    std::map<std::string, unsigned char> files,
    std::string const& format) 
    : _streams(files), 
    _format(format)
{
    for (const auto& pair : _streams) 
    {
        if(pair.first != "cerr")
        {
            auto it = _all_streams.find(pair.first);

            if (it != _all_streams.end())
            {
                it->second.second += 1;

                continue;
            }

            std::ofstream* file = new std::ofstream;

            file->open(pair.first);

            if (!file->is_open())
            {
                std::cerr 
                << ("File or directory does not exist: " + pair.first);

                delete file;
            }

            _all_streams[pair.first] = {file, 1};
        }
    }
}

client_logger::client_logger(
    client_logger const &other) 
    : _streams(other._streams),
    _format(other._format)
{
}

client_logger &client_logger::operator=(
    client_logger const &other)
{
    if(&other != this)
    {
        _streams = other._streams;
        _format = other._format;
    }

    return *this;
}

client_logger::client_logger(
    client_logger &&other) noexcept
    : _streams(std::move(other._streams)),
    _format(std::move(other._format))
{
}

client_logger &client_logger::operator=(
    client_logger &&other) noexcept
{
    if(&other != this)
    {
        _streams = std::move(other._streams);
        _format = std::move(other._format);
    }

    return *this;
}

client_logger::~client_logger() noexcept
{
    for (const auto& pair : _streams) 
    {
        if(pair.first != "cerr")
        {
            auto it = _all_streams.find(pair.first);

            if (it != _all_streams.end())
            {
                it->second.second -= 1;

                if(it->second.second == 0)
                {
                    it->second.first->close();

                    delete it->second.first;

                    _all_streams.erase(it);
                }
            }
        }
    }
}

std::string client_logger::string_with_format(
    std::string const & message,
    logger::severity severity) const
{
    std::string result;

    auto time = std::time(nullptr);

    std::ostringstream result_stream;

    for(int i = 0; i < _format.size(); i++)
    {
        if(_format[i] == '%' && _format[i + 1] == 'd')
        {
            result += current_datetime_to_string().substr(0, 10);

            i++;

            continue;
        }
        else if(_format[i] == '%' && _format[i + 1] == 't')
        {
            result += current_datetime_to_string().substr(11, 9);

            i++;

            continue;
        }
        else if(_format[i] == '%' && _format[i + 1] == 's')
        {
            switch (severity)
            {
                case logger::severity::trace:
                    result += "TRACE";
                    break;
                case logger::severity::debug:
                    result += "DEBUG";
                    break;
                case logger::severity::information:
                    result += "INFORMATION";
                    break;
                case logger::severity::warning:
                    result += "WARNING";
                    break;
                case logger::severity::error:
                    result += "ERROR";
                    break;
                case logger::severity::critical:
                    result += "CRITICAL";
                    break;
            }

            i++;

            continue;
        }
        else if(_format[i] == '%' && _format[i + 1] == 'm')
        {
            result += message;

            i++;

            continue;
        }

        result += _format[i];
    }

    return result;
}
        

logger const *client_logger::log(
    const std::string &text,
    logger::severity severity) const noexcept
{
    for (const auto& pair : _streams) 
    {
        if((pair.second & (1 << static_cast<int>(severity))) != 0)
        {
            std::string result;

            result = string_with_format(text, severity);

            if(pair.first != "cerr")
            {   
                *_all_streams[pair.first].first << result << std::endl;
            }
            else
            {
                std::cerr << result << std::endl;
            }
        }
    }

    return this;
}