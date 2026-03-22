import re
import json
from pathlib import Path

ROOT = Path(__file__).parent
MAIN = ROOT / "src" / "main.cpp"
DIAGRAM = ROOT / "diagram.json"

if not MAIN.exists():
    raise SystemExit(f"Cannot find {MAIN}")
if not DIAGRAM.exists():
    raise SystemExit(f"Cannot find {DIAGRAM}")

text = MAIN.read_text(encoding="utf-8")
# read defines like: #define GREEN_PIN 17
pins = {}
for m in re.finditer(r"#\s*define\s+(\w+)_PIN\s+(\d+)", text):
    name = m.group(1).upper() + "_PIN"
    pins[name] = m.group(2)

# mapping from led id in diagram to define name in code
mapping = {
    "led_green": "GREEN_PIN",
    "led_yellow": "YELLOW_PIN",
    "led_red": "RED_PIN",
}

with DIAGRAM.open(encoding="utf-8") as f:
    data = json.load(f)

changed = False
for i, conn in enumerate(data.get("connections", [])):
    if not isinstance(conn, list) or len(conn) < 2:
        continue
    left = conn[0]
    # look for patterns like 'led_green:A'
    if ":A" in left:
        ledid = left.split(":")[0]
        if ledid in mapping:
            define = mapping[ledid]
            pin = pins.get(define)
            if pin:
                new = f"esp:{pin}"
                if conn[1] != new:
                    data["connections"][i][1] = new
                    changed = True

if changed:
    DIAGRAM.write_text(json.dumps(data, indent=2, ensure_ascii=False), encoding="utf-8")
    print("diagram.json updated")
else:
    print("No changes needed")
