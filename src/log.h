#pragma once
// Logging próprio em arquivo, na raiz do jogo (cwd do processo = pasta do exe).
// Não depende de loader.dll (evita o ABI MSVC do loader::LOG).
#include <cstdio>
#include <ctime>
#include <mutex>
#include <string>

namespace scr {
inline std::mutex& log_mutex() { static std::mutex m; return m; }

inline void logf(const char* level, const char* fmt, ...) {
  std::lock_guard<std::mutex> lock(log_mutex());
  FILE* f = std::fopen("SkippableCutscenesRevival.log", "a");
  if (!f) return;
  std::time_t t = std::time(nullptr);
  char stamp[32];
  std::strftime(stamp, sizeof(stamp), "%H:%M:%S", std::localtime(&t));
  std::fprintf(f, "[ %s ] [%s] ", stamp, level);
  va_list args;
  va_start(args, fmt);
  std::vfprintf(f, fmt, args);
  va_end(args);
  std::fputc('\n', f);
  std::fclose(f);
}
}  // namespace scr

#define SCR_INFO(...) ::scr::logf("INFO", __VA_ARGS__)
#define SCR_WARN(...) ::scr::logf("WARN", __VA_ARGS__)
#define SCR_ERR(...)  ::scr::logf("ERR",  __VA_ARGS__)
