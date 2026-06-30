#!/usr/bin/env python3
"""Minimal HTTP server — serves the SQL editor UI and proxies parse requests to the sql_parser binary."""

import json
import os
import re
import subprocess
import sys
from http.server import HTTPServer, SimpleHTTPRequestHandler
from pathlib import Path

HOST = "127.0.0.1"
PORT = 8765

HERE = Path(__file__).resolve().parent
PARSER = (HERE / ".." / "sql_parser").resolve()

ERROR_RE = re.compile(
    r"^\s*\[ERROR\]\s+Line\s+(\d+),\s+Col\s+(\d+):\s*(?:'([^']*)'\s*[—–-]\s*)?(.*)"
)


def parse_sql(sql: str) -> dict:
    """Run the sql_parser binary on `sql` and return a structured result dict."""
    try:
        result = subprocess.run(
            [str(PARSER), sql],
            capture_output=True,
            text=True,
            timeout=30,
        )
    except FileNotFoundError:
        return {"valid": False, "summary": "sql_parser binary not found", "errorCount": 1, "errors": []}
    except subprocess.TimeoutExpired:
        return {"valid": False, "summary": "Parser timed out", "errorCount": 1, "errors": []}

    stdout = result.stdout
    stderr = result.stderr

    # Detect valid / invalid
    valid = "INVALID SQL" not in stdout and "VALID SQL" in stdout

    # Parse token count from stdout
    tokens = 0
    m = re.search(r"Tokens\s*:\s*(\d+)", stdout)
    if m:
        tokens = int(m.group(1))

    # Parse statement type from stdout
    statement = ""
    m = re.search(r"Statement:\s*(.+)", stdout)
    if m:
        statement = m.group(1).strip()

    # Parse errors from stderr
    errors = []
    for line in stderr.splitlines():
        m = ERROR_RE.match(line)
        if m:
            line_num = int(m.group(1))
            col = int(m.group(2))
            lexeme = m.group(3) or ""
            message = m.group(4)
            errors.append({"line": line_num, "column": col, "lexeme": lexeme, "message": message})

    # Fallback: if stdout has a count but we got no structured errors
    if not valid and not errors:
        summary = "Parse failed (no structured errors)"
        error_count = 1
        m = re.search(r"INVALID\s+SQL\s+\((\d+)\s+errors?\)", stdout)
        if m:
            error_count = int(m.group(1))
    else:
        summary = "VALID SQL" if valid else errors[0]["message"] if errors else "Parse failed"
        error_count = len(errors)

    return {
        "valid": valid,
        "summary": summary,
        "tokens": tokens,
        "statement": statement,
        "errorCount": error_count,
        "errors": errors,
    }


class Handler(SimpleHTTPRequestHandler):
    """Custom handler – serves static files and handles /parse."""

    def do_POST(self):
        if self.path == "/parse":
            length = int(self.headers.get("Content-Length", 0))
            raw = self.rfile.read(length)
            try:
                data = json.loads(raw)
                sql = data.get("sql", "")
            except (json.JSONDecodeError, TypeError):
                self._json(400, {"error": "Invalid JSON"})
                return

            result = parse_sql(sql)
            self._json(200, result)
        else:
            self.send_response(404)
            self.end_headers()

    def _json(self, status, obj):
        payload = json.dumps(obj).encode("utf-8")
        self.send_response(status)
        self.send_header("Content-Type", "application/json")
        self.send_header("Content-Length", str(len(payload)))
        self.end_headers()
        self.wfile.write(payload)

    def log_message(self, fmt, *args):
        sys.stderr.write("[server] %s\n" % (fmt % args))


def main():
    os.chdir(str(HERE))
    server = HTTPServer((HOST, PORT), Handler)
    print("  SQL Parser UI")
    print("  ─────────────────────────────")
    print(f"  Server : http://{HOST}:{PORT}")
    try:
        server.serve_forever()
    except KeyboardInterrupt:
        print("\n  bye.")
        server.server_close()


if __name__ == "__main__":
    main()
