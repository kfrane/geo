from datetime import datetime, timedelta

def measure(func):
    def inner(*args, **kwargs):
        t_start = datetime.now()
        ret = func(*args, **kwargs)
        t_end = datetime.now()
        print 'Function {} was executing for {}'.format(
                func.__name__, (t_end-t_start).total_seconds())
        return ret
    return inner

class Counter():
    def __init__(self):
        self.request_cnt = 0
        self.t_start = datetime.now()

    def add_request(self):
        self.request_cnt += 1

    def rate(self):
        t_end = datetime.now()
        return self.request_cnt/(t_end-self.t_start).total_seconds()
