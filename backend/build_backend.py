from pathlib import Path
import subprocess
import platform
import hashlib
import sys

BACKEND_DIR = Path(__file__).resolve().parent

if platform.system() == "Windows":
    PYTHON = BACKEND_DIR / ".venv" / "Scripts" / "python.exe"
else:
    PYTHON = BACKEND_DIR / ".venv" / "bin" / "python"

BACKEND_PY = BACKEND_DIR / "backend.py"

DIST_DIR = BACKEND_DIR / "dist"
BUILD_DIR = BACKEND_DIR / "build"

HASH_FILE = BUILD_DIR / ".backend.hash"


def calculate_hash():
    """Вычисляет общий хэш всех .py файлов и requirements.txt."""
    hasher = hashlib.sha256()

    # Все Python-файлы проекта
    for file in sorted(BACKEND_DIR.rglob("*.py")):
        # Не включаем служебные каталоги
        if ".venv" in file.parts or "build" in file.parts or "dist" in file.parts:
            continue

        hasher.update(file.relative_to(BACKEND_DIR).as_posix().encode())
        hasher.update(file.read_bytes())

    # requirements.txt тоже влияет на сборку
    requirements = BACKEND_DIR / "requirements.txt"
    if requirements.exists():
        hasher.update(requirements.read_bytes())

    return hasher.hexdigest()


def load_previous_hash():
    if HASH_FILE.exists():
        return HASH_FILE.read_text().strip()
    return None


def save_hash(hash_value):
    BUILD_DIR.mkdir(parents=True, exist_ok=True)
    HASH_FILE.write_text(hash_value)


current_hash = calculate_hash()
previous_hash = load_previous_hash()

if current_hash == previous_hash:
    print("Backend is up to date.")
    sys.exit(0)

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

save_hash(current_hash)

print("Backend successfully rebuilt.")