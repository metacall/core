#!/usr/bin/python3.5

from os import path
from datetime import datetime

def index():
  basepath = path.dirname(path.abspath(__file__))

  f = open(path.join(basepath, 'index.html'), 'r')

  return f.read()

def time():
  return datetime.now().strftime('%Y-%m-%d %H:%M:%S')
