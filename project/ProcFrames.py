#!/usr/bin/env python
import os

cmd = "/home/gabe/class/cs660/project/imagesubtract "
for i in range(79):
  cmd = "%s frame%d_iq.png" % (cmd, i+1)
print cmd
os.system(cmd)
