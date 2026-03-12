# OpenDRIVE Unreal Engine plugin - Release notes

## 2026-03

* Updated branch structure to match Unreal Engine releases
  * Starting with 5.5, this plugin has a branch for each engine release
  * The repository's default branch is the highest supported engine release

## 2026-02

* Fixed landscape artifacts when using OpenDRIVE sculpting

## 2026-01

* Improved Viewer to support junctions

## 2025-11

* Added Unreal Engine 5.7 support

## 2025-10

* Upgraded esmini/roadmanager from 2.36.4 to 2.52.1

## 2025-07

* Added [BP_OpenDriveMover](https://github.com/brifsttar/OpenDRIVE#mover)
* Added [BP_SpawnAlongS](https://github.com/brifsttar/OpenDRIVE#spawn-along-s)

## 2025-01

* New tags
  * `5.3`
  * `5.4`
* Added OpenDRIVE Gizmo ([PR #24](https://github.com/brifsttar/OpenDRIVE/pull/24))
  * [README description](https://github.com/brifsttar/OpenDRIVE?tab=readme-ov-file#opendrive-gizmo)
  * Includes migration of old Editor Mode to newer Scriptable Tools API
* Added `GetRealT` and `SetRealT`
  * Workaround to the fact that `GetT` and `SetT` actually work with offset (to lane center), and not actual T. Those methods are now deprecated in favor of `GetOffset` and `SetOffset`.
* Fixed `SetS` and `SetT` not refreshing the inertial coordinates

## Previously

Tags from `4.26` to `5.2`, with incremental features supported along the way (see the README for each tag for more details).
