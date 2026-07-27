# FontTool

FontTool 是一个可独立构建、可嵌入现有工程的 C++17 字体检查与子集化库。项目从原 `Compress` 工程的字体模块中抽离，不依赖 PDF 文档对象、XML 或 Fontconfig；FreeType 是唯一必须由使用者提供的外部依赖。

项目同时提供：

- `FontTool::FontTool` CMake 库目标；
- 可选的 `fonttool` 命令行工具；
- 静态库和共享库构建；
- `add_subdirectory`、安装后 `find_package`、pkg-config 三种集成方式；
- 文件与内存加载、TTC/OTC 字体面枚举；
- Unicode、UTF-8、GID、CID 四种子集输入；
- TrueType 与 Type1/CFF 子集化；
- 完整错误码、GID/CID 映射结果、测试和示例。

## 1. 支持范围

| 能力 | 状态 |
|---|---|
| TrueType `glyf/loca` 检查与子集化 | 支持 |
| OpenType/CFF、Type1 CFF、CID-keyed CFF 子集化 | 支持 |
| Type1 PFB/PFA 输入 | 支持读取，并输出 CID-keyed CFF 子集程序 |
| TTC/OTC 字体集合枚举与指定 face | 支持 |
| UTF-8 / Unicode / GID / CID 子集输入 | 支持 |
| 保留原始 GID、保留 glyph count | TrueType 支持 |
| 保留原始 cmap | TrueType 支持，必须同时保留 GID 和 glyph count |
| CFF2 子集化 | 暂不支持；可以识别格式 |
| WOFF/WOFF2 输出 | 不支持 |
| 可变字体实例化 | 不支持 |
| OpenType Layout 重写（GSUB/GPOS） | 不支持 |

> CFF 与 Type1 后端输出的是原始 CFF 字体程序，而不是重新封装后的 OTF 文件。建议使用 `.cff` 扩展名，或直接把返回字节嵌入 PDF 等上层格式。

## 2. 依赖

- CMake 3.20+
- 支持 C++17 的编译器
- FreeType 2
- Ninja 可选，但推荐

AFDKO 子集化源码、utf8proc、utf8cpp 和 `tcb::span` 已按各自许可证随项目源码提供，不需要额外安装。

Ubuntu/Debian：

```bash
sudo apt-get install cmake ninja-build g++ libfreetype6-dev
```

macOS：

```bash
brew install cmake ninja freetype
```

Windows 推荐使用 vcpkg：

```powershell
vcpkg install freetype:x64-windows
```

## 3. 独立构建

### Release 静态库

```bash
cmake -S . -B build -G Ninja \
  -DCMAKE_BUILD_TYPE=Release \
  -DFONTTOOL_BUILD_CLI=ON \
  -DFONTTOOL_BUILD_EXAMPLES=ON \
  -DFONTTOOL_BUILD_TESTS=ON
cmake --build build --parallel
ctest --test-dir build --output-on-failure
```

也可以使用预设：

```bash
cmake --preset release
cmake --build --preset release
ctest --preset release
```

### Release 共享库

```bash
cmake --preset shared-release
cmake --build --preset shared-release
ctest --preset shared-release
```

### Windows + vcpkg

```powershell
cmake -S . -B build -A x64 `
  -DCMAKE_TOOLCHAIN_FILE=$env:VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake `
  -DFONTTOOL_BUILD_CLI=ON
cmake --build build --config Release --parallel
ctest --test-dir build -C Release --output-on-failure
```

## 4. 作为子目录集成

```cmake
add_subdirectory(external/FontTool)

target_link_libraries(your_target
    PRIVATE
        FontTool::FontTool)
```

作为父工程子目录时，CLI、示例、测试和安装规则默认关闭。需要时在 `add_subdirectory` 前设置：

```cmake
set(FONTTOOL_BUILD_CLI ON CACHE BOOL "" FORCE)
set(FONTTOOL_BUILD_TESTS ON CACHE BOOL "" FORCE)
add_subdirectory(external/FontTool)
```

## 5. 安装后使用 `find_package`

安装：

```bash
cmake --install build --prefix /opt/fonttool
```

消费方 `CMakeLists.txt`：

```cmake
cmake_minimum_required(VERSION 3.20)
project(FontToolConsumer LANGUAGES CXX)

find_package(FontTool 1 CONFIG REQUIRED)

add_executable(app main.cpp)
target_link_libraries(app PRIVATE FontTool::FontTool)
```

配置消费方：

```bash
cmake -S . -B build -DCMAKE_PREFIX_PATH=/opt/fonttool
```

## 6. 最小 C++ 示例

```cpp
#include <fonttool/fonttool.hpp>

#include <iostream>

int main() {
    fonttool::FontTool tool;
    auto status = tool.load_file("input.ttf");
    if (!status) {
        std::cerr << status.message << '\n';
        return 1;
    }

    fonttool::SubsetOptions options;
    options.include_hinting = true;

    fonttool::SubsetResult result;
    status = tool.subset_utf8("Hello, FontTool!", options, result);
    if (!status) {
        std::cerr << status.message << '\n';
        return 1;
    }

    status = fonttool::FontTool::write_file("subset.ttf", result);
    if (!status) {
        std::cerr << status.message << '\n';
        return 1;
    }
}
```

## 7. 从内存加载

`load_memory()` 会复制需要的字体数据，函数返回后调用方可以释放原缓冲区：

```cpp
std::vector<std::uint8_t> font_bytes = read_font();

fonttool::FontTool tool;
auto status = tool.load_memory(font_bytes, 0);
font_bytes.clear();
```

## 8. TTC/OTC 字体集合

```cpp
std::vector<fonttool::FaceInfo> faces;
auto status = fonttool::FontTool::inspect_file("collection.ttc", faces);
if (!status)
    return;

for (const auto& face : faces) {
    std::cout << face.index << ": "
              << face.family_name << " "
              << face.style_name << '\n';
}

fonttool::FontTool tool;
tool.load_file("collection.ttc", faces[2].index);
```

## 9. 子集选项

```cpp
fonttool::SubsetOptions options;
options.retain_gids = true;
options.retain_glyph_count = true;
options.include_cmap = true;
options.include_hinting = false;
```

约束：

- `retain_glyph_count=true` 必须同时设置 `retain_gids=true`；
- `include_cmap=true` 必须同时设置 `retain_gids=true` 和 `retain_glyph_count=true`；
- 不保留 GID 时，必须使用 `SubsetResult::glyph_mappings` 或 `cid_mappings` 获取新 GID；
- `.notdef`（GID 0）会自动保留；
- 复合 TrueType 字形引用到的子字形会自动加入子集。

## 10. 命令行工具

```bash
fonttool info input.ttf
fonttool info collection.ttc --face 2 --json
fonttool faces collection.ttc --json
fonttool subset input.ttf -o subset.ttf --text "Hello 世界"
fonttool subset input.otf -o subset.cff --text-file characters.txt
fonttool subset input.ttf -o subset.ttf --gids 1,2,36,42 --retain-gids
```

完整帮助：

```bash
fonttool --help
```

## 11. 线程模型与错误处理

公共 API 不向调用方抛出 C++ 异常，所有可恢复错误通过 `fonttool::Status` 返回。内存耗尽等无法恢复的运行时故障仍可能终止进程。

不同 `FontTool` 实例可以并行使用；同一个实例包含可变加载状态，不应在没有外部同步的情况下由多个线程同时访问。

## 12. 目录结构

```text
FontTool/
├── include/fonttool/       # 稳定公共 API
├── src/fonttool.cpp        # 公共 API 适配层
├── src/cli/                # 可选 CLI
├── src/internal/podofo/    # 字体专用内部实现，不对外安装
├── third_party/            # 随源码构建的第三方组件
├── examples/               # 集成示例
├── tests/                  # 回归测试
├── cmake/                  # find_package / pkg-config 模板
├── docs/                   # API、集成与架构说明
└── LICENSES/               # 完整许可证文本
```

## 13. 许可证

新编写的公共封装、CLI、示例和构建文件采用 MPL-2.0。PoDoFo 派生文件保留 `LGPL-2.0-or-later OR MPL-2.0` 双许可证；AFDKO、utf8proc、utf8cpp、tcb::span 分别保留其上游许可证。从原 Compress 集成代码迁移时，请参阅 `docs/MIGRATION_FROM_COMPRESS.md`。

详见 `LICENSE`、`NOTICE`、`THIRD_PARTY_NOTICES.md` 和 `LICENSES/`。
