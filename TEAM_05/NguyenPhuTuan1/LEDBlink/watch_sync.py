import time
import subprocess
from pathlib import Path
from watchdog.observers import Observer
from watchdog.events import FileSystemEventHandler

ROOT = Path(__file__).parent
SRC = ROOT / "src" / "main.cpp"
SYNC_CMD = ["python", str(ROOT / "sync_diagram.py")]

class Handler(FileSystemEventHandler):
    def on_modified(self, event):
        if event.src_path.endswith("main.cpp"):
            print("Detected change in main.cpp — syncing diagram.json...")
            subprocess.run(SYNC_CMD)

if not SRC.exists():
    raise SystemExit(f"Cannot find {SRC}")

observer = Observer()
observer.schedule(Handler(), str(SRC.parent), recursive=False)
observer.start()
print("Watching main.cpp for changes. Press Ctrl+C to stop.")
try:
    while True:
        time.sleep(1)
except KeyboardInterrupt:
    observer.stop()
observer.join()
