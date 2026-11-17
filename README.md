# Horion Android — LeviLauncher Native Package Scaffold

This branch adapts the existing LeviLauncher Android native-mod template into an ARM64
LeviPack project labelled **Horion Android**.

## Important scope

This is **not a conversion of HorionContinued/Injector**. That repository is a
Windows DLL injector; it does not contain the Horion client source that would be
needed for a native Android port. Renaming a Windows DLL or injector to .so does
not make it compatible with Android.

This branch is a safe native-mod scaffold demonstrating the supported LeviLauncher
Preloader lifecycle, typed configuration, Android ARM64 compilation, and LeviPack
packaging. It only writes diagnostic messages to the mod logger. It does **not**
implement process injection, game-memory hooks, patches, signatures, or gameplay
modifications.

## Package details

- Mod ID: horion_android
- Display name: Horion Android Native Scaffold
- ABI: arm64-v8a
- Package: horion_android-0.1.0-arm64-v8a.levipack
- Preloader Android SDK: 0.2.2 (pinned by CMake)
- Android NDK: 28.2.13676358
- Minecraft version declaration: empty until a target version is tested

The package contains:

~~~text
horion_android/
├── manifest.json
├── libhorion_android.so
└── config/
    ├── config.json
    └── config.schema.json
~~~

## Build locally

Install Android SDK/NDK, CMake 3.22+, Ninja, and PowerShell 7+. Set
ANDROID_HOME to your Android SDK directory, then run from the repository root:

~~~powershell
$env:ANDROID_HOME = "C:/Users/<you>/AppData/Local/Android/Sdk"
./scripts/package.ps1 -Abi arm64-v8a
~~~

The output is written to build-arm64-v8a/.

## GitHub Actions

The workflow in .github/workflows/build.yml builds the ARM64 package on pushes to
horion-android-levipack, pull requests, and manual runs. It validates the ZIP
contents and uploads the .levipack as an artifact. Version tags beginning with
v also attach the package to a GitHub Release.

## Install in LeviLauncher

1. Download the .levipack artifact from the Actions run.
2. Copy it to the Android device.
3. In LeviLauncher, open the target instance's Mods page and choose Add Mod.
4. Import the package, then launch a test instance and inspect Android log output.

Use a separate test instance and back up worlds before testing native mods.

## Next steps

This branch establishes the build and package pipeline only. A genuine Horion
feature port would require Android-compatible Horion client source, a review of
the source licence and dependencies, and separate implementation work. The
Windows injector repository alone is not enough to produce that port.
