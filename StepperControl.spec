# -*- mode: python ; coding: utf-8 -*-
"""
PyInstaller one-dir spec for DRV8825 Stepper GUI.
"""

import os, pathlib, ctypes.util
block_cipher = None

# ---------------------------------------------------------------------
# Directories
# ---------------------------------------------------------------------
app_root = pathlib.Path(os.getcwd()).resolve()     # repo root
gui_dir  = app_root / "host" / "gui"
res_dir  = gui_dir / "resources"

icon_path = res_dir / "stepper_logo.ico"           # <— single source of truth

# ---------------------------------------------------------------------
# VC runtime DLLs (optional)
# ---------------------------------------------------------------------
vc_dlls = []
for name in ("vcruntime140", "msvcp140"):
    p = ctypes.util.find_library(name)
    if p:
        vc_dlls.append((p, "."))

# ---------------------------------------------------------------------
# ANALYSIS
# ---------------------------------------------------------------------
a = Analysis(
    [str(gui_dir / "stepper_gui.py")],
    pathex=[str(gui_dir)],
    binaries=vc_dlls,
    datas=[
        (str(icon_path),                      "resources"),   # the .ico
        (str(res_dir / "stepper_logo.png"),   "resources"),   # the .png
    ],
    hiddenimports=["serial.tools.list_ports"],
    hookspath=[],
    runtime_hooks=[],
    excludes=[],
    win_no_prefer_redirects=False,
    win_private_assemblies=False,
    cipher=block_cipher,
)

pyz = PYZ(a.pure, a.zipped_data, cipher=block_cipher)

# ---------------------------------------------------------------------
# EXE
# ---------------------------------------------------------------------
exe = EXE(
    pyz,
    a.scripts,
    [],
    name="StepperControl",
    icon=str(icon_path),          # absolute path, exists at build-time
    console=False,
    exclude_binaries=True,
    debug=False,
    upx=True,
    strip=False,
)

# ---------------------------------------------------------------------
# COLLECT
# ---------------------------------------------------------------------
coll = COLLECT(
    exe,
    a.binaries,
    a.zipfiles,
    a.datas,
    strip=False,
    upx=True,
    name="StepperControl",
)
