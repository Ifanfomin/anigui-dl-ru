from pathlib import Path
import subprocess
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


def ensure_venv():
    if not VENV_DIR.exists():
        print("Creating virtual environment...")
        venv.create(VENV_DIR, with_pip=True)
    else:
        print("Virtual environment already exists.")


def upgrade_pip():
    print("Upgrading pip...")
    run([
        str(PYTHON),
        "-m",
        "pip",
        "install",
        "--upgrade",
        "pip"
    ])


def install_requirements():
    print("Installing requirements...")
    run([
        str(PIP),
        "install",
        "-r",
        str(REQUIREMENTS)
    ])


ensure_venv()
upgrade_pip()
install_requirements()

print("Backend environment ready.")