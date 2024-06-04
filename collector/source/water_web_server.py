from http.server import HTTPServer, BaseHTTPRequestHandler
import cgi
import re, json, time, datetime
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
   - start time in sec since EPOCH
   - duration in minute
 * status (Could be all 0 after reboot)
  - current time in sec since epoch
  - last water scheduled time in sec since epoch
  - last water scheduled duration in minutes
  - next water scheduled time in sec since epoch
  - next water scheduled duration in minutes

The request is answered with a JSON object including :
 * water schedule config : 
   - start time in sec since EPOCH
   - period in days
   - duration in minute
   (Normally is the same as the request, except if it had been changed by humans)
 * water schedule command :
   - period in days
   - start time in sec since EPOCH
   - duration in minute

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
            config = None
            with open(self.yaml_config_path, 'r+') as file:
                try:
                    config = yaml.safe_load(file)
                    print('orig:', config)
                except yaml.YAMLError as e:
                    print("Failed loading YAML water controller config", e)

                # get GET params, if any
                #query_components = urllib.parse.parse_qs(url_parsed.query)
                form = cgi.FieldStorage(
                    fp=self.rfile,
                    headers=self.headers,
                    environ={'REQUEST_METHOD': 'POST',
                            'CONTENT_TYPE': self.headers['Content-Type'],
                            }
                )
                # Get the form values
                first_name = form.getvalue("first_name")
                last_name = form.getvalue("last_name")
                print(form.list)
                #for key in form.keys():
                #    config["water-control"][key] = form.getvalue(key)
                config["water-control"]['start_time'] = form.getvalue('start_time')
                config["water-control"]['period_day'] = int(form.getvalue('period_day'))
                config["water-control"]['duration_minute'] = int(form.getvalue('duration_minute'))
                config["water-control"]['enabled'] = form.getvalue('enable') == 'on'
                print('-->', config)
                file.seek(0)
                yaml.dump(config, file, sort_keys=False)
                file.truncate()

            self.render_index(config["water-control"])
        

    def do_GET(self):
        print('>>', self.path)
        url_parsed = urllib.parse.urlparse(self.path)
        print('>>>>', url_parsed)

        # human interface
        if url_parsed.path == '/':
            # get config from yaml
            config = None
            with open(self.yaml_config_path, 'rb') as file:
                try:
                    config = yaml.safe_load(file)["water-control"]
                    print (config)
                except yaml.YAMLError as e:
                    print("Failed loading YAML water controller config", e)

                # get GET params, if any
                #query_components = urllib.parse.parse_qs(url_parsed.query)
                #print('components:', query_components)
                #if 'start_time' in query_components:
                #    config['start_time'] = query_components['start_time'][0]
                #if 'period_day' in query_components:
                #    config['period_day'] = query_components['period_day'][0]
                #if 'duration_minute' in query_components:
                #    config['duration_minute'] = query_components['duration_minute'][0]
                #if 'enabled' in query_components:
                #    config['enabled'] = query_components['enabled'][0]
            self.render_index(config)
            

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
            period_day_value = query_components.get('period_day_value', [''])[0]
            start_time_hour_minute_value = query_components.get('start_time_hour_minute_value', [''])[0]
            duration_minute_value = query_components.get('duration_minute_value', [''])[0]
            water_liter_value = query_components.get('water_liter_value', [''])[0]

            print(time.time())
            print(int(time.time()))
            print('++++')
            print(time.gmtime())
            print(time.gmtime().tm_zone)
            print(time.localtime())
            print('=====')
            print(time.mktime(time.localtime()))
            print(time.mktime(time.localtime()) - time.mktime(time.gmtime()))
            print('///////')
            f = 1676591760.6215856
            print(time.mktime(time.localtime(f)) - time.mktime(time.gmtime(f)))
            print(time.gmtime(f))
            print(time.localtime(f))
            # TODO diff gives the +1 of UTC+1, localtile().tm_isdst if true => +1
            f = time.time()
            time_to_send = int(f)
            time_to_send += int(time.mktime(time.localtime(f)) - time.mktime(time.gmtime(f)))
            if time.localtime(f).tm_isdst:
                time_to_send += 3600


            # TODO save those var to display in web ui

            # TODO send new config, if any...
            # debug : echo back the request
            data = {\
                'period_day_value': period_day_value,
                'start_time_hour_minute_value': start_time_hour_minute_value,
                'duration_minute_value': duration_minute_value,
                'sec_since_1970': time_to_send
            }
            print(data)
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
            next_debug = datetime.datetime.fromtimestamp(time.time() + (3600*12)).strftime('%Y-%m-%d %H:%M').encode('utf-8')
            content = re.sub(b'{{next_scheduled_watering}}', next_debug, content)
            content = re.sub(b'{{battery_status_string}}', b'100% (4.7V)', content)
            content = re.sub(b'{{watering_now_string}}', b'nope', content)
            content = re.sub(b'{{uptime_day_value}}', b'3 days', content)
            self.wfile.write(content)


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
