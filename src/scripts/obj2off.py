#!/usr/bin/env python3
# Convert WaveFront OBJ to OFF
# Some time on or before Blender 2.8, support for OFF export
# of meshes was dropped. There don't seem to be any good solutions
# for converting OBJ to OFF with polygon support (MeshLab has a known
# issue at this time).
# Basically read all v entries and f entries. Support negative indices
# and ignore all textures and normals.
# Validate against possible vertex ordering issues - PaperCut assumes
# clockwise ordering when viewing faces from outside the model.

import argparse
import sys
import os

SCRIPT_VER = '1.00'

parser = argparse.ArgumentParser( description = 'Obj2Off v' + SCRIPT_VER )
parser.add_argument( 'InputObj', nargs=1, help = 'Obj file to read' )
parser.add_argument( 'OutputOff', nargs=1, help = 'OFF file to write' )

args = parser.parse_args()

# Make sure input exists
if not os.access(args.InputObj[0], os.R_OK):
  print( 'Input file %s not found or not readable (cwd=%s)' % (args.InputObj[0], os.getcwd()) )
  sys.exit(1)

print( 'obj2off v%s: reading %s, writing %s' % (SCRIPT_VER, args.InputObj[0], args.OutputOff[0]) )
LineCount = 0
VertexCount = 0
FaceCount = 0
v = []
f = []
with open(args.InputObj[0]) as inp:
  s = inp.readline()
  while s:
    LineCount = LineCount + 1
    if s.startswith('v '):
      #print( 'Vertex: %s' % (s))
      VertexCount = VertexCount + 1
      vl = s.split()
	  # Skip first token v
      v.append( (float(vl[1]), float(vl[2]), float(vl[3])) )
    if s.startswith('f '):
      #print( 'Face: %s' % (s))
      # Negative indices are relative to the length (end+1) of vertex array. We need to translate
      # them now to absolute indices. Normal indices are origin:1 and we need to convert to origin:0
      fl = s.split()
      fv = []
      for flv in fl[1:]:
        fi = int(flv.split('/')[0])
        if fi < 0:
          fi = len(v) + fi
        else:
          fi = fi - 1
        fv.append(fi)
      f.append(fv)
      FaceCount = FaceCount + 1
    s = inp.readline()
print( 'Read %d lines, %d vertices, %d faces' % (LineCount, VertexCount, FaceCount) )

with open(args.OutputOff[0], 'w') as outp:
  outp.write('OFF\n')
  outp.write('%d %d 0\n' % (VertexCount, FaceCount))
  for vp in v:
    outp.write('%.07f %.07f %.07f\n' % (vp))
  for fp in f:
    outp.write('%d' % (len(fp)))
    for fpi in fp:
      outp.write(' %d' % (fpi))
    outp.write('\n')
