from http.server import HTTPServer, BaseHTTPRequestHandler
import cgi
import re, json, time
from datetime import datetime
import urllib.parse
import yaml

'''
Logic that handles http requests from the water control unit
And that exposes a simple wep page to set water scheduling (for humans)

When the water control unit awakes, it send a GET request to /report
Which variables : 
 * water counter in Liter
   (could be reset any time to 0)
 * water schedule : 
   - period in days
   - next scheduled start time in sec since EPOCH
   - duration in minute
   - enabled
 * status (Could be all 0 after reboot)
  - sec since last boot
  - last water time in sec since epoch
  - last water duration in minutes
  - next water scheduled time in sec since epoch
  - next water scheduled duration in minutes

The request is answered with a JSON object including :
 * water schedule config : 
   - start time in sec since EPOCH
   - period in days
   - duration in minute
   - enabled

None of the above variables or json entries are guaranteed to be present
'''
class WaterWebRequestHandler(BaseHTTPRequestHandler):
    def do_POST(self):
        print('>>', self.path)
        url_parsed = urllib.parse.urlparse(self.path)
        print('>>>>', url_parsed)
        
        # human interface
        if url_parsed.path == '/':
            # get config from yaml
            config = self.load_config_file()
            if config != None:
                # get GET params, if any
                #query_components = urllib.parse.parse_qs(url_parsed.query)
                form = cgi.FieldStorage(
                    fp=self.rfile,
                    headers=self.headers,
                    environ={
                        'REQUEST_METHOD': 'POST',
                        'CONTENT_TYPE': self.headers['Content-Type'],
                    }
                )
                ## Get the form values
                #first_name = form.getvalue("first_name")
                #last_name = form.getvalue("last_name")
                print(form.list)
                #for key in form.keys():
                #    config["water-control"][key] = form.getvalue(key)
                config["water-control"]['start_time'] = form.getvalue('start_time')
                config["water-control"]['period_day'] = int(form.getvalue('period_day'))
                config["water-control"]['duration_minute'] = int(form.getvalue('duration_minute'))
                config["water-control"]['enabled'] = form.getvalue('enable') == 'on'
                print('-->', config)
                self.write_config_file(config)
                
                self.render_index(config["water-control"])
        

    def do_GET(self):
        print('>>', self.path)
        url_parsed = urllib.parse.urlparse(self.path)
        print('>>>>', url_parsed)

        # human interface
        if url_parsed.path == '/':
            # get config from yaml
            config = self.load_config_file()
            if config != None:
                self.render_index(config["water-control"])

        # icon
        elif self.path == '/favicon.ico':
            self.send_response(200)
            self.send_header('Content-Type', 'image/x-icon')
            self.end_headers()
            with open('source/static/favicon.ico', 'rb') as file:
                #self.send_header('Content-Length', 0)
                self.wfile.write(file.read())
        
        # water-controller, ie machine
        elif url_parsed.path == '/report':
            print('-- report !')
            self.send_response(200)
            self.send_header('Content-type', 'application/json')
            self.end_headers()
            query_components = urllib.parse.parse_qs(url_parsed.query)
            print(query_components)
            #period_day = query_components.get('period_day', [''])[0]
            #start_time_hour_minute = query_components.get('start_time_hour_minute', [''])[0]
            #duration_minute = query_components.get('duration_minute', [''])[0]
            enabled = query_components.get('enabled', [''])[0]
            water_liter = query_components.get('water_liter', [''])[0]
            battery_milliv = query_components.get('battery_milliv', [''])[0]
            print("water_liter: ", water_liter);
            print("battery_voltage: ", battery_milliv);

            # TODO log report
            # WARNING
            # Looks like server epoch time is always GMT
            # --> but no need for convertion coz it's never displayed to humans

            config = self.load_config_file()
            start_time = datetime.strptime(config["water-control"]['start_time'], '%Y-%m-%dT%H:%M')
            print('-->', start_time)
            data = {\
                'start_time': int(time.mktime(start_time.timetuple())),
                'period_day': config["water-control"]['period_day'],
                'duration_minute': config["water-control"]['duration_minute'],
                'enabled': config["water-control"]['enabled'],
                'sec_since_1970': int(time.mktime(time.localtime()))
            }
            print("Sending: ", data)
            json_data = json.dumps(data)
            self.wfile.write(json_data.encode())

        elif url_parsed.path == '/debug':
            print('-- debug !')
            self.send_response(200)
            self.send_header('Content-type', 'application/json')
            self.end_headers()
            query_components = urllib.parse.parse_qs(url_parsed.query)
            print(query_components)
        else:
            self.send_error(404, "File Not Found at path:", self.path)
    
    def render_index(self, config):
        self.send_response(200)
        self.send_header('Content-type', 'text/html')
        self.end_headers()
        # Read template content, fill it, send it as the response
        with open('source/static/index.html', 'rb') as file:
            content = file.read()
            content = re.sub(b'{{start_time}}', config['start_time'].encode(), content)
            content = re.sub(b'{{period_day}}', str(config['period_day']).encode(), content)
            content = re.sub(b'{{duration_minute}}', str(config['duration_minute']).encode(), content)
            checked = b''
            if config["enabled"]:
                checked = b'checked'
            content = re.sub(b'{{enable}}', checked, content)
            content = re.sub(b'{{last_scheduled_watering}}', b'unknown', content)
            # TODO ?
            next_debug = datetime.fromtimestamp(time.time() + (3600*12)).strftime('%Y-%m-%d %H:%M').encode('utf-8')
            content = re.sub(b'{{next_scheduled_watering}}', next_debug, content)
            content = re.sub(b'{{battery_status_string}}', b'100% (4.7V)', content)
            content = re.sub(b'{{watering_now_string}}', b'nope', content)
            content = re.sub(b'{{uptime_day_value}}', b'3 days', content)
            self.wfile.write(content)
    
    def load_config_file(self):
        config = None
        with open(self.yaml_config_path, 'r+') as file:
            try:
                config = yaml.safe_load(file)
            except yaml.YAMLError as e:
                print("Failed loading YAML water controller config", e)
        return config
    
    def write_config_file(self, config):
        with open(self.yaml_config_path, 'r+') as file:
            file.seek(0)
            yaml.dump(config, file, sort_keys=False)
            file.truncate()

class WaterWebServer:
    def __init__(self, yaml_config_path) -> None:
        self.yaml_config_path = yaml_config_path

    def run_server(self, port=8000):
        server_address = ('', port)
        httpd = HTTPServer(server_address, WaterWebRequestHandler)
        httpd.RequestHandlerClass.yaml_config_path = self.yaml_config_path
        print(f"Server started on port {port}")
        httpd.serve_forever()


'''For standalone tests'''
if __name__ == '__main__':
    wwc = WaterWebServer(yaml_config_path="./water-web-config.yaml")
    wwc.run_server()
