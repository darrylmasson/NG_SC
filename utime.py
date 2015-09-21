#!/usr/bin/env python
# Tue Apr 19 17:13:27 CEST 2011 - Alfio - 

import sys
import os
from datetime import datetime

if len(sys.argv)>1:
    unixtime = float(sys.argv[1])
    print datetime.fromtimestamp(unixtime)

