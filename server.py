import os
from http.server import HTTPServer, SimpleHTTPRequestHandler
from functools import partial


class CORSRequestHandler(SimpleHTTPRequestHandler):
    def end_headers(self):
        self.send_header("Cross-Origin-Opener-Policy", "same-origin")
        self.send_header("Cross-Origin-Embedder-Policy", "require-corp")
        super().end_headers()


if __name__ == "__main__":
    handler_class = partial(
        CORSRequestHandler,
        directory=os.path.join("build_wasm", "bin", "axmol-extensions"),
    )
    server_address = ("", 8000)
    httpd = HTTPServer(server_address, handler_class)
    print("Serving at http://localhost:8000")
    httpd.serve_forever()
