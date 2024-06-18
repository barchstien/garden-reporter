from http.server import ThreadingHTTPServer, BaseHTTPRequestHandler
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
                content_len = int(self.headers.get('Content-Length'))
                post_body = self.rfile.read(content_len)
                print('post_body', post_body.decode("utf-8"))
                # get POST params, if any
                query_components = urllib.parse.parse_qs(post_body.decode("utf-8"))
                print('query_components', query_components)
                try:
                    config["water-control"]['start_time'] = query_components.get('start_time')[0]
                    config["water-control"]['period_day'] = int(query_components.get('period_day')[0])
                    config["water-control"]['duration_minute'] = int(query_components.get('duration_minute')[0])
                    config["water-control"]['enabled'] = query_components.get('enable', ['off'])[0] == 'on'
                    print('-->', config)
                    self.write_config_file(config)
                except Exception as e:
                    print('POST data does not contain expected params, ignore it...')
                    print(e)
                
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
        
        # human interface AJAX
        elif self.path == '/status':
            print('-- status !')
            self.send_response(200)
            self.send_header('Content-type', 'application/json')
            self.end_headers()
            last_schedule = datetime.fromtimestamp(self.server.valve_status.last_water_epoch_t).strftime('%Y-%m-%d %H:%M')
            next_schedule = datetime.fromtimestamp(self.server.valve_status.next_water_epoch_t).strftime('%Y-%m-%d %H:%M')
            battery_status_string = "{:.0f}% ({:.2f})V".format(
                (float(self.server.valve_status.battery_milliv) / 1000.0 - 3.2) * 100  / 1, 
                float(self.server.valve_status.battery_milliv) / 1000.0
            )
            watering_now_string = 'nope'
            if self.server.valve_status.is_water_on :
                watering_now_string = 'yes'
            uptime_day_value = "{:.2f} days".format(
                float(self.server.valve_status.uptime_sec) / (3600.0 * 24.0)
            )
            last_report = datetime.fromtimestamp(self.server.valve_status.last_report).strftime('%Y-%m-%d %H:%M')
            data = {\
                'last_scheduled_watering': last_schedule,
                'next_scheduled_watering': next_schedule,
                'battery_status': battery_status_string,
                'watering_now': watering_now_string,
                'uptime_day': uptime_day_value,
                'last_report': last_report
            }
            print("Sending: ", data)
            json_data = json.dumps(data)
            self.wfile.write(json_data.encode())

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
            # TODO also echo request ?
            self.server.valve_status.water_liter = query_components.get('water_liter', [''])[0]
            self.server.valve_status.battery_milliv = query_components.get('battery_milliv', [''])[0]
            self.server.valve_status.next_water_epoch_t = int(query_components.get('next_water_epoch_t', [''])[0])
            self.server.valve_status.last_water_epoch_t = int(query_components.get('last_water_epoch_t', [''])[0])
            self.server.valve_status.is_water_on = query_components.get('water_on', [''])[0] != '0'
            self.server.valve_status.uptime_sec = query_components.get('uptime_sec', [''])[0]
            print("water_liter: ", self.server.valve_status.water_liter)
            print("battery_voltage: ", self.server.valve_status.battery_milliv)
            print("uptime_sec: ", self.server.valve_status.uptime_sec)
            self.server.valve_status.last_report = time.time()

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
            # config
            content = re.sub(b'{{start_time}}', config['start_time'].encode(), content)
            content = re.sub(b'{{period_day}}', str(config['period_day']).encode(), content)
            content = re.sub(b'{{duration_minute}}', str(config['duration_minute']).encode(), content)
            checked = b''
            if config["enabled"]:
                checked = b'checked'
            content = re.sub(b'{{enable}}', checked, content)

            # status
            last_schedule = b'unknown'
            if self.server.valve_status.last_water_epoch_t != 0:
                last_schedule = datetime.fromtimestamp(self.server.valve_status.last_water_epoch_t).strftime('%Y-%m-%d %H:%M').encode()
            content = re.sub(b'{{last_scheduled_watering_string}}', last_schedule, content)

            next_schedule = datetime.fromtimestamp(self.server.valve_status.next_water_epoch_t).strftime('%Y-%m-%d %H:%M').encode()
            content = re.sub(b'{{next_scheduled_watering_string}}', next_schedule, content)
            
            content = re.sub(b'{{battery_status_string}}', 
                             "{:.0f}% ({:.2f})V".format(
                                 (float(self.server.valve_status.battery_milliv) / 1000.0 - 3.2) * 100  / 1, 
                                 float(self.server.valve_status.battery_milliv) / 1000.0).encode(), 
                            content)
            
            water_now = b'nope'
            if self.server.valve_status.is_water_on :
                water_now = b'yes'
            content = re.sub(b'{{watering_now_string}}', water_now, content)
            
            content = re.sub(b'{{uptime_day_string}}', "{:.2f} days".format(float(self.server.valve_status.uptime_sec) / (3600.0 * 24.0)).encode(), content)

            last_report = datetime.fromtimestamp(self.server.valve_status.last_report).strftime('%Y-%m-%d %H:%M').encode()
            content = re.sub(b'{{last_report_string}}', last_report, content)

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

class ValveStatus:
    def __init__(self):
        self.uptime_sec = 0
        self.water_liter = 0
        self.battery_milliv = 0
        self.next_water_epoch_t = 0
        self.last_water_epoch_t = 0
        self.is_water_on = False
        self.last_report = 0

class WaterWebServer:
    def __init__(self, yaml_config_path) -> None:
        self.yaml_config_path = yaml_config_path
        self.valve_status = ValveStatus()

    def run_server(self, port=8000):
        server_address = ('', port)
        httpd = ThreadingHTTPServer(server_address, WaterWebRequestHandler)
        httpd.RequestHandlerClass.yaml_config_path = self.yaml_config_path
        httpd.valve_status = ValveStatus()
        print(f"Server started on port {port}")
        httpd.serve_forever()


'''For standalone tests'''
if __name__ == '__main__':
    wwc = WaterWebServer(yaml_config_path="./water-web-config.yaml")
    wwc.run_server()
