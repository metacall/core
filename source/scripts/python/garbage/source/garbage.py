#!/usr/bin/env python3

import gc

def set_debug():
    gc.set_debug(gc.DEBUG_LEAK | gc.DEBUG_STATS)

def garbage():
    return repr(gc.garbage)
