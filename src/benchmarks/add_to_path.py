def add():
    import sys
    import os.path
    dir_up = os.path.split(os.path.abspath('.'))[0]
    sys.path.append(dir_up)
