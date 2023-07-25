#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <ctime>
#include <map>
#include <numeric>
#include <string>
#include <vector>

template<typename StdType, typename... ArgTypes>
auto log(StdType stream, ArgTypes... args) {
  fprintf(stream, args...);
}

struct KSYSTEM_TIME {
  uint32_t LowPart;
  int32_t  High1Time;
  int32_t  High2Time;

  auto asquad(void) {
    return *reinterpret_cast<int64_t*>(this);
  }
};
using PKSYSTEM_TIME = KSYSTEM_TIME*;

std::string tmtostr(const int64_t& tm, const char* fm) {
  char buf[std::size("yyyy-mm-dd hh:mm:ss")];
  auto val = (tm - 116444736000000000) / 10000000;
  std::strftime(std::data(buf), std::size(buf), fm, std::gmtime(&val));
  return buf;
}

auto tmblock(const int& tp) {
  std::vector<int64_t> vals{0x7FFE0008, 0x7FFE0014, 0x7FFE0020};
  std::transform(vals.begin(), vals.end(), vals.begin(), [](const int64_t addr){
    return (*reinterpret_cast<PKSYSTEM_TIME>(addr)).asquad();
  });

  switch (tp) {
    case 0:
      [&vals](void) {
        auto sec = vals[0] / 10000000;
        auto stm = tmtostr(vals[1] - vals[2], "%H:%M:%S");
        log(stdout, "%s up %lld.%.2lld:%.2lld:%.2lld\n",
          stm.c_str(), sec / 86400, sec / 3600 % 24, sec % 3600 / 60, sec % 60
        );
      }();
      break;

    case 1:
      [&vals](void) {
        auto sec = vals[0] / 10000000;
        std::map<std::string, int64_t> parts {
          {"day",    sec / 86400},
          {"hour",   sec / 3600 % 24},
          {"minute", sec % 3600 / 60},
          {"second", sec % 60}
        };
        auto res = std::accumulate(parts.begin(), parts.end(), std::string(),
          [](const std::string& s, const std::pair<const std::string, int64_t>& p) {
            if (!p.second) return s;
            return s + std::to_string(p.second) + " " + p.first + (p.second > 1 ? "s" : "") + ", ";
          }
        );
        log(stdout, "%.*s\n", static_cast<int>(res.size() - 2), res.c_str());
      }();
      break;

    case 2:
      log(stdout, "%s\n", tmtostr(vals[1] - vals[2] - vals[0], "%Y-%m-%d %H:%M:%S").c_str());
      break;
  }
}

int main(int argc, char** argv) {
  switch (argc) {
    case 1:
      tmblock(0);
      break;

    case 2:
      if (0 == _stricmp(argv[1], "-p")) tmblock(1);
      else if (0 == _stricmp(argv[1], "-s")) tmblock(2);
      else if (0 == _stricmp(argv[1], "-h")) {
        log(stdout, "Usage: uptime [option]\n\nOptions:\n");
        log(stdout, "  -p - show uptime in pretty format\n");
        log(stdout, "  -h - display this message and exit\n");
        log(stdout, "  -s - system up since\n");
      }
      else log(stderr, "[!] Unknown option has been specified.\nUse -h option to get help.\n");
      break;

    default:
      log(stderr, "[!] Option index is out of range.\nSpecify -h to get available options.\n");
      break;
  }

  return 0;
}
