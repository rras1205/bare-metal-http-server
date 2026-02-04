from http.server import HTTPServer, BaseHTTPRequestHandler
import os

# Load index.html into memory
with open('index.html', 'rb') as f:
    INDEX_HTML = f.read()

class Handler(BaseHTTPRequestHandler):
    def do_GET(self):
        if self.path == '/':
            self.send_response(200)
            self.send_header('Content-Type', 'text/html')
            self.send_header('Content-Length', len(INDEX_HTML))
            self.end_headers()
            self.wfile.write(INDEX_HTML)
        else:
            self.send_response(404)
            self.end_headers()
            self.wfile.write(b'Not Found')
    
    def log_message(self, format, *args):
        pass  # Suppress logging for fair comparison

if __name__ == '__main__':
    server = HTTPServer(('localhost', 4000), Handler)
    print('Python server running on port 4000')
    server.serve_forever()
