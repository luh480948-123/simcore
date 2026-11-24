# Entity Handle Contribution

## Contributor

- GitHub: [`wwpily`](https://github.com/wwpily)
- Repository: [`luh480948-123/simcore`](https://github.com/luh480948-123/simcore)
- Implementation branch: `feature/entity-handle-hash`
- Pull request: [#2 — Add entity handle equality and hash support](https://github.com/luh480948-123/simcore/pull/2)
- Merge commit: [`0850705`](https://github.com/luh480948-123/simcore/commit/085070518cfc4896818efd03245dc32d8d42818e)
- Date: 2026-11-21

## Scope

This contribution extends `EntityHandle<T>` without changing registry storage, entity lifetime, event dispatch, or update ordering.

The implementation adds:

1. Equality comparison based on the owning registry and entity ID.
2. A `std::hash<EntityHandle<T>>` specialization for use with `std::unordered_set` and `std::unordered_map`.
3. Stable handle identity and hashing after despawn, including distinction from an entity that later reuses the same slot.
4. Tests covering empty handles, separate registries, stale handles, slot reuse, and unordered containers.

## Commits

| Commit | Description |
|---|---|
| [`36356ce`](https://github.com/luh480948-123/simcore/commit/36356ce43cd50a87209625394884e0373dad46b4) | `feat: make entity handles usable as hash keys` |
| [`ed62ba5`](https://github.com/luh480948-123/simcore/commit/ed62ba5adb1cb398b6c8060f9d8c770dd8f63800) | `test: verify stable entity handle identity` |
| [`ff2f214`](https://github.com/luh480948-123/simcore/commit/ff2f214a91d5b01fa4b0bf24ad9c9e5d8d1f7f36) | `docs: record entity handle contribution` |

## Implementation Files

| File | Added lines | Change |
|---|---:|---|
| `include/simcore/entity_handle.h` | 24 | Handle equality and `std::hash` support |
| `tests/registry_test.cpp` | 68 | Four identity and hash-container tests |

## Verification

The merged `main` branch was built in Release mode with CMake, Ninja, and GCC 16.2.0. All **19 of 19 tests passed**, including:

- `EntityHandleTest.EqualityTracksEntityIdentity`
- `EntityHandleTest.EqualityIncludesOwningRegistry`
- `EntityHandleTest.HashRemainsStableAfterDespawn`
- `EntityHandleTest.WorksAsUnorderedMapKey`

To repeat the checks:

```powershell
cmake --build <build-dir> --config Release
ctest --test-dir <build-dir> -C Release --output-on-failure
```

The pull request history and the commits listed above provide the authoritative record of these changes.
