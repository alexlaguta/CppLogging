/*!
    \file logger.inl
    \brief Logger interface inline implementation
    \author Ivan Shynkarenka
    \date 29.07.2016
    \copyright MIT License
*/

namespace CppLogging {

inline Logger::Logger(const std::string& name, const std::shared_ptr<Processor>& sink) : _name(name), _sink(sink)
{
}

inline Logger::~Logger()
{
    Flush();
}

template <typename... Args>
inline void Logger::Log(Level level, bool format, std::string_view message, Args&&... args)
{
    // Thread local thread Id
    thread_local uint64_t thread = CppCommon::Thread::CurrentThreadId();
    // Thread local instance of the logging record
    thread_local Record record;

    // Clear the logging record
    record.Clear();

    // Fill necessary fields of the logging record
    record.timestamp = CppCommon::Timestamp::utc();
    record.thread = thread;
    record.level = level;
    record.logger = _name;

    // Check for valid and started logging sink
    if (_sink && _sink->IsStarted())
    {
        // Filter the logging record
        if (!_sink->FilterRecord(record))
            return;

        // Format or serialize arguments list
        if (format)
            record.Format(message, std::forward<Args>(args)...);
        else
            record.StoreFormat(message, std::forward<Args>(args)...);

        // Process the logging record
        _sink->ProcessRecord(record);
    }
}

template <typename... Args>
inline void Logger::Debug(std::string_view debug, Args&&... args)
{
#if defined(NDEBUG)
    // Log nothing in release mode...
#else
    Log(Level::DEBUG, false, debug, std::forward<Args>(args)...);
#endif
}

template <typename... Args>
inline void Logger::Info(std::string_view info, Args&&... args)
{
    Log(Level::INFO, false, info, std::forward<Args>(args)...);
}

template <typename... Args>
inline void Logger::Warn(std::string_view warn, Args&&... args)
{
    Log(Level::WARN, false, warn, std::forward<Args>(args)...);
}

template <typename... Args>
inline void Logger::Error(std::string_view error, Args&&... args)
{
    Log(Level::ERROR, false, error, std::forward<Args>(args)...);
}

template <typename... Args>
inline void Logger::Fatal(std::string_view fatal, Args&&... args)
{
    Log(Level::FATAL, false, fatal, std::forward<Args>(args)...);
}

inline void Logger::Flush()
{
    if (_sink && _sink->IsStarted())
        _sink->Flush();
}

} // namespace CppLogging
