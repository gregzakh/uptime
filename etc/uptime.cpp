#include <windows.h>
#include <algorithm>
#include <map>
#include <cstdio>
#include <string>
#include <vector>
#include <numeric>

#define fnset(P) reinterpret_cast<P>(GetProcAddress(GetModuleHandle("ntdll.dll"), (&((#P)[1]))))

struct KSYSTEM_TIME {
   ULONG LowPart;
   LONG  High1Time;
   LONG  High2Time;

   auto asquad(void) {
     return (reinterpret_cast<PLARGE_INTEGER>(this))->QuadPart;
   }
};
using PKSYSTEM_TIME = KSYSTEM_TIME*;

using CSHORT = SHORT;
struct TIME_FIELDS {
   CSHORT Year;
   CSHORT Month;
   CSHORT Day;
   CSHORT Hour;
   CSHORT Minute;
   CSHORT Second;
   CSHORT Milliseconds;
   CSHORT Weekday;

   void getbootime(void) {
     printf("%.4hu-%.2hu-%.2hu %.2hu:%.2hu:%.2hu\n",
       this->Year, this->Month, this->Day, this->Hour, this->Minute, this->Second
     );
   }

   void getpretty(void) {
     std::map<std::string, CSHORT> vals {
       {"day",    this->Day},
       {"hour",   this->Hour},
       {"minute", this->Minute},
       {"second", this->Second}
     };
     std::string res = std::accumulate(vals.begin(), vals.end(), std::string(),
       [](const std::string& s, const std::pair<const std::string, CSHORT>& p){
         if (!p.second) return s;
         return s + std::to_string(p.second) + " " + p.first + (p.second > 1 ? "s" : "") + ", ";
       }
     );
     printf("up %sboot id: %lu\n", res.c_str(), *reinterpret_cast<PULONG>(0x7FFE02C4));
   }
};
using PTIME_FIELDS = TIME_FIELDS*;

using pRtlTimeToTimeFields = VOID(__stdcall *)(PLARGE_INTEGER, PTIME_FIELDS);
using pRtlTimeToElapsedTimeFields = VOID(__stdcall *)(PLARGE_INTEGER, PTIME_FIELDS);

struct ntapi {
  pRtlTimeToTimeFields RtlTimeToTimeFields = fnset(pRtlTimeToTimeFields);
  pRtlTimeToElapsedTimeFields RtlTimeToElapsedTimeFields = fnset(pRtlTimeToElapsedTimeFields);

  bool isvalid(void) {
    std::vector<PVOID> v{RtlTimeToTimeFields, RtlTimeToElapsedTimeFields};
    return std::all_of(v.begin(), v.end(), [](PVOID const x){ return nullptr != x; });
  }

  auto tfget(VOID (*fn)(PLARGE_INTEGER, PTIME_FIELDS), LONGLONG delta) {
    TIME_FIELDS tf{};
    fn(reinterpret_cast<PLARGE_INTEGER>(&delta), &tf);

    return tf;
  }
};

int main(int argc, char **argv) {
  if (3 <= argc) {
    printf("Option index is out of range.\nSpecify -h to get available options.\n");
    return 1;
  }

  ntapi ntdll{};
  if (!ntdll.isvalid()) {
    printf("Cannot find required signature.\n");
    return 1;
  }

  std::vector<LONGLONG> vals{0x7FFE0008, 0x7FFE0014, 0x7FFE0020};
  std::transform(vals.begin(), vals.end(), vals.begin(), [](LONGLONG const addr){
     return (*reinterpret_cast<PKSYSTEM_TIME>(addr)).asquad();
  });

  if (2 == argc) {
    if (0 == _stricmp(argv[1], "-p")) {
      ntdll.tfget(ntdll.RtlTimeToElapsedTimeFields, vals[0]).getpretty();
      return 0;
    }

    if (0 == _stricmp(argv[1], "-h")) {
      printf("Usage: uptime [option]\n\nOptions:\n");
      printf("  -p - show uptime in pretty format\n");
      printf("  -h - display this help and exit\n");
      printf("  -s - system up since\n");

      return 0;
    }

    if (0 == _stricmp(argv[1], "-s")) {
      ntdll.tfget(ntdll.RtlTimeToTimeFields, vals[1] - vals[2] - vals[0]).getbootime();
      return 0;
    }

    printf("Unknown option has been specified.\n");
  }
  else {
    auto st = ntdll.tfget(ntdll.RtlTimeToTimeFields, vals[1] - vals[2]);
    auto up = ntdll.tfget(ntdll.RtlTimeToElapsedTimeFields, vals[0]);
    printf("%.2hu:%.2hu:%.2hu up %hu.%.2hu:%.2hu:%.2hu\n",
      st.Hour, st.Minute, st.Second, up.Day, up.Hour, up.Minute, up.Second
    );
  }

  return 0;
}
