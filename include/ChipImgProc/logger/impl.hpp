#pragma once
#include "level.hpp"
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <Nucleona/language.hpp>
namespace chipimgproc {

constexpr struct Logger {
private:
    auto& console_sink() const {
        static auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        return console_sink;
    }
    auto& file_sink() const {
        static auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>("chipimgproc.log", true);
        return file_sink;
    }
    auto create_logger() const {
        auto p_log = std::make_shared<spdlog::logger>("CIMP", console_sink());
        p_log->sinks().push_back(file_sink());
        p_log->set_pattern("[%Y-%m-%d %H:%M:%S.%e][%n][%l][thread %t] %v");
        return p_log;
    }
    spdlog::logger& core() const {
        static auto p_log(create_logger());
        return *p_log;
    }
public:
    template<class... Args>
    void trace(Args&&... args) const {
        core().trace(FWD(args)...);
    }
    
    template<class... Args>
    void debug(Args&&... args) const {
        core().debug(FWD(args)...);
    }
    
    template<class... Args>
    void info(Args&&... args) const {
        core().info(FWD(args)...);
    }
    
    template<class... Args>
    void warn(Args&&... args) const {
        core().warn(FWD(args)...);
    }
    
    template<class... Args>
    void error(Args&&... args) const {
        core().error(FWD(args)...);
    }

    template<class... Args>
    void critical(Args&&... args) const {
        core().critical(FWD(args)...);
    }
    
    void set_level(int n) const {
        auto level = logger::level_trans(n);
        console_sink()->set_level(level);
        file_sink()->set_level(level);
        core().set_level(level);
    }

} log;

}