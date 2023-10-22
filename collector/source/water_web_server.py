from http.server import HTTPServer, BaseHTTPRequestHandler
import re, json, time, datetime
import urllib.parse

'''
Logic that handles http requests from the water control unit
And that exposes a simple wep page to set water scheduling (for humans)

When the water control unit awakes, it send a GET request to /report
Which variables : 
 * water counter in Liter
   (could be reset any time to 0)
 * water schedule : 
   - period in days
   - start time in HH:mm
   - duration in minute
 * status (Could be all 0 after reboot)
  - current time in sec since epoch
  - last water scheduled time in sec since epoch
  - last water scheduled duration in minutes
  - next water scheduled time in sec since epoch
  - next water scheduled duration in minutes

The request is answered with a JSON object including :
 * water schedule config : 
   - period in days
   - start time in HH:mm
   - duration in minute
   (Normally is the same as the request, except if it had been changed by humans)
 * water schedule command :
   - period in days
   - start time in HH:mm
   - duration in minute

None of the above variables or json entries are guaranteed to be present
'''
class WaterWebRequestHandler(BaseHTTPRequestHandler):
    def do_GET(self):
        print('>>', self.path)
        print('>>>>', urllib.parse.urlparse(self.path))
        url_parsed = urllib.parse.urlparse(self.path)
        
        if url_parsed.path == '/':
            self.send_response(200)
            self.send_header('Content-type', 'text/html')
            self.end_headers()

            # Read content from index.html file and send it as the response
            with open('source/static/index.html', 'rb') as file:
                content = file.read()
                content = re.sub(b'{{period_day_value}}', b'2', content)
                content = re.sub(b'{{start_time_hour_minute_value}}', b'20:03', content)
                content = re.sub(b'{{duration_minute_value}}', b'45', content)
                content = re.sub(b'{{enable_value}}', b'checked', content)
                content = re.sub(b'{{last_scheduled_watering}}', b'unknown', content)
                # debug
                next_debug = datetime.datetime.fromtimestamp(time.time() + (3600*12)).strftime('%Y-%m-%d %H:%M').encode('utf-8')
                content = re.sub(b'{{next_scheduled_watering}}', next_debug, content)
                content = re.sub(b'{{battery_status_string}}', b'100% (4.7V)', content)
                content = re.sub(b'{{watering_now_string}}', b'nope', content)
                content = re.sub(b'{{uptime_day_value}}', b'3 days', content)
                self.wfile.write(content)
        
        elif self.path == '/favicon.ico':
            self.send_response(200)
            self.send_header('Content-Type', 'image/x-icon')
            self.end_headers()
            with open('source/static/favicon.ico', 'rb') as file:
                #self.send_header('Content-Length', 0)
                self.wfile.write(file.read())
        
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

class WaterWebServer:
    def run_server(this, port=8000):
        server_address = ('', port)
        httpd = HTTPServer(server_address, WaterWebRequestHandler)
        print(f"Server started on port {port}")
        httpd.serve_forever()


'''For standalone tests'''
if __name__ == '__main__':
    wwc = WaterWebServer()
    wwc.run_server()
