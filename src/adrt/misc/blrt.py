# default to active layers
# don't export identity matrices

import Blender
import os
from Blender import BGL, Draw, NMesh, Object, Camera, Lamp, Scene, Types
import math

ExportEvt = 10
FilenameEvt = 11
raysEvt = 12
RenderEvt = 13
animEvt = 14

meshEvt = 15
materEvt = 16
lightsEvt = 17
framesEvt = 18
sceneEvt = 19

allLayersEvt = 20
noLayersEvt = 21

layerBase = 40  # Base event for a layer selection
fileNamePrefix = "rt"
fileNameButton = ""
layers = []
i = 0
while i < 20:
    layers.append(1)
    i += 1
layerBits = 0
layerBtns = []

exportStatus = ""
animVal = 0

raysBtn = 0
raysPP = 2

meshVal = 1
materVal = 1
lightsVal = 1
framesVal = 1
sceneVal = 1

shaderList = []

#
#	E V E N T
#
def event(evt, val): # Event handler
    global layerBase, ExportEvt, FilenameEvt, fileNamePrefix, layers, raysEvt

    if evt == Draw.ESCKEY:
        Draw.Exit()
        return
    else:
	# important: do not re-register if event not captured
	return

    Draw.Register(draw_gui, event, button_event)

#
#	B U T T O N _ E V E N T
#
def button_event(evt): # action handler
    global layerBase, ExportEvt, FilenameEvt, fileNamePrefix, fileNameButton
    global raysEvt, layers, exportStatus, raysBtn, raysPP, animVal
    global meshVal, materVal, lightsVal, framesVal, sceneVal
    global meshEvt, materEvt, lightsEvt, framesEvt, sceneEvt
    global layerBtns

    exportStatus = ""
    if evt == ExportEvt:
        exportStatus = "Exporting..."
        exportBlender()
    elif evt == FilenameEvt:
        print "Filename %s" % fileNameButton.val
        fileNamePrefix = fileNameButton.val
    elif evt == RenderEvt:
        renderBlender()
    elif evt >= layerBase and evt < layerBase + 20:
        layerNumber = evt - layerBase
        print "layer %d" % (layerNumber)
        layers[layerNumber] = 1 - layers[layerNumber]
    elif evt == allLayersEvt:
        for i in range(20):
            layers[i] = 1
            layerBtns[i].val = 1
    elif evt == noLayersEvt:
        for i in range(20):
            layers[i] = 0
            layerBtns[i].val = 0
            
    elif evt == raysEvt:
        raysPP = raysBtn.val
    elif evt == animEvt:
        animVal = 1 - animVal
    elif evt == meshEvt:
        meshVal = 1 - meshVal
    elif evt == materEvt:
        materVal = 1 - materVal
    elif evt == lightsEvt:
        lightsVal = 1 - lightsVal
    elif evt == framesEvt:
        framesVal = 1 - framesVal
    elif evt == sceneEvt:
        sceneVal = 1 - sceneVal
    else:
        print "unknown button event"

    Draw.Redraw(1)
    

#
#	D R A W _ G U I
#
def draw_gui(): #render gui
    global layerBase, ExportEvt, FilenameEvt, fileNamePrefix, fileNameButton
    global raysEvt, layers, raysBtn, raysPP, animVal
    global meshVal, materVal, lightsVal, framesVal, sceneVal
    global meshEvt, materEvt, lightsEvt, framesEvt, sceneEvt
    global layerBtns

    BGL.glClearColor(.7, .7, .6, 1)
    BGL.glClear(BGL.GL_COLOR_BUFFER_BIT)
    BGL.glColor3f(0, 0, 0)


    yLoc = 10
    yDelta = 20

    rendWidth = Draw.GetStringWidth("Render", 'normal')
    Draw.Button("Render", RenderEvt, 10, yLoc, \
                rendWidth+10, 16,
                "Render Exported Scene")
    sc = Blender.Scene.GetCurrent()
    p = sc.getWinSize()
    BGL.glRasterPos2i(rendWidth+25, yLoc+4);
    Draw.Text("%d,%d" % (p[0],p[1]))


    yLoc += yDelta

    expWidth = Draw.GetStringWidth("Export", 'normal')
    Draw.Button("Export", ExportEvt, 10, yLoc, expWidth+10, 16, \
                "write to File")
    BGL.glRasterPos2i(expWidth+25, yLoc+4);
    Draw.Text(exportStatus);

    yLoc += yDelta

    fileNameButton = Draw.String("Filename Prefix:", FilenameEvt, \
                                 10, yLoc, \
                                 200, 16, fileNamePrefix, 35, \
                                 "Filename Prefix for output")
    yLoc += yDelta

    raysBtn = Draw.Number("RaysPerPixel:",
                raysEvt, 10, yLoc, 120, 16,
                raysPP, 1, 16, "Number of rays to shoot per pixel")


    yLoc += yDelta

    # Draw the layer buttons
    layerBtn = 0 # start with the bottom row
    btnSize = 10

    for row in range(2):
        ypos = yLoc + (1-row) * btnSize

        xpos = 10
        for col in range(10):
            layerBtns.append(Draw.Toggle("",  layerBase + layerBtn,
                        xpos, ypos, btnSize, btnSize, layers[layerBtn]))
            if (col+1) % 5:
                xpos += btnSize
            else:
                xpos += btnSize * 2
            layerBtn += 1

    Draw.Button("All", allLayersEvt, 130, yLoc, 40, 16)
    Draw.Button("None", noLayersEvt, 180, yLoc, 40, 16)

    yLoc += yDelta + 4

    Draw.Toggle("Save All Frames", animEvt, 10, yLoc, 120, 16, animVal)

    yLoc += yDelta

    x = 10
    Draw.Toggle("Meshes", meshEvt, x, yLoc, 65, 16, meshVal)
    yLoc += yDelta
    Draw.Toggle("Materials", materEvt, x, yLoc, 65, 16, materVal)
    yLoc += yDelta
    Draw.Toggle("Lights", lightsEvt, x, yLoc, 65, 16, lightsVal)
    yLoc += yDelta
    Draw.Toggle("Frames", framesEvt, x, yLoc, 65, 16, framesVal)
    yLoc += yDelta
    Draw.Toggle("Scene", sceneEvt, x, yLoc, 65, 16, sceneVal)




Draw.Register(draw_gui, event, button_event)

#
#	NMeshDump
#
def NMeshDump(of, obj, mesh):
    global shaderList
    myName = obj.getName()
    if not mesh.materials:
        of.write("<Mesh name=\"%s\">\n"% (myName))
    else:
        matName = mesh.materials[0].getName()
        of.write("<Mesh name=\"%s\" shader=\"%s\">\n" % \
                 (myName, matName))
        found = -1
        try:
            found = shaderList.index(matName)
        except ValueError:
            pass


    loc = obj.getLocation()

    v_normals = 0
    uv_coords = 0
    faceCount = 0

    for f in mesh.faces:
        if len(f.v) == 4:
            faceCount += 2
        else:
            faceCount += 1

        if f.smooth != 0:
            v_normals = 1

    for v in mesh.verts:
        NMVertDump(of, v, v_normals)

    for f in mesh.faces:
        NMFaceDump(of, f, myName)
    of.write("</Mesh>\n")

#
#	NMFaceDump
#
def NMFaceDump(of, f, meshName):
    if len(f.v) < 3:
        print "Warning: mesh %s face with %d verts being skipped" % (meshName, len(f.v))
        return
    of.write("    <Face smooth=\"%d\" vlist=\"%d %d %d\">\n" % \
             (f.smooth, f.v[0].index, f.v[1].index, f.v[2].index))

    if len(f.v) == 4:
        of.write("    <Face smooth=\"%d\" vlist=\"%d %d %d\">\n" % \
                 (f.smooth, f.v[0].index, f.v[2].index, f.v[3].index))

    
#
#	NMVertDump
#
def NMVertDump(of, v, normFlag):
    if len(v.co) != 3:
        return

    of.write("    <Vert idx=\"%d\" coord=\"%f %f %f\"" % \
             (v.index, v.co[0], v.co[1], v.co[2]))

    if normFlag != 0:
        of.write(" normal=\"%f %f %f\"" % (v.no[0], v.no[1], v.no[2]))

    of.write(">\n")

#
#	d u m p M e s h e s
#
def dumpMeshes(of):
    global exportStatus, layerBits
    objList = Blender.Object.Get()

    objCount = 0
    for obj in objList:
        if (obj.Layer & layerBits) == 0:
            continue

        if type(obj.getData()) == Types.NMeshType:
            objCount += 1

    i = 0
    for obj in objList:
        if (obj.Layer & layerBits) == 0:
            continue
        data = obj.getData()
        if type(data) == Types.NMeshType:
            exportStatus = "Mesh %d of %d %s" % \
                           (i, objCount, obj.getName())
            print exportStatus
#            Draw.Draw()
            NMeshDump(of, obj, data)
            i += 1

def dumpCamera(of, obj, cam):
    of.write("<Camera")

    loc = obj.matrix[3]
    upvec = obj.matrix[2]
    look = obj.matrix[1]
    of.write(" pos=\"%f %f %f\"" % (loc[0], loc[1], loc[2]))
    of.write(" look=\"%f %f %f\"" % (-upvec[0], -upvec[1], -upvec[2]))
    of.write(" up=\"%f %f %f\"" % (look[0], look[1], look[2]))

    # Vertical FoV = 2 * atan(height/(2*lens_mm)) * pi / 180
    # Horiz FoV = 2 * atan(width/(2*lens_mm)) * pi / 180
    #
    # simulate a 35mm camera w 24x36mm image plane
    # Blender uses the image width for this calcuation
    FoV = math.atan(35.0 / (2.0 * cam.getLens())) * 180 / math.pi * 0.75

    of.write(" fov=\"%f\" rpp=\"%d\"" % (FoV, raysBtn.val))

    if cam.type == 0:
        of.write(" type=\"persp\"")
    elif cam.type == 1:
        of.write(" type=\"ortho\"")

    of.write(">\n")


#
#	getTransform
#
def getTransform(obj):
    parent = obj.getParent()
    if parent != None:
        p = getTransform(parent)
        mat_print(parent.getName(), p)
        m = mat_mul(obj.matrix, p)
        return m
    else:
        return obj.matrix

#
#	dumpFrames
#
def dumpFrames(of):
    global animVal

    print "dumpFrames"
    cur = Blender.Get('curframe')
    if animVal == 0:
        dumpOneFrame(of, cur)
        return

    sta = Blender.Get('staframe')
    end = Blender.Get('endframe')
    for frame in range (sta, end+1):
        Blender.Set('curframe', frame)
        Blender.Window.RedrawAll()
        dumpOneFrame(of, frame)
        

def dumpOneFrame(of, n):
    global fileNamePrefix
    of.write("<Frame imagename=\"%s%04d.bmp\">\n" % (fileNamePrefix, n))
    objList = Blender.Object.Get()
    for obj in objList:
        if (obj.Layer & layerBits) == 0:
            continue
        data = obj.getData()
        if type(data) == Types.CameraType:
            exportStatus = "Camera"
#            Draw.Draw()
            dumpCamera(of, obj, data)
        if type(data) == Types.NMeshType:
            name = obj.getName()
            exportStatus = "Transform Mesh %s" % name
#            Draw.Draw()

            
            m_identity = [[1, 0, 0, 0], [0, 1, 0, 0], [0, 0, 1, 0], [0, 0, 0, 1]]
            m = obj.matrix
            same = 1
            for y in range(4):
                for x in range(4):
                    if m[x][y] != m_identity[x][y]:
                        same = 0

            if same != 1:
                of.write("<transform mesh=\"%s\" matrix=\"\n" % (name) )
                for i in range(4):
                    of.write("%f %f %f %f\n" % (m[i][0], m[i][1], m[i][2], m[i][3]))
                of.write("\">\n")

    of.write("</Frame>\n")




def mat_mul(b, a):
    m = [[1, 0, 0, 0], [0, 1, 0, 0], [0, 0, 1, 0], [0, 0, 0, 1]]
    mat_print("    a", a)
    mat_print("    b", b)
    for j in range(4):
        for i in range(4):
            m[i][j] = a[i][0] * b[0][j] + a[i][1] * b[1][j] + \
                      a[i][2] * b[2][j] + a[i][3] * b[3][j]
    mat_print("    m", m)
    return m


def mat_print(s, m):
    print s
    print "\t%f %f %f %f" % (m[0][0], m[0][1], m[0][2], m[0][3])
    print "\t%f %f %f %f" % (m[1][0], m[1][1], m[1][2], m[1][3])
    print "\t%f %f %f %f" % (m[2][0], m[2][1], m[2][2], m[2][3])
    print "\t%f %f %f %f" % (m[3][0], m[3][1], m[3][2], m[3][3])

#
#	dumpLamp
#
def dumpLamp(of, obj, data):
    l = obj.getLocation()

    of.write("<light pos=\"%f %f %f\"" % (l[0], l[1], l[2]))

    d = obj.matrix[3]

    of.write(" dir=\"%f %f %f\"" % (-d[0], -d[1], -d[2]))
    of.write(" color=\"%f %f %f\"" % (data.R, data.G, data.B))
    of.write(" type=\"%s\">\n" % data.type)


#
#	dumpShaders
#
def dumpShaders(of):
    print "dumpShaders"

    objCount = 0
    objCount = len(Blender.Material.Get())

    i = 0
    for m in Blender.Material.Get():
        myName = m.getName()
        exportStatus = "Shader %s (%d of %d)" % (myName, i, objCount)

        of.write("<shader name=\"%s\">\n" % (myName))

        of.write("  <phong specularity=\"%f\" ambient=\"%f\" transmission=\"%f\" emission=\"%f\" shine=\"%f\" reflection=\"0\">\n" % \
                 (m.getSpec(), m.getAmb(), (1.0-m.getAlpha()), m.getEmit(), m.getHardness()) )

        of.write("  <color r=\"%f\" g=\"%f\" b=\"%f\">\n" % \
                 (m.rgbCol[0], m.rgbCol[1], m.rgbCol[2]))
        of.write("</shader>\n")
        i += 1

    objList = Blender.Object.Get()

    for obj in objList:
        if obj.Layer & layerBits:
            data = obj.getData()
            if type(data) == Types.LampType:
                dumpLamp(of, obj, data)
    


#
#	E X P O R T B L E N D E R
#
def exportBlender():
    global fileNamePrefix, exportStatus, layerBits
    global meshVal, materVal, lightsVal, framesVal, sceneVal
    global meshEvt, materEvt, lightsEvt, framesEvt, sceneEvt

    str = "%sScene.db" % (fileNamePrefix)
    meshFileName = "%sMeshes.db" % fileNamePrefix
    shaderFileName = "%sShaders.db" % fileNamePrefix
    frameFileName = "%sFrames.db" % fileNamePrefix
    
    i = 0
    layerBits = 0
    while i < 20:
        if layers[i] != 0:
            layerBits |= (1 << i)
        i += 1

    if sceneVal == 1:
        scene = open(str, "w")
        exportStatus = "Scenes"
        scene.write("<background r=\"0.2\" g=\"0.3\" b=\"0.3\">\n")
        scene.write("<shaderdata=\"%s\">\n" % shaderFileName)
        scene.write("<meshdata=\"%s\">\n" % meshFileName)
        scene.write("<framedata=\"%s\">\n" % frameFileName)
        Draw.Draw()
        scene.close()

    if meshVal == 1:
        mesh = open(meshFileName, "w")
        exportStatus = "Meshes"
        Draw.Draw()
        dumpMeshes(mesh)
        mesh.close()

    if materVal == 1:
        shaders = open(shaderFileName, "w")
        exportStatus = "Shaders"
        Draw.Draw()
        dumpShaders(shaders)
        shaders.close()

    if framesVal == 1:
        frames = open(frameFileName, "w")
        exportStatus = "Frames"
        Draw.Draw()
        dumpFrames(frames)
        frames.close()

    exportStatus = ""
    Draw.Draw()


def renderBlender():
    print "renderBlender"

    exportBlender()
    sc = Blender.Scene.GetCurrent()
    p = sc.getWinSize()
    
    fd = os.popen("./adrt -s %d,%d -f %sScene.db" % (p[0], p[1], fileNamePrefix))
    fd.close()
