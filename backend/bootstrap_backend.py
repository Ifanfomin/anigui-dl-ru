from pathlib import Path
import subprocess
import shutil
import venv
import platform

BACKEND_DIR = Path(__file__).resolve().parent
VENV_DIR = BACKEND_DIR / ".venv"

if platform.system() == "Windows":
    PYTHON = VENV_DIR / "Scripts" / "python.exe"
    PIP = VENV_DIR / "Scripts" / "pip.exe"
else:
    PYTHON = VENV_DIR / "bin" / "python"
    PIP = VENV_DIR / "bin" / "pip"

REQUIREMENTS = BACKEND_DIR / "requirements.txt"


def run(cmd):
    print("Running:", " ".join(map(str, cmd)))
    subprocess.check_call(cmd)


def recreate_venv():
    if VENV_DIR.exists():
        print("Removing existing virtual environment...")
        shutil.rmtree(VENV_DIR)

    print("Creating virtual environment...")
    venv.create(VENV_DIR, with_pip=True)


def install_requirements():
    print("Installing requirements...")
    run([
        str(PIP),
        "install",
        "-r",
        str(REQUIREMENTS)
    ])


recreate_venv()
install_requirements()

print("Backend environment ready.")
