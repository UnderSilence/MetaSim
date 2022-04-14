#include "Utils/logger.hpp"

namespace MS {

std::shared_ptr<spdlog::logger> MetaLog::s_logger;
void MetaLog::init() {
  s_logger = spdlog::stdout_color_mt("Sim");
  s_logger->set_pattern("[%^%l%$]\t%v");
  s_logger->set_level(spdlog::level::level_enum::trace);
}
}   // namespace MS