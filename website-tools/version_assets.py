"""Refresh asset URLs when their content changes, avoiding mixed cached versions."""
import hashlib
import re
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1] / "docs"
PATTERN = re.compile(r'((?:src|href)=")(assets/[^"?]+)(?:\?[^\"]*)?(\")')


def main():
    changed = 0
    for page in ROOT.glob("*.html"):
        content = page.read_text()

        def version(match):
            asset = ROOT / match[2]
            digest = hashlib.sha256(asset.read_bytes()).hexdigest()[:12]
            return f'{match[1]}{match[2]}?v={digest}{match[3]}'

        updated = PATTERN.sub(version, content)
        if updated != content:
            page.write_text(updated)
            changed += 1
    print(f"Updated asset versions on {changed} pages.")


if __name__ == "__main__":
    main()
