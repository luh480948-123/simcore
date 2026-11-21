# simcore 个人贡献证明

## 贡献者信息

- GitHub 账号：[`wwpily`](https://github.com/wwpily)
- 组项目仓库：[`luh480948-123/simcore`](https://github.com/luh480948-123/simcore)
- 个人 Fork：[`wwpily/simcore`](https://github.com/wwpily/simcore)
- 功能分支：`feature/entity-handle-hash`
- Pull Request：[PR #2 — Add entity handle equality and hash support](https://github.com/luh480948-123/simcore/pull/2)
- 在线贡献证明：[PR #2 贡献证明评论](https://github.com/luh480948-123/simcore/pull/2#issuecomment-7063373921)
- 目标分支：`main`
- 记录日期：2026-11-21

## 负责功能

负责“句柄可用性（Entity Handle Usability）”，在不改变原有 Registry、Stepper、EventBus、实体生命周期和更新顺序的前提下，为 `EntityHandle<T>` 增加：

1. 基于所属 Registry 和 EntityId 的相等比较。
2. `std::hash<EntityHandle<T>>` 支持，使句柄可以作为 `std::unordered_set` 和 `std::unordered_map` 的键。
3. 实体删除后保持稳定身份和哈希，并能区分槽位复用后的新实体。
4. 覆盖空句柄、不同 Registry、失效句柄、槽位复用及哈希容器行为的测试。

## Commit 记录

| Commit | 说明 | 作者 | 在线记录 |
|---|---|---|---|
| `36356ce43cd50a87209625394884e0373dad46b4` | `feat: make entity handles usable as hash keys` | `wwpily` | [查看功能 commit](https://github.com/wwpily/simcore/commit/36356ce43cd50a87209625394884e0373dad46b4) |
| `ed62ba5adb1cb398b6c8060f9d8c770dd8f63800` | `test: verify stable entity handle identity` | `wwpily` | [查看测试 commit](https://github.com/wwpily/simcore/commit/ed62ba5adb1cb398b6c8060f9d8c770dd8f63800) |
| 本证明文档所在 commit | `docs: record entity handle contribution` | `wwpily` | [在 PR Commits 页面查看](https://github.com/luh480948-123/simcore/pull/2/commits) |

功能代码、测试和本证明文档分别通过上述 commit 提交到 `wwpily/simcore` 的 `feature/entity-handle-hash` 分支，并包含在组仓库 PR #2 中。

## 功能代码修改

以下统计不包含本证明文档：

| 文件 | 新增行 | 删除行 | 内容 |
|---|---:|---:|---|
| `include/simcore/entity_handle.h` | 24 | 0 | 句柄相等比较和 `std::hash` 特化 |
| `tests/registry_test.cpp` | 68 | 0 | 4 项句柄身份与哈希容器测试 |
| **合计** | **92** | **0** | **2 个功能文件** |

## 测试证明

- 构建环境：CMake、Ninja、GCC 16.2.0、Release 配置。
- 完整 CTest 测试套件：**19/19 通过**。
- 其中包含本次新增的 4 项测试：
  - `EntityHandleTest.EqualityTracksEntityIdentity`
  - `EntityHandleTest.EqualityIncludesOwningRegistry`
  - `EntityHandleTest.HashRemainsStableAfterDespawn`
  - `EntityHandleTest.WorksAsUnorderedMapKey`
- 提交前已检查完整 diff，`git diff --check origin/main...HEAD` 无错误。

测试命令：

```powershell
cmake --build <build-dir> --config Release
ctest --test-dir <build-dir> -C Release --output-on-failure
```

## 在线核验方法

1. 打开 [PR #2](https://github.com/luh480948-123/simcore/pull/2)。
2. 在 **Commits** 页面确认功能、测试和文档 commit 及作者 `wwpily`。
3. 在 **Files changed** 页面查看功能代码、测试和本证明文档。
4. 在 PR 描述中查看功能摘要和测试记录。
5. 合并后，还可在组仓库的 commit 历史和 Contributors 页面核验贡献。

## 说明

本证明作为项目贡献文档提交，不修改库源码或测试行为。最原始的贡献证据以 GitHub 上的 commit 作者、commit 哈希、PR #2 和 Files changed 记录为准。
