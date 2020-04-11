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

