#include "uptime.hpp"

int wmain(int argc, wchar_t **argv) {
  ntapi ntdll{};
  if (!ntdll.isvalid()) {
    printf("Cannot find required signature.\n");
    return 1;
  }

  auto getlasterror = [&ntdll](NTSTATUS nts) {
    HLOCAL loc{};
    auto size = ::FormatMessage(
      FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER,
      nullptr, ntdll.RtlNtStatusToDosError(nts),
      MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
      reinterpret_cast<LPWSTR>(&loc), 0, nullptr
    );

    if (size)
      printf("[!] %.*ws\n", static_cast<INT>(size - sizeof(WCHAR)), reinterpret_cast<LPWSTR>(loc));
    else
      printf("[?] Unknown error has been occured.\n");

    if (nullptr != ::LocalFree(loc))
      printf("[!] LocalFree (%lu) fatal error.\n", ::GetLastError());
  };

  static const struct {
    const wchar_t *param;
    const wchar_t *desc;
  } params[] = {
    L"-p", L"show uptime in pretty format",
    L"-h", L"display this help and exit",
    L"-s", L"system up since"
  };
  SYSTEM_TIMEOFDAY_INFORMATION sti{};
  auto nts = ntdll.NtQuerySystemInformation(SystemTimeOfDayInformation, &sti, sizeof(sti), nullptr);
  if (!NT_SUCCESS(nts)) {
    getlasterror(nts);
    return 1;
  }

  if (3 <= argc) {
    printf("[*] Index is out of range.\nSpecify -h to get available options.\n");
    return 1;
  }
  else if (2 == argc) {
    if (0 == _wcsicmp(argv[1], L"-p")) {
      sti.getsysuptime(ntdll, true);
      return 0;
    }

    if (0 == _wcsicmp(argv[1], L"-s")) {
      sti.getsysboot();
      return 0;
    }

    if (0 == _wcsicmp(argv[1], L"-h")) {
      printf("Usage: uptime [option]\n\nOptions:\n");
      for (const auto& x : params)
        printf("%5ws - %ws\n", x.param, x.desc);
      return 0;
    }

    printf("[*] Unknown option has been specified.\n");
  }
  else sti.getsysuptime(ntdll, false);

  return 0;
}
