from pathlib import Path
import subprocess
import platform

BACKEND_DIR = Path(__file__).resolve().parent

if platform.system() == "Windows":
    PYTHON = BACKEND_DIR / ".venv" / "Scripts" / "python.exe"
else:
    PYTHON = BACKEND_DIR / ".venv" / "bin" / "python"

BACKEND_PY = BACKEND_DIR / "backend.py"

DIST_DIR = BACKEND_DIR / "dist"
BUILD_DIR = BACKEND_DIR / "build"

cmd = [
    str(PYTHON),
    "-m",
    "PyInstaller",

    "--noconfirm",
    "--clean",
    "--onedir",

    "--hidden-import=socks",

    "--distpath", str(DIST_DIR),
    "--workpath", str(BUILD_DIR),
    "--specpath", str(BACKEND_DIR),

    BACKEND_PY.name,
]

print("Running:", " ".join(cmd))

subprocess.check_call(
    cmd,
    cwd=BACKEND_DIR
)
