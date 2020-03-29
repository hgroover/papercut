PaperCut v1.0 alpha
$Id: ReadMe.txt 16 2006-04-15 06:39:12Z henry_groover $

General notes
-------------

This is the first release in over a year. Many bugs have been addressed, but I've
also been forced to move to a different development platform (Visual Studio 2005).
Some things may not work the same as with previous versions, particularly in setup
and the help subsystem.

If you run into a problem with this alpha version, please feel free to email me at
support@paper-model.com 

I always like to hear about bugs. Fixes are cheap, but bugs are precious.  If you
want to report a bug in a way that will help me fix it quickly, please make sure 
you know how to reproduce it, and tell me how to make it happen.  Also report the 
first unusual event - what was the FIRST thing that happened that didn't seem 
right.  Usually that's all that's needed.

If you have things to add to the wish list, send them to support@paper-model.com

Henry Groover
28 Oct 2007
San Diego CA USA


New features and fixes
----------------------

  1.01.001 (28 Oct 2007)

  First release built with VS2005
  Enabled image rotation using a very slow method, and fixed a number of bugs related
  to importing arbitrary .off shape files from Blender. There are more edge conditions
  related to edge winding in OFF import. Hopefully they have all been nailed now, but
  the test cases take a bit of time to work through.
  
+ Changes to setup for vs2005
+ Still working with old winhelp but it appears to work. Need to convert to html help
- Need to fix OFF import edge winding problems with deathstar and chumby.
  Pumpkinhead is OK, though.
+ Fixed bug in persistence of rotation-enabled attribute

 
  1.00.300 (08 Apr 2007)
  
  Interim checkin - not released yet - VS2005 conversion in progress
  Fixed bug in placing text and images on non-square quadrangular faces. This may affect
  a larger number of non-triangular faces, since the original technique used was designed
  around triangular faces. Also fixed many bugs in OFF import caused by bad edge winding,
  and added new "force edge split" feature.
  
+ Fix bug in text placement on non-triangular faces.
+ Fix edge winding problem on OFF import. Still not totally there yet - we assume the first
  face has clockwise edges from a view outside the shape, but don't currently have the
  required shape topology analysis in place.
+ Added some validation on OFF import and error / warning reporting
+ Added auto-scaling with confirmation on OFF import
+ Added right-click menu option to edge to force split. This lets us work around certain "scissored"
  layouts (although we mostly avoid them, there are some cases where tab overlaps are not desired)
+ Improved line scaling for print - lines on laser printers at 600dpi+ were mostly invisible.
+ Began conversion of old winhelp to html help
+ Added display text for unconnected face edge tabs - show them as (edge_fq_name) with "face xxx is 
  not connected" in fine print below
+ Added outdent for quadrangular faces

  
  1.00.299 (24 June 2006)
  
  Fixed bugs in 3D shape import and layout calculation. More complex 3D shapes came out
  unhinged, and layouts ended up with overlaps (this has been a problem for some time).
  The solution to the overlapping layout problem required a bit of computational geometry
  speculation which resulted in an interesting solution.
  
+ Fixed bug in 3D shape import which caused open or "scissored" polygons
+ Fixed "bug" which erased concave attribute for tabbed edges. Tabs always fold the same
  way but the concave attribute of joined edges is not always obvious.
+ Fixed layout bugs which caused overlapping parts. A test for polygon collision was
  missing from the brute-force layout algorithm. The test added is based on a postulate
  for polygon intersection which is described at http://paper-model.com/software/polygon-intersection.php


  1.00.298 (27 May 2006)
  
  First version with some level of 3D shape import. Other bug fixes include a bug which
  affected creation of Escher's solid.
 
+ Fixed bug in calculation of trapezoidal pyramid height - this caused Escher's solid to
  use a stellation factor of 100% instead of 2/sqrt(3) (~ 115.47%)
+ Fixed rounding error in get stellation percentage dialog
+ Make stellation percentage persistent, i.e. remember last percentage entered
+ Rough OFF import. We don't check for points in quadrilateral faces being coplanar.
  Two test files are included, monkey.off (moderately complex) and uvsphere.off (simple).


  1.00.297 (7 May 2006)
  
  This is mostly a bug-fix release. There was a problem with the accuracy of angles in the
  rhombic dodecahedron which has been fixed. The shapedef syntax has now been expanded to
  support creation of the rhombic dodecahedron and other rhombus-based shapes. Other defects
  in the .shp file format have been corrected. The Escher's solid shape included with the
  distribution has been corrected based on the updated rhombic dodecahedron angles.
  
+ Added precision to .shp file for edges
+ Added angles to shapedef file
+ Added base edge to .shp file for faces
+ Added rhombic dodecahedron to shapedef file
+ Save concave attribute in .shp file for edges (really need it for Escher's Solid)
+ Fix Escher's Solid - ratio needed is 115.47005383472541098344341974809%, and the rhombic dodecahedron 
  used as a starting point needs to be accurate (the one included as a .shp file previously didn't have 
  accurate angles)


  1.00.296 (24 Apr 2006)
  
  Added rhombic dodecahedron. This is the first solid PaperCut is capable of producing 
  with quadrilateral faces which are not equiangular. It is also a space-filling polyhedron.
  Fixed cumulation of non-equiangular quadrilateral faces. This enables Escher's Solid
  to be generated from a rhombic dodecahedron.
  
+ Allowed inner angle specification from .shp file for polygons where nfaces > 3
+ Use configurable value for tab min fit and shoulder angle
+ Allow join angle of 0 degrees when no tabs are used
+ Allow stellation of entire shape for polyhedra with quadrangular faces
+ Fix bug in handling faces with acute angles - there was a rounding error
  in some of the calculations due to unnecessary conversion from radians to degrees and back.
+ Fix and create test case for cumulation of non-equiangular quadrilateral polygons.


  1.00.295 (15 Apr 2006)
  
  Removed some useless user interface stuff. Added right-click add polygon.
  Added save shape definition file. Added blank tab and no tab options.
  Added open souce license to distro. Need to add it to setup.
  Added more shapes. We now have all but 3 of the Archimedean solids.
  
+ Removed max faces per page
+ Removed rotation slider
+ Changed sense of sizing slider
+ Changed tab shoulder angle from 30 degrees to 27. This should slightly improve possible fits.
+ Add no tab, blank tab, and no-hint tab options to preferences.
+ Added default edge display in non-print non-print preview.
+ Added join edge and detach edge to right click menu.
+ Added Icosidodecahedron (Archimedean A4) and great rhombicuboctahedron (Archimedean A3)
+ Added truncated dodecahedron (Archimedean A10)


  1.00.294 (14 Mar 2006)
  
  Added cumulation of square faces. Added small rhombicuboctahedron. Chased down
  what I thought was a bug in tab labelling but it would not break for me.
  
+ Tried to reproduce tab labelling bug. No success. It seems to be working fine.
+ Added cumulation of square faces. It's not working in an acceptably robust manner.
  It is possible to specify a stellation percentage which is too low and will result
  in a model which does not fit.
+ Added small rhombicuboctahedron. Stellate the square faces on it 200% and you'll
  have the same design as a lamp I saw hanging in my brother's house...


  1.00.293 (9 Mar 2006)
  
  Baseline revision added to svn at sourceforge.net. Added truncated tetrahedron and truncated cube.
  Shape definitions for Shape / Create now read from papercut.shapedef. Changed licensing from freeware 
  to Open Source (ha ha, big difference - just kidding). Some of the places where I'm supposed to 
  display license info might not be updated, though.
  
+ Added truncated tetrahedron. 
+ Added truncated cube
+ Add elongated square Gyrobicupola
+ Reading from papercut.shapedef. 
+ Fixed limitation in not being able to read files over 512k
+ Removed sample content. All those pictures were a nice touch but bloated the download. Need to make it
  available as a separate download.


  1.00.291 (26 Feb 2006)
  
  Added snub cube, north pole orientation (for some shapes), and icosahedron-dodecahedron compound.
  Some enhancements added to face clip/delete dialog (Shape / Clip/delete...) Added layout split
  feature to enable printing on different types of paper. Added image caption feature. Added
  buckyball (truncated icosahedron).
  
+ Added snub cube
+ Added icosa-dodecahedron compound. Can also be done with 1.0.286 and up by K-stellating with
  a factor of 61.803398874989484820458683436564% (2/(1+sqrt(5)), or the inverse of phi, the
  "golden mean").
+ Added buckyball (truncated icosahedron - take each vertex of an icosahedron and slice off a
  chunk - this truncation adds a new pentagonal face. See <a href="http://mathworld.wolfram.com/TruncatedIcosahedron.html">MathWorld</a>
  for further info.
+ Started adding "north pole" orientation for some shapes. Currently set for tetrahedron, cube,
  icosahedron, buckyball and snub cube. Polar orientation for stellation goes towards point of pyramid.
  Doesn't work for images unless you enable image rotation (off by default, still not stable).
+ Changed behavior of clip/delete dialog to allow either clipping or deletion of multiple
  groups, with feedback as to the faces involved appearing in a text control. Reset button
  added to clear the current working set.
+ Added layout split. This allows different face groups to be assigned to separate layouts.
  Only one active layout is visible at a time. This allows different parts of a shape to be
  printed on different colored papers. The interface for this is REALLY rudimentary and clunky -
  I need to implement drag and drop. Another weekend...
+ Change default shape to cube, with a prompt to drag and drop pictures on it.
+ Add image captioning via right-click. Currently we use a fixed size relative to the
  text font and don't support captioning rotated images. Also only one line supported.
+ Some enhancements to page footers for printing.
+ This version has too many new features to be considered stable. It needs testing.


  1.00.289 (19 Feb 2006)
  
  Media Manager added (for copying content). More additions to variable substitution. Default
  picture scaling method for cube faces is now 96% of base. Added two new Archimedean shape primitives,
  the truncated octahedron and cuboctahedron.
  
+ Changed default picture scaling method for cube faces to 96% of base
+ Media manager dialog (new) supports content copying
+ Added $__shapepath__$ to variable substitution - replaced with last pathname shape was
  saved as.
+ Added truncated octahedron and cuboctahedron.
+ Added menu option to create cube-octahedron compound.


  1.00.288 (15 Feb 2006)
  
  This release adds two new shapes, cube and dodecahedron.
  There are still problems with these - stellation is enabled but will cause a crash.
  Also the default image scaling used for cubes is more appropriate for triangles.
  
+ Added cube and dodecahedron
+ Fixed general problems related to number of edges per face <> 3


  1.00.287 (11 Feb 2006)
  
  This release adds some new features to text expansion. See readme.txt for details.
  There is also a bug fix (a bug which was introduced in 1.0.286) where trying to
  stellate with ratios less than 57.735% (tan 30) would crash.
  Shape / Clip has been added. This displays all face groups hierarchically and allows
  the shape to be "clipped" to a particular face group.
  
+ New text expansion - $__globals__$ has been added to predefined variables, and takes 
  an optional parameter to specify the index (default=0). Each geodesic, k-stellation
  or stellation transform adds a new face group, so $__group__(0)$ will be the highest-level 
  group (equivalent to the original face boundary), $__group__(1)$ will be the next highest 
  level, etc.
+ Other new variables are 
	$__ver__$			1.0.287
	$__verdate__$		8 Feb 2006
	$__contentpath__$	c:\documents and settings\user\my documents\my pictures\bunny.jpg
	$__contentfile__$	bunny.jpg
	$__font__$			Arial
	$__page__$			3
	$__shape__$			icosa
	$__nfaces__$		20
+ Fixed bug introduced with stellation calculation change in build 286. Not all stellation
  ratios are now valid. For an equilateral triangle, the maximum will be 57.735%, or tan(30)
+ Updated custom stellation ratio dialog help bitmap
+ Added clip / delete dialog via Shape / Clip
  

  1.00.286 (4 Feb 2006)
  
  A few fixes and enhancements. Changes to tab text should make it easier to
  assemble by referencing the page another piece is on, as well as displaying
  the page this piece belongs on.
  
+ Added K-stellation (after Kepler's Stella Octangula). This type of stellation
  increases the face count by 500% (as opposed to regular stellation, which only
  increases it by 200%).
+ Note that K-stellation does not currently work with faces > 20. You can
  do a geodesic breakdown of an icosahedron and do normal stellation, but
  K-stellation is not allowed currently.
+ For stellation, rename the original face to be orthogonal to the other two
  newly created faces. Do this for K-stellation as well as normal stellation.
+ Tab text now uses a font of a predefined size for the join text. Other information
  displayed on the tab uses a smaller italic font to display face name, page
  number, joining face name, and joining face page number.
+ Fixed build defect where Help / About doesn't show the current version number
+ Changed stellation (K-stellate and normal) to apply percentage to longest pyramid
  edge rather than longest epicenter segment. Percent stellation dialog still documents
  the old method, however.

  
  1.00.284 (24 Jan 2006)
 
  This is the first release using the .NET platform. You may need the .NET runtime. 
  Not tested on pre-XP versions of Windows. Using new deployment system.
 
+ Updated help to point to new url, http://paper-model.com
+ Fix problems with size slider
+ First version built with Visual Studio 2003
+ Implements shape dictionary with $varname$ substitution in face text. Application dictionary
  also implemented. Predefined variables $__date__$ and $__time__$ are available.
+ Fixed some minor bugs handling print margins


  1.00.281 (20 Feb 2004)

+ Enable paging using PgUp/PgDn. First post-hiatus release


  1.00.280 (28 May 2002)

+ Fixed bug where print preview reveals shapes going off page


  1.00.279 (13 May 2002)

+ Fixed bug: tab fold lines should not show up dashed
+ Added geofractal stellation.  Basically a 2v geodesic breakdown, then
  turn the center triangle into the base of a tetrahedron.
+ Added right-click Stellate Face to individually stellate faces


  Build 277 (11 May 2002)

+ Basic bitmap rotation for images.  Disabled by default because it won't work
  with many printers.
+ Add right-click set face image (previously you had to use drag and drop)
+ Always save and load images using relative paths (this breaks "Save as..."
  if you change directories!  No copying or relocation of image files)
+ Fix auto-layout so it always works the same with a given .shp file.
+ Add clipping for images (default is no clipping).  At the moment
  clipping is always on for rotated images.  This is a bug and will be fixed
  in the next build.


  Build 267 (30 Apr 2002)

+ Add shape and global properties
+ Fix reporting of vertex count
+ Improved sizing of text in faces
+ Allow setting face background, edge color, tab edge color, font color, typeface, size
+ Allow basic image size options.  Flow does not work yet.


  Build 258 (20 Apr 2002)

+ Add 4v geodesic
+ Added right-click remove face, remove group (after geodesic and/or stellation)
+ Save scale and other layout information with .shp file
+ Add 10%, 125%, 150%, 200% and other stellation
+ Fix blank line handling for face text
+ Added help text on assembly tips


  Build 240 (15 Apr 2002)

+ Drag and drop handles multiple files
+ Images are resized by area ratio
+ Allow right-click editing of face text


Features to add (the "wish list")
---------------------------------

- Fractal stellation
- Bug reporting utility
- Allow (regular) stellation one face at a time
- One click update - check for new version and download
- Allow manual rotation and positioning of images
- Help still needs more work, only very basic things.
- Add status bar to view window
- Add 3D view (when geometry is known)
- Implement convex hull for fitting
- Use knapsack fitting for pages
- Select vertex as "north pole" and calculate facial orientation
- Render text in face
- Implement resource specification in .shp files
- Allow text files to be specified as a resource and wrapped across 
  adjoining faces
- Support in-place activation and editing via COM
- Export to .gif, .jpg or .png
- Export to .pdf
- Import 3d drawing files
- Allow arbitrary geodesic breakdowns
- Allow an image to be centered on a face and spread over to adjoining faces
  This requires projection
- Print layout summary using empty space on pages
- Allow direct printing of layout summary from View / Layout summary
- Allow arbitrary tweaking of layout
- Build a good collection of examples


Known bugs
----------

- Have occasionally had crashes with release version after dropping large
  number of jpegs, then doing 3 or more resize operations on a 2v or greater
  geodesic
- Cannot split tetrahedron - 2 pages each with 4 faces :-~
- Print rotated clips stellated tetrahedron off page
- Need to print preview on initial auto-layout - sometimes rotation is not right
  Workaround: change size slider slightly and check print preview again
- Operations on 4v geodesics are dog slow, and deleting face groups is painfully
  slow on a 900mhz machine.  At least show an hourglass!
- Geodesic does not use correct reference model.  Applying geodesic
  to a tetrahedron, hexahedron or octahedron does not give the
  expected results!


=================
end of file
