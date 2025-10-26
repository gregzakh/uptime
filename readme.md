### uptime

Building
 - Linux
```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j$(nproc)
```
 - macOS
```zsh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j$(sysctl -n hw.logicalcpu)
```
 - Windows
```cmd
cmake -S . -B build -G "Visual Studio 17 2022"
cmake --build build --config Release
```
