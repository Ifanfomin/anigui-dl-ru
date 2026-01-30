from pathlib import Path
import subprocess
import sys


# прежде чем использовать скрипт надо
# один раз вручную сделать окружение с requirements.txt

# после сборки трогать ничего не надо, 
# проект сам возьмёт бинарь из backend/dist/


BACKEND_DIR = Path(__file__).resolve().parent

BACKEND_PY = BACKEND_DIR / "backend.py"
DIST_DIR   = BACKEND_DIR / "dist"
BUILD_DIR  = BACKEND_DIR / "build"

cmd = [
    sys.executable, "-m", "PyInstaller",
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
