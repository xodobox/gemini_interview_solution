import os,time,subprocess,argparse,json
from queue import Queue, Empty
from threading  import Thread

class Test(object):
    def __init__(self, binary, script):
        self.proc = subprocess.Popen(binary,stdin=subprocess.PIPE, stdout=subprocess.PIPE,text=True,shell=True) #universal_newlines=True)
        self.queue = Queue()
        self.worker = Thread(target = self.get_outs)
        self.worker.daemon = True
        self.worker.start()
        self.script = script
    def get_outs(self):
        for line in iter(self.proc.stdout.readline, ''):
            fields = line.replace('\n','').split(' ')
            if len(fields) > 2:
                if (fields[0] == 'TRADE') or (fields[1] == 'SELL') or (fields[1] == 'BUY'):
                    item = ' '.join(fields[:6])
                    self.queue.put(item)

    def test(self):
        success = True
        script = json.load(open(self.script))
        for item in script:
            for c in item['input']:
                self.proc.stdin.write(c+'\n');
            self.proc.stdin.flush()
            time.sleep(0.01)
            results = []
            while True:
                try:
                    r = self.queue.get(timeout=0.1);
                    results.append(r)
                except: break;
            if(results != item['output']):
                print('expected:', item['output']);
                print('got     :', results);
                success = False;
        if success: print('Success');

if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('--binary', required=False, default='../src/build/match_engine',type=str)
    parser.add_argument('--script', required=False, default='./test.json',type=str)
    args = parser.parse_args()
    o = Test(args.binary, args.script)
    o.test()
