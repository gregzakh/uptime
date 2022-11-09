#pragma once

#ifndef UNICODE
  #define UNICODE
#endif

#ifndef WIN32_LEAN_AND_MEAN
  #define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>
#include <algorithm>
#include <string>
#include <cstdio>
#include <string>
#include <vector>
#include <locale>

constexpr ULONG SystemTimeOfDayInformation = 3UL;

using CSHORT  = SHORT;
using NTSTATUS = LONG;

#define NT_SUCCESS(Status) (static_cast<NTSTATUS>(Status) >= 0L)
#define fnset(P) reinterpret_cast<P>(GetProcAddress(GetModuleHandle(L"ntdll.dll"), (&((#P)[1]))))

struct TIME_FIELDS {
   CSHORT Year;
   CSHORT Month;
   CSHORT Day;
   CSHORT Hour;
   CSHORT Minute;
   CSHORT Second;
   CSHORT Milliseconds;
   CSHORT Weekday;
};
using PTIME_FIELDS = TIME_FIELDS*;

using pNtQuerySystemInformation = NTSTATUS(__stdcall *)(ULONG, PVOID, ULONG, PULONG);
using pRtlNtStatusToDosError = ULONG(__stdcall *)(NTSTATUS);
using pRtlTimeToElapsedTimeFields = VOID(__stdcall *)(PLARGE_INTEGER, PTIME_FIELDS);

struct ntapi {
  pNtQuerySystemInformation NtQuerySystemInformation = fnset(pNtQuerySystemInformation);
  pRtlNtStatusToDosError RtlNtStatusToDosError = fnset(pRtlNtStatusToDosError);
  pRtlTimeToElapsedTimeFields RtlTimeToElapsedTimeFields = fnset(pRtlTimeToElapsedTimeFields);

  bool isvalid(void) {
    std::vector<PVOID> v{NtQuerySystemInformation, RtlNtStatusToDosError, RtlTimeToElapsedTimeFields};
    return std::all_of(v.begin(), v.end(), [](PVOID const x){ return nullptr != x; });
  }
};

struct SYSTEM_TIMEOFDAY_INFORMATION {
   LARGE_INTEGER BootTime;
   LARGE_INTEGER CurrentTime;
   LARGE_INTEGER TimeZoneBias;
   ULONG TimeZoneId;
   ULONG Reserved;
   ULONGLONG BootTimeBias;
   ULONGLONG SleepTimeBias;
private:
   auto stime(LONGLONG delta) {
     SYSTEMTIME st{};
     auto ft = *reinterpret_cast<PFILETIME>(&delta);
     ::FileTimeToSystemTime(&ft, &st);

     return st;
   }
public:
   void getsysuptime(ntapi const nt, const bool pretty) {
     auto ct = stime(this->CurrentTime.QuadPart - this->TimeZoneBias.QuadPart);
     auto up = this->CurrentTime.QuadPart - this->BootTime.QuadPart;

     TIME_FIELDS tf{};
     nt.RtlTimeToElapsedTimeFields(reinterpret_cast<PLARGE_INTEGER>(&up), &tf);
     if (pretty) {
       std::string str = "up ";
       auto append = [&str](const CSHORT value, const std::string part){
         if (!value) return str;
         str += std::to_string(value) + " " + part + (value > 1 ? "s" : "") + ", ";
         return str;
       };
       append(tf.Day,    "day");
       append(tf.Hour,   "hour");
       append(tf.Minute, "minute");
       append(tf.Second, "second");
       printf("%sboot id: %lu\n", str.c_str(), *reinterpret_cast<PULONG>(0x7FFE02C4));
       return;
     }
     printf("%.2hu:%.2hu:%.2hu up %hu.%.2hu:%.2hu:%.2hu\n",
       ct.wHour, ct.wMinute, ct.wSecond, tf.Day, tf.Hour, tf.Minute, tf.Second
     );
   }

   void getsysboot(void) {
     auto bt = stime(this->BootTime.QuadPart - this->TimeZoneBias.QuadPart - this->BootTimeBias);
     printf("%.4hu-%.2hu-%.2hu %.2hu:%.2hu:%.2hu\n",
       bt.wYear, bt.wMonth, bt.wDay, bt.wHour, bt.wMinute, bt.wSecond
     );
   }
};
