# FenSu Cloud Album — D 版（第一版）

跨平台云相册客户端。**Android 优先**，Core 层保持纯 C++、完全跨平台。

技术栈：**C++20 · Qt 6 · Qt Quick / QML · CMake · SQLite · Qt Network**

---

## 目录

- [这是什么](#这是什么)
- [当前进度](#当前进度)
- [架构](#架构)
- [目录结构](#目录结构)
- [核心设计约定](#核心设计约定)
- [本地构建](#本地构建)
- [CI / 构建产物](#ci--构建产物)
- [路线图](#路线图)

---

## 这是什么

一个**以 WebDAV 为云端存储后端**的云相册 + 本地相册应用，目标是长期可用、可增量同步、可传输超大文件，并且拥有接近 Google Photos 的 UI。

D 版（第一版）的边界非常明确：

| 做 | 不做 |
|----|------|
| 本地媒体库（SQLite） | 地图 / 地点功能（**整体排除**） |
| 相册（WebDAV 文件夹一一对应） | AI 回忆生成 |
| 全部照片 / 回忆 三个主 Tab | 社交 / 分享到第三方 |
| WebDAV 增量同步（后续阶段接入） | 人脸识别 |
| 超大文件流式传输（后续阶段接入） | 视频编辑 |

> **关键业务规则**：WebDAV 目录结构与相册**一一对应，绝不扁平化**。任何一层代码都不允许把远端文件夹路径压平成文件名。

---

## 当前进度

**第一阶段**（可编译骨架 + 基础 UI + SQLite 初始化）已完成代码落盘。

| 模块 | 状态 | 说明 |
|------|------|------|
| CMake 工程 | ✅ | `CMakeLists.txt`，Qt 6.5+，Android/桌面双目标 |
| 应用层 | ✅ | `AppController`（QML 以 `App` 引用） |
| 数据库层 | ✅ | `DatabaseSchema` / `DatabaseManager`，6 张表，WAL，64 位尺寸 |
| 模型层 | ✅ | `MediaModel` / `AlbumModel` / `TimelineModel` / `MemoryModel` |
| 缓存层 | ✅ | `CacheManager`（thumbnails / tmp / logs） |
| 网络状态 | ✅ | `NetworkMonitor` |
| 平台层 | ✅ | `AndroidPlatform`（沙箱路径、可用空间、设备描述） |
| UI（QML） | ✅ | `Main` + 3 个页面 + 5 个组件 + `Theme` 单例 |
| Android 清单 | ✅ | `android/AndroidManifest.xml` |
| CI | ✅ | GitHub Actions：构建 APK + 仓库一致性校验 |

> ⚠️ 本机（开发设备）**没有 Qt 6 与 CMake**，因此编译验证完全依赖 GitHub Actions。

---

## 架构

四层，依赖方向严格单向（上层依赖下层，下层绝不反向依赖）：

```
┌───────────────────────────────────────────────┐
│  UI 层        Qt Quick / QML                  │
│               Main.qml, pages/, components/   │
│               只做展示，不含业务规则            │
├───────────────────────────────────────────────┤
│  Application  AppController                   │
│               协调 Core，翻译成 QML 友好的接口  │
├───────────────────────────────────────────────┤
│  Core         database / model / cache /      │
│               network / sync                  │
│               纯 C++，零平台依赖，可跨平台编译   │
├───────────────────────────────────────────────┤
│  Platform     AndroidPlatform 等              │
│               唯一允许出现平台 API 的地方        │
└───────────────────────────────────────────────┘
```

QML 通过 `App`（`AppController` 实例，作为 context property 注入）访问数据：

```qml
App.ready            // 核心初始化是否完成
App.albums           // AlbumModel
App.mediaModel       // MediaModel（按 display_date_taken DESC 排序）
App.timelineModel    // TimelineModel
App.memoryModel      // MemoryModel
App.lastError        // 统一错误出口
App.refreshAlbums()  // 重新加载
```

---

## 目录结构

```
FenSuCloudAlbum_D/
├── CMakeLists.txt
├── .gitignore
├── README.md
├── android/
│   └── AndroidManifest.xml          # QT_ANDROID_PACKAGE_SOURCE_DIR 指向此处
├── .github/
│   └── workflows/
│       └── android-build.yml        # CI：构建 APK + 一致性校验
└── src/
    ├── main.cpp
    ├── app/
    │   └── AppController.{h,cpp}    # 应用层
    ├── core/
    │   ├── database/
    │   │   ├── DatabaseSchema.{h,cpp}
    │   │   └── DatabaseManager.{h,cpp}
    │   ├── model/
    │   │   ├── MediaItem.{h,cpp}
    │   │   ├── AlbumItem.h
    │   │   ├── MediaModel.{h,cpp}
    │   │   ├── AlbumModel.{h,cpp}
    │   │   ├── TimelineModel.{h,cpp}
    │   │   └── MemoryModel.{h,cpp}
    │   ├── cache/
    │   │   └── CacheManager.{h,cpp}
    │   └── network/
    │       └── NetworkMonitor.{h,cpp}
    ├── platform/
    │   └── android/
    │       └── AndroidPlatform.{h,cpp}
    └── ui/
        └── qml/
            ├── Main.qml
            ├── theme/
            │   └── Theme.qml        # pragma Singleton
            ├── components/
            │   ├── BottomNavBar.qml
            │   ├── PhotoGrid.qml
            │   ├── AlbumCard.qml
            │   └── SectionHeader.qml
            └── pages/
                ├── AlbumsPage.qml
                ├── AllPhotosPage.qml
                └── MemoriesPage.qml
```

---

## 核心设计约定

这些约定是硬约束，改动前请先确认不会破坏它们。

### 1. 尺寸一律 64 位

照片视频**不设大小上限**，必须支持单张 **5 GB+**。

- C++：`qint64`
- SQLite：`INTEGER`
- QML：`number`（安全区间内）或由 C++ 提供格式化字符串

```cpp
// ✅ 正确
qint64 fileSize = 0;

// ❌ 绝对禁止
int fileSize = 0;   // 4GB 就溢出了
```

### 2. 禁止 `readAll()`

大文件必须**流式 + 分块**处理，配合 HTTP `Range` 请求与断点续传。

```cpp
// ✅ 正确
cacheManager->copyRange(src, dst, offset, length, /*chunkSize=*/2 * 1024 * 1024);

// ❌ 绝对禁止
QByteArray data = file.readAll();   // 5GB 文件直接 OOM
```

### 3. 双时间字段

媒体有两个时间，**不要混用**：

| 字段 | 含义 | 用途 |
|------|------|------|
| `originalDateTaken` | 拍摄时间（EXIF） | 保留原始信息，同步不丢失 |
| `displayDateTaken` | 展示时间 | 时间轴排序、分组用这个 |

### 4. 数据库取值按列名，不按序号

```cpp
// ✅ 正确
QString name = query.value(QStringLiteral("name")).toString();

// ❌ 禁止
QString name = query.value(3).toString();   // 加一列就全崩
```

### 5. 非破坏性编辑

任何编辑（旋转、裁剪、滤镜）都**不覆盖原文件**，只记录编辑指令。

### 6. 地点字段只预留，不实现

`schema` 中保留 `location*` 四个字段，但 **D 版不显示、不请求权限、不引入任何地图依赖**。这是为了 E 版扩展时不需要迁移数据库。

---

## 本地构建

### 前置要求

- Qt **6.5+**（CI 使用 6.7.2）
- CMake **3.21+**
- C++20 编译器（GCC 11+ / Clang 14+ / MSVC 19.30+）
- 可选：Ninja

### 桌面（开发调试用）

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build --parallel
./build/FenSuCloudAlbum
```

### Android

```bash
cmake -S . -B build-android \
  -G Ninja \
  -DCMAKE_BUILD_TYPE=Debug \
  -DCMAKE_PREFIX_PATH="$QT_ROOT_DIR" \
  -DCMAKE_TOOLCHAIN_FILE="$ANDROID_NDK_HOME/build/cmake/android.toolchain.cmake" \
  -DANDROID_ABI=arm64-v8a \
  -DANDROID_PLATFORM=android-34

cmake --build build-android --parallel
# APK 位于 build-android/android-build/build/outputs/apk/
```

需要的环境变量：

| 变量 | 说明 |
|------|------|
| `QT_ROOT_DIR` | Qt for Android 安装根目录 |
| `ANDROID_NDK_HOME` | NDK 路径 |
| `ANDROID_SDK_ROOT` | Android SDK 路径 |

---

## CI / 构建产物

每次推送到 `main` / `master` 或提 PR，GitHub Actions 自动执行 `.github/workflows/android-build.yml`：

**Job 1 — Build Android APK**

1. 安装 JDK 17、Ninja、Qt 6.7.2 (android_arm64_v8a)
2. 安装 Android SDK / NDK 26.1
3. CMake 配置 + 编译
4. 产出 `FenSuCloudAlbum-debug.apk` 并作为 artifact 上传

**Job 2 — Repository sanity check**

不依赖 Qt 工具链，快速校验仓库自洽性：

- `CMakeLists.txt` 中引用的每一个源文件是否真实存在
- 列出所有 QML 文件

> Job 2 的存在是为了让「CMake 引用了一个不存在的文件」这类错误在 30 秒内暴露，而不是等 Job 1 跑完 20 分钟。

---

## 路线图

D 版计划十个阶段，每阶段结束时必须**可编译**。

| 阶段 | 内容 | 状态 |
|------|------|------|
| 1 | 工程骨架 + 基础 UI + SQLite 初始化 | ✅ 本阶段 |
| 2 | 本地媒体扫描（MediaStore / 文件系统） | ⬜ |
| 3 | 缩略图管线（异步、可取消、磁盘缓存） | ⬜ |
| 4 | WebDAV 客户端（PROPFIND / GET / PUT） | ⬜ |
| 5 | 增量同步引擎（双向、断点续传、冲突处理） | ⬜ |
| 6 | 超大文件传输（分块、Range、校验） | ⬜ |
| 7 | 相册详情页 + 层级导航（保持目录结构） | ⬜ |
| 8 | 图片查看器（手势、渐进加载） | ⬜ |
| 9 | 视频播放器 | ⬜ |
| 10 | 设置页、账号管理、打磨 | ⬜ |

E 版才会考虑：地图 / 地点、AI 回忆、人脸分组、共享相册。

---

## 许可

私有项目，保留所有权利。
