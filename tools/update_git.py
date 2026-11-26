import datetime
import getpass
import hashlib
import json
import os
from pathlib import Path
import socket
import subprocess


ROOT = Path(__file__).resolve().parents[1]


def git(*arguments):
	result = subprocess.run(
		["git", "-C", str(ROOT), *arguments],
		capture_output=True,
		text=True,
		check=False,
	)
	return result.stdout.strip() if result.returncode == 0 else ""


def main():
	commit = git("rev-parse", "--short", "HEAD") or "unknown"
	branch = git("branch", "--show-current") or "detached"
	if git("status", "--porcelain", "--untracked-files=normal"):
		commit += "-dirty"
	tag = git("describe", "--tags", "--exact-match", "HEAD")
	version = tag if tag and not commit.endswith("-dirty") else f"vNext ({commit})"
	host = socket.gethostname()
	user = getpass.getuser()
	timestamp = datetime.datetime.now().strftime("%y%m%d-%H%M")
	fingerprint_source = f"0-0-0-{commit}-{branch}-{host}-{user}-{timestamp}"
	fingerprint = hashlib.sha1(fingerprint_source.encode()).hexdigest()[:16]
	values = {
		"BUILD_VERSION_STRING": version,
		"BUILD_COMMIT_HASH": commit,
		"BUILD_BRANCH_NAME": branch,
		"BUILD_HOST_NAME": host,
		"BUILD_USER_NAME": user,
		"BUILD_TIMESTAMP": timestamp,
		"BUILD_FINGERPRINT": fingerprint,
	}
	header = "#pragma once\n\n"
	for name, value in values.items():
		header += f"#define {name} {json.dumps(value)}\n"
	output = ROOT / "src/generated/version.hpp"
	output.parent.mkdir(parents=True, exist_ok=True)
	if not output.exists() or output.read_text(encoding="utf-8") != header:
		temporary = output.with_suffix(".tmp")
		temporary.write_text(header, encoding="utf-8")
		os.replace(temporary, output)
	print(f"Version: {version}")


if __name__ == "__main__":
	main()
