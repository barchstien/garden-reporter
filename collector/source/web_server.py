from http.server import HTTPServer, BaseHTTPRequestHandler
import re

class SimpleHTTPRequestHandler(BaseHTTPRequestHandler):
    def do_GET(self):
        self.send_response(200)
        self.send_header('Content-type', 'text/html')
        self.end_headers()

        # Read content from index.html file and send it as the response
        with open('source/index.html', 'rb') as file:
            content = file.read()
            content = re.sub(b'{{period_value}}', b'2', content)
            content = re.sub(b'{{start_time_value}}', b'20:03', content)
            content = re.sub(b'{{duration_value}}', b'45', content)
            self.wfile.write(content)

def run_server(port=8000):
    server_address = ('', port)
    httpd = HTTPServer(server_address, SimpleHTTPRequestHandler)
    print(f"Server started on port {port}")
    httpd.serve_forever()

if __name__ == '__main__':
    run_server()
