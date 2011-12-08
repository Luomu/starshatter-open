
 ======================================
 OPCODE (OPtimized COllision DEtection)
 ======================================

 Demo version 1.3b

 This demo compares RAPID 2.01 vs OPCODE 1.0.

 Information about RAPID can be found there:
 http://www.cs.unc.edu/~geom/OBB/OBBT.html

 Information about OPCODE can be found there:
 http://www.codercorner.com/Opcode.htm

                             -----

 I tried to be as fair as possible, and selected several scenes
 where both RAPID and OPCODE show their forces. RAPID is usually
 faster in close-proximity scenarii, especially when one object
 is totally surrounded by another. OPCODE is usually faster when
 objects deeply overlap. On my machine (Celeron 500Mhz), I have
 found OPCODE to be faster overall. See for yourself.

 By the way, don't forget OPCODE's primary goal was *memory*, not
 speed. Considering this, I'm quite pleased with the results!

                             -----

 Scenes have been exported with Flexporter:
 http://www.codercorner.com/Flexporter.htm

 If you don't like them, you can export your own test scenes from
 MAX - just ensure there are only 2 meshes in them.
 
                             -----

 How to proceed:
 1) Run Opcode.exe
 2) Drag&drop a ZCB file on the window
    Messages such as "Chunk MOVE not found" are normal.
 3) Play!

                             -----

 OPCODE-related keys:

 1   Toggle contact mode
      - All contacts: report all colliding triangles
      - First contact: report first contact only (a simple yes/no
        answer to the overlap question)

 2   Toggle BV-BV tests
      - full tests: standard SAT (15 separating axes)
      - no class III: SAT-lite (6 separating axes)

 3   Toggle Prim-BV tests
      - full tests: standard SAT (15 separating axes)
      - no class III: SAT-lite (6 separating axes)

 4   Toggle leaf nodes
      - discarded: use N-1 nodes only for a complete tree
      - kept: standard complete tree with 2*N-1 nodes

 5   Toggle compression
      - enabled: use quantized trees
      - disabled: use normal trees

 6   Toggle temporal coherence (only for first contact mode)
      - enabled: test the previous pair of colliding triangles
                 before everything else.
      - disabled: well, do not....

                             -----

 NB: in order to switch from one feature to another quickly, I create
 4 trees for each mesh: normal, no-leaf, quantized, quantized no-leaf.

 i.e. 4 OPCODE models + 1 RAPID model / mesh, which explains why the
 building phase is not very fast... Especially in the last scene, it
 takes a while - please wait!

                             -----

 Generic keys to play with:

 F1 Texture control
 F2 Camera control
 F3 Mesh control
 F4 Material control
 F5 Scene control
 F6 Light control
 F7 Helper control

 To move the camera or a mesh, use the mouse. Should be intuitive.
 In camera mode, press +/- to jump from one camera to another.
 Actually +/- jump to "next current" or "previous current", where
 "current" is a mesh, camera, texture, etc - depends on selected
 control mode.

 To test collision detection, useful keys are:
 - S to spin a model automatically (on/off)
 - P to pause any animation
 - O (in pause mode) to play one frame only

 Other keys shouldn't be useful there but anyway:
 - F displays the framerate on/off (BTW, I do 2 collision queries/frame
   and displays a lot of slow text, so don't pay too much attention to
   the overall framerate)
 - T toggles texturing
 - G toggles gouraud
 - W toggles wireframe
 - R toggles the profiling
 - I toggles general information
 - B toggles bounding boxes

 Special keys
 - Y should subdivide a mesh with the Butterfly algorithm. Since it
   implies rebuilding all collision trees, it may be quite slow.
 - L should smooth a mesh (same remark)
 - M should make a mesh manifold, in case ICE complains....
 - U should unfold a mesh, so beware.

 Otherwise, play with the menu *at your own risk*.


                             -----

 In situations where RAPID is really faster (for example when a mesh
 is surrounded by another one) try re-enabling leaf nodes....

                             -----

 Pierre Terdiman
 May 03, 2001

 p.terdiman@wanadoo.fr
 p.terdiman@codercorner.com
 
 www.codercorner.com

 