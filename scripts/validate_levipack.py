#!/usr/bin/env python3
"""Validate the structure and manifest of a generated LeviLauncher package."""

from __future__ import annotations

import json
import sys
import zipfile
from pathlib import Path


def fail(message: str) -> int:
    print(f"ERROR: {message}", file=sys.stderr)
    return 1


def main() -> int:
    if len(sys.argv) != 2:
        return fail("usage: validate_levipack.py <build-directory>")

    build_dir = Path(sys.argv[1])
    packages = sorted(build_dir.glob("*.levipack"))
    if len(packages) != 1:
        return fail(
            f"expected exactly one .levipack in {build_dir}, found {len(packages)}"
        )

    package = packages[0]
    required = {
        "horion_android/manifest.json",
        "horion_android/libhorion_android.so",
        "horion_android/config/config.json",
        "horion_android/config/config.schema.json",
    }

    try:
        with zipfile.ZipFile(package) as archive:
            corrupt = archive.testzip()
            if corrupt is not None:
                return fail(f"corrupt ZIP member: {corrupt}")

            names = set(archive.namelist())
            missing = sorted(required - names)
            if missing:
                return fail("missing package entries: " + ", ".join(missing))

            manifest = json.loads(archive.read("horion_android/manifest.json"))
            json.loads(archive.read("horion_android/config/config.json"))
            json.loads(archive.read("horion_android/config/config.schema.json"))

            if manifest.get("type") != "preload-native":
                return fail("manifest type must be 'preload-native'")
            if manifest.get("entry") != "libhorion_android.so":
                return fail("manifest entry does not match the packaged library")
            if not isinstance(manifest.get("minecraft_versions"), list):
                return fail("manifest minecraft_versions must be a JSON array")

            library_info = archive.getinfo("horion_android/libhorion_android.so")
            if library_info.file_size <= 0:
                return fail("native library is empty")

    except (OSError, zipfile.BadZipFile, KeyError, json.JSONDecodeError) as exc:
        return fail(f"could not validate {package}: {exc}")

    print(f"Validated {package}")
    print("  mod id: horion_android")
    print("  type: preload-native")
    print("  ABI: arm64-v8a (CI target)")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
