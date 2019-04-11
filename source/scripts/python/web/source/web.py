#!/usr/bin/python3.5

from os import path
from datetime import datetime

def index():
  basepath = path.dirname(path.abspath(__file__))

  with open(path.join(basepath, 'index.html'), 'r') as f:
    return f.read()

def time():
  return datetime.now().strftime('%Y-%m-%d %H:%M:%S')
