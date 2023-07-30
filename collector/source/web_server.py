from http.server import HTTPServer, BaseHTTPRequestHandler
import re, json
import urllib.parse

class SimpleHTTPRequestHandler(BaseHTTPRequestHandler):
    def do_GET(self):
        if self.path == '/':
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
        
        elif self.path == '/poll':
            self.send_response(200)
            self.send_header('Content-type', 'application/json')
            self.end_headers()
            data = {'question': 'What is your favorite color?', 'options': ['Red', 'Blue', 'Green']}
            json_data = json.dumps(data)
            self.wfile.write(json_data.encode())
        
        elif self.path.startswith('/update'):
            self.send_response(200)
            self.send_header('Content-type', 'text/html')
            self.end_headers()
            query_components = urllib.parse.parse_qs(urllib.parse.urlparse(self.path).query)
            period_value = query_components.get('period_value', [''])[0]
            start_time_value = query_components.get('start_time_value', [''])[0]
            duration_value = query_components.get('duration_value', [''])[0]
            water_liter_value = query_components.get('water_liter_value', [''])[0]

            response_message = f'Period: {period_value}<br>'
            response_message += f'Start Time: {start_time_value}<br>'
            response_message += f'Duration: {duration_value}<br>'
            response_message += f'Water Liter: {water_liter_value}<br>'
            self.wfile.write(response_message.encode())

        else:
            self.send_error(404, "File Not Found")

def run_server(port=8000):
    server_address = ('', port)
    httpd = HTTPServer(server_address, SimpleHTTPRequestHandler)
    print(f"Server started on port {port}")
    httpd.serve_forever()

if __name__ == '__main__':
    run_server()
