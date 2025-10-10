#include <iostream>
#include <cctype>
#include <chrono>
#include <format>
#include <string>
#include <string_view>
#if defined(__linux__)
   #include <sys/sysinfo.h>
#endif

auto get_seconds() {
   using seconds = std::chrono::seconds;
#if defined(_WIN64)
   return seconds(*reinterpret_cast<int64_t*>(0x7FFE0008)) / 10000000LL;
#elif defined(__linux__)
   struct sysinfo si{};
   sysinfo(&si);
   return seconds(si.uptime);
#else
   return seconds(0LL);
#endif
}

auto get_local_time() {
   return std::format("{:%T}", std::chrono::zoned_time{
      std::chrono::current_zone(), std::chrono::floor<std::chrono::seconds>(
         std::chrono::system_clock::now()
      )
   });
}

struct uptime {
   using days = std::chrono::days;
   using hours = std::chrono::hours;
   using minutes = std::chrono::minutes;
   using seconds = std::chrono::seconds;
   
   days d{};
   hours h{};
   minutes m{};
   seconds s{};
   
   explicit uptime(seconds total) : sec(total) {
      d = std::chrono::duration_cast<days>(total);
      total -= d;
      h = std::chrono::duration_cast<hours>(total);
      total -= h;
      m = std::chrono::duration_cast<minutes>(total);
      s = (total -= m);
   }
   
   auto to_pretty() const {
      auto plural = [](auto v, std::string_view s, std::string_view p = "s") {
         return v == 1
            ? std::format("{} {}", v, s)
            : std::format("{} {}{}", v, s, p);
      };
      return std::format("{}, {}, {}, {}",
         plural(d.count(), "day"),
         plural(h.count(), "hour"),
         plural(m.count(), "minute"),
         plural(s.count(), "second")
      );
   }
   
   auto to_regular() const {
      return std::format("{} day{}, {:02}:{:02}:{:02}",
         d.count(), (d.count() == 1 ? "" : "s"),
         h.count(), m.count(), s.count()
      );
   }
   
   auto to_since() const {
      return std::format("{:%F %H:%M:%S}", std::chrono::zoned_time{
         std::chrono::current_zone(), std::chrono::floor<seconds>(
            std::chrono::system_clock::now() - sec
         )
      });
   }

private:
   seconds sec;
};

void print_help() {
   std::cout << R"(Usage: uptime [options]
Options:
   -p   show uptime in pretty format
   -h   display this help and exit
   -s   system up since
)";
}

bool test_equal(std::string_view a, std::string_view b) {
#if defined(_MSC_VER) && _MSC_VER < 2000 || !defined(__cpp_lib_ranges)
  return a.size() == b.size() &&
    std::equal(a.begin(), a.end(), b.begin(),
       [](unsigned char ac, unsigned char bc){
          return std::tolower(ac) == std::tolower(bc);
       }
    );
#else
   return a.size() == b.size() &&
      std::ranges::equal(a, b, {}, [](unsigned char c){ return std::tolower(c); });
#endif
}

int main(int argc, char** argv) {
   auto up = uptime(get_seconds());
   switch (argc) {
      case 1:
         std::cout << get_local_time() << " up " << up.to_regular() << "\n";
         break;
      
      case 2: {
         std::string_view arg = argv[1];
         if (test_equal(arg, "-p")) std::cout << "up " << up.to_pretty() << "\n";
         else if (test_equal(arg, "-s")) std::cout << up.to_since() << "\n";
         else if (test_equal(arg, "-h")) print_help();
         else std::cerr << R"([!] Unknown option has been specified.
Use -h option to get help.
)";
         break;
      }
      
      default:
         std::cerr << R"([!] Option index is out of range.
Specify -h to get available options.
)";
         break;
   }
}
