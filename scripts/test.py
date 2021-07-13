import os,time,subprocess,argparse,json
from queue import Queue, Empty
from threading  import Thread

class Test(object):
    def __init__(self, binary, script):
        self.proc = subprocess.Popen(binary,stdin=subprocess.PIPE, stdout=subprocess.PIPE,text=True,shell=True)
        self.queue = Queue()
        self.worker = Thread(target = self.get_outs)
        self.worker.daemon = True
        self.worker.start()
        self.script = script
    def get_outs(self):
        for line in iter(self.proc.stdout.readline, ''):
            fields = line.replace('\n','').split(' ')
            if len(fields) > 2:
                if (fields[0] == 'TRADE'):
                    item = ' '.join(fields[:6])
                    self.queue.put(item)
                if (fields[1] == 'SELL') or (fields[1] == 'BUY'):
                    item = ' '.join(fields[:5])
                    self.queue.put(item)

    def step(self, process, outpipe, message):
        if isinstance(message,list):
            for m in message: process.stdin.write(m+'\n');
        else: process.stdin.write(message+'\n');
        process.stdin.flush()
        results = []
        while True:
            try: results.append(outpipe.get(timeout=0.1));
            except: break;
        return results

    def test(self):
        success = True
        script = json.load(open(self.script))
        for item in script:
            results = self.step(self.proc, self.queue, item['input'])
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
