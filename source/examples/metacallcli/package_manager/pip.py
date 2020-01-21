#!/usr/bin/env python3

try:
	from pip import main as pipmain
except ImportError:
	from pip._internal import main as pipmain

def package_manager(args):
	return pipmain(args);
