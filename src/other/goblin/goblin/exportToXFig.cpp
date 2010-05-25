
//  This file forms part of the GOBLIN C++ Class Library.
//
//  Initially written by Christian Fremuth-Paeger, August 2000
//
//  Copying, compiling, distribution and modification
//  of this source code is permitted only in accordance
//  with the GOBLIN general licence information.

/// \file   exportToXFig.cpp
/// \brief  #exportToXFig class implementation

#include "exportToXFig.h"


static TIndex OFS_COLOUR_TABLE = 32;
static TIndex OFS_NODE_COLOURS = OFS_COLOUR_TABLE+ZERO_COLOUR;
static TIndex NUM_NODE_COLOURS = 254;
static TIndex OFS_EDGE_COLOURS = OFS_NODE_COLOURS+NUM_NODE_COLOURS;
static TIndex NUM_EDGE_COLOURS = 254;

static const double MM_PER_INCH = 25.4;
static const int PIXELS_PER_INCH = 1200;

exportToXFig::exportToXFig(const abstractMixedGraph& _G,const char* expFileName) throw(ERFile) :
    canvasBuilder(_G,MM_PER_INCH/PIXELS_PER_INCH,MM_PER_INCH/PIXELS_PER_INCH),
    expFile(expFileName, ios::out)
{
    if (!expFile)
    {
        sprintf(CT.logBuffer,"Could not open export file %s, io_state %d",
            expFileName,expFile.rdstate());
        Error(ERR_FILE,"exportToXFig",CT.logBuffer);
    }

    expFile.flags(expFile.flags() | ios::right);
    expFile.setf(ios::floatfield);
    expFile.precision(5);

    expFile << "#FIG 3.2" << endl;
    expFile << "#File generated by the GOBLIN library (Release "
        << CT.MajorVersion() << "." << CT.MinorVersion()
        << CT.PatchLevel() << ")" << endl;

    if (DP.CanvasWidth()<=DP.CanvasHeight())
    {
        expFile << "Portrait" << endl;
    }
    else
    {
        expFile << "Landscape" << endl;
    }

    expFile << "Center" << endl;
    expFile << "Inches" << endl;  // "Metric" or "Inches"
    expFile << "A4" << endl;      // Paper format
    expFile << "100.0" << endl;   // Magnification in %
    expFile << "Single" << endl;  // Single page
    expFile << (OFS_COLOUR_TABLE+NO_COLOUR) << endl;      // Transparent background
    expFile << PIXELS_PER_INCH << " 2" << endl;  // 2 = origin at upper left corner

    // Colours

    expFile << endl << "# Colour table:" << endl;

    char rgbBuffer[8];

    for (TIndex i=0;i<ZERO_COLOUR;++i)
    {
        sprintf(rgbBuffer,"#%06lX",DP.RGBFixedColour(i));
        expFile << "0 " << (OFS_COLOUR_TABLE+i) << " " << rgbBuffer << endl;
    }

    if (DP.nodeColourMode==graphDisplayProxy::NODES_FLOATING_COLOURS)
    {
        TIndex effectiveNumColours =
            (DP.maxNodeColour<NUM_NODE_COLOURS) ? DP.maxNodeColour+1 : NUM_NODE_COLOURS;

        for (TIndex i=0;i<effectiveNumColours;++i)
        {
            sprintf(rgbBuffer,"#%06lX",DP.RGBSmoothColour(ZERO_COLOUR+i,ZERO_COLOUR+effectiveNumColours-1));
            expFile << "0 " << (OFS_NODE_COLOURS+i) << " " << rgbBuffer << endl;
        }
    }
    else
    {
        for (TIndex i=0;i<=MAX_COLOUR-ZERO_COLOUR;++i)
        {
            sprintf(rgbBuffer,"#%06lX",DP.RGBFixedColour(ZERO_COLOUR+i));
            expFile << "0 " << (OFS_NODE_COLOURS+i) << " " << rgbBuffer << endl;
        }
    }

    if (DP.arcColourMode==graphDisplayProxy::ARCS_FLOATING_COLOURS)
    {
        TIndex effectiveNumColours =
            (DP.maxEdgeColour<NUM_EDGE_COLOURS) ? DP.maxEdgeColour+1 : NUM_EDGE_COLOURS;

        for (TIndex i=0;i<effectiveNumColours;++i)
        {
            sprintf(rgbBuffer,"#%06lX",DP.RGBSmoothColour(ZERO_COLOUR+i,ZERO_COLOUR+effectiveNumColours-1));
            expFile << "0 " << (OFS_EDGE_COLOURS+i) << " " << rgbBuffer << endl;
        }
    }
    else
    {
        for (TIndex i=0;i<=MAX_COLOUR-ZERO_COLOUR;++i)
        {
            sprintf(rgbBuffer,"#%06lX",DP.RGBFixedColour(ZERO_COLOUR+i));
            expFile << "0 " << (OFS_EDGE_COLOURS+i) << " " << rgbBuffer << endl;
        }
    }


    // Invisible bounding Box
    int minXNodeGrid = int(floor(minX/nodeSpacing-0.5));
    int maxXNodeGrid = int(ceil(maxX/nodeSpacing+0.5));
    int minYNodeGrid = int(floor(minY/nodeSpacing-0.5));
    int maxYNodeGrid = int(ceil((maxY+CFG.legenda)/nodeSpacing+0.5));

    expFile << "2 1 0 0 0 7 150 0 -1 0.000 0 0 -1 0 0 5" << endl;
    expFile << "     "
        << int(DP.CanvasCX(minXNodeGrid*nodeSpacing)) << " "
        << int(DP.CanvasCY(minYNodeGrid*nodeSpacing)) << " "
        << int(DP.CanvasCX(maxXNodeGrid*nodeSpacing)) << " "
        << int(DP.CanvasCY(minYNodeGrid*nodeSpacing)) << " "
        << int(DP.CanvasCX(maxXNodeGrid*nodeSpacing)) << " "
        << int(DP.CanvasCY(maxYNodeGrid*nodeSpacing)) << " "
        << int(DP.CanvasCX(minXNodeGrid*nodeSpacing)) << " "
        << int(DP.CanvasCY(maxYNodeGrid*nodeSpacing)) << " "
        << int(DP.CanvasCX(minXNodeGrid*nodeSpacing)) << " "
        << int(DP.CanvasCY(minYNodeGrid*nodeSpacing)) << endl;

    // Merge the graph into a compound object

    expFile << "6 "
        << int(DP.CanvasCX(minXNodeGrid*nodeSpacing)) << " "
        << int(DP.CanvasCY(minYNodeGrid*nodeSpacing)) << " "
        << int(DP.CanvasCX(maxXNodeGrid*nodeSpacing)) << " "
        << int(DP.CanvasCY(maxYNodeGrid*nodeSpacing))
        << endl;
}


unsigned long exportToXFig::Size() const throw()
{
    return
          sizeof(exportToXFig)
        + managedObject::Allocated();
}


unsigned long exportToXFig::Allocated() const throw()
{
    return 0;
}


void exportToXFig::DisplayLegenda(long xm,long ym,long radius) throw()
{
    int xl = int(xm-radius);
    int xr = int(xm+radius);
    int outlineColour = OFS_COLOUR_TABLE+OUTLINE_COLOUR;
    int fillColour    = OFS_COLOUR_TABLE+NO_COLOUR;
    int forwardArrow = arrowPosMode!=ARROWS_CENTERED && G.Blocking(1);

    // Complete the graph compound object

    expFile << "-6" << endl;


    // Merge the legenda into a nested compound object

    int nodeWidth = int(DP.CanvasNodeWidth());
    int nodeHeight = int(DP.CanvasNodeHeight());
    int arrowSize = int(DP.CanvasArrowSize());
    int textShift = arrowSize;

    expFile << "6 "
        << xl-nodeWidth << " " << ym-nodeHeight << " "
        << xr+nodeWidth << " " << ym+nodeHeight << " "
        << endl;

    expFile << "2 1 0 2 0 7 100 0 -1 0.000 0 0 -1 " << forwardArrow
        << " 0 2" << endl;
    if (forwardArrow) expFile << "2 1 3.00 120.00 150.00" << endl;
    expFile << "     "
        << xl+nodeWidth << " " << ym << " "
        << xr-nodeWidth << " " << ym << " " << endl;

    if (!G.IsUndirected() && arrowPosMode==ARROWS_CENTERED)
    {
        WriteArrow(NoArc,xm+arrowSize,ym,1,0);
        textShift = 2*arrowSize;
    }

    DP.ArcLegenda(tmpLabelBuffer,LABEL_BUFFER_SIZE,"a");

    if (tmpLabelBuffer[0]!=0)
    {
        int arcLabelFontSize = int(DP.ArcLabelFontSize());

        expFile << "4 1 0 30 0 " << arcFontType << " " << arcLabelFontSize << " 0.000 4 "
            << int(2*DP.CanvasNodeWidth()) << " " << int(2*DP.CanvasNodeHeight()) << " "
            << int(xm) << " " << int(ym)+5*arcLabelFontSize-textShift << " "
            << tmpLabelBuffer << "\\001" << endl;
    }

    switch (DP.NodeShapeMode())
    {
        case NODE_SHAPE_POINT:
        {
            WriteSmallNode(NoNode,xl,ym,outlineColour,fillColour);
            WriteSmallNode(NoNode,xr,ym,outlineColour,fillColour);
            WriteNodeLegenda(xl,ym+nodeHeight,"u");
            WriteNodeLegenda(xr,ym+nodeHeight,"v");

            return;
        }
        case NODE_SHAPE_BY_COLOUR:
        case NODE_SHAPE_CIRCULAR:
        {
            WriteCircularNode(NoNode,xl,ym,outlineColour,fillColour);
            WriteCircularNode(NoNode,xr,ym,outlineColour,fillColour);
            WriteNodeLegenda(xl,ym,"u");
            WriteNodeLegenda(xr,ym,"v");

            return;
        }
        case NODE_SHAPE_BOX:
        {
            WriteRectangularNode(NoNode,xl,ym,outlineColour,fillColour);
            WriteRectangularNode(NoNode,xr,ym,outlineColour,fillColour);
            WriteNodeLegenda(xl,ym,"u");
            WriteNodeLegenda(xr,ym,"v");

            return;
        }
    }
}


void exportToXFig::WriteNodeLegenda(int x,int y,char* nodeLabel) throw()
{
    DP.NodeLegenda(tmpLabelBuffer,LABEL_BUFFER_SIZE,nodeLabel);

    if (tmpLabelBuffer[0]==0) return;

    int radius = 100;
    int nodeLabelFontSize = int(DP.NodeLabelFontSize());

    expFile << "4 1 0 30 0 " << nodeFontType << " " << nodeLabelFontSize << " 0.000 4 "
        << 2*radius << " " << 2*radius << " "
        << int(x) << " " << int(y)+5*nodeLabelFontSize << " "
        << tmpLabelBuffer << "\\001" << endl;
}


void exportToXFig::WritePolyLine(vector<double>& cx,vector<double>& cy,
    TDashMode dashMode,int width,TArrowDir displayedArrows,
    TIndex colourIndex,int depth) throw()
{
    int figColour;

    if (colourIndex<ZERO_COLOUR)
    {
        figColour = OFS_COLOUR_TABLE+colourIndex;
    }
    else
    {
        TIndex edgeColour = colourIndex - ZERO_COLOUR;

        if (   ( DP.arcColourMode==graphDisplayProxy::ARCS_FLOATING_COLOURS && edgeColour>=NUM_EDGE_COLOURS )
            || ( DP.arcColourMode==graphDisplayProxy::ARCS_FIXED_COLOURS && colourIndex>=MAX_COLOUR )
           )
        {
            figColour = OFS_COLOUR_TABLE+VAGUE_COLOUR;
        }
        else
        {
            figColour = OFS_EDGE_COLOURS+edgeColour;
        }
    }

    float gap = ((dashMode%4)==0) ? 0.000 : 3.000;

    int forwardArrow = (displayedArrows & ARROW_FORWARD) ? 1 : 0;
    int backwardArrow = (displayedArrows & ARROW_BACKWARD) ? 1 : 0;
    double arrowSize = DP.CanvasArrowSize();

    int nPoints = cx.size();

    if (arcShapeMode==ARC_SHAPE_SMOOTH)
    {
        expFile << "3 4 " << int(dashMode) << " " << width << " "
            << figColour << " 7 " << depth << " 0 -1 "
            << gap << " 0 " << forwardArrow << " " << backwardArrow << " "
            << nPoints << endl;
    }
    else // if (arcShapeMode==ARC_SHAPE_POLYGONES)
    {
        expFile << "2 1 " << int(dashMode) << " " << width << " "
            << figColour << " 7 " << depth << " 0 -1 "
            << gap << " 0 0 -1 " << forwardArrow << " " << backwardArrow << " "
            << nPoints << endl;
    }

    if (forwardArrow)
    {
        expFile << "2 1 " << width << " "
            <<       int(arrowSize*1.8/sqrt(double(width)))
            << " " << int(arrowSize*1.8/sqrt(double(width))) << endl;
    }

    if (backwardArrow)
    {
        expFile << "2 1 " << width << " "
            <<       int(arrowSize*1.8/sqrt(double(width)))
            << " " << int(arrowSize*1.8/sqrt(double(width))) << endl;
    }

    expFile << "    ";


    // Write coordinates
    vector<double>::iterator px = cx.begin();
    vector<double>::iterator py = cy.begin();

    while (px!=cx.end() && py!=cy.end())
    {
        expFile << " " << long(*px) << " " << long(*py);
        ++px;
        ++py;
    }

    expFile << endl;


    if (arcShapeMode==ARC_SHAPE_SMOOTH)
    {
        expFile << "     0.000";

        for (size_t i=0;i<cx.size()-2;++i) expFile << " 1.000";

        expFile << " 0.000" << endl;
    }
}


void exportToXFig::WriteArrow(TArc,long xtop,long ytop,double dx,double dy) throw()
{
    double arrowSize = DP.CanvasArrowSize();
    double ox =  dy;
    double oy = -dx;

    expFile << "2 3 0 1 0 0 100 0 20 0.000 1 0 -1 0 0 5" << endl;
    expFile << "     "
        << int(xtop) << " "
        << int(ytop) << " "
        << int(xtop-(1.8*dx+0.7*ox)*arrowSize) << " "
        << int(ytop-(1.8*dy+0.7*oy)*arrowSize) << " "
        << int(xtop- 1.4*dx        *arrowSize) << " "
        << int(ytop- 1.4*dy        *arrowSize) << " "
        << int(xtop-(1.8*dx-0.7*ox)*arrowSize) << " "
        << int(ytop-(1.8*dy-0.7*oy)*arrowSize) << " "
        << int(xtop) << " "
        << int(ytop) << endl;
}


void exportToXFig::WriteArcLabel(TArc a,long xm,long ym) throw()
{
    DP.CompoundArcLabel(tmpLabelBuffer,LABEL_BUFFER_SIZE,2*a);

    if (tmpLabelBuffer[0]==0) return;

    int arclabelFontSize = int(DP.ArcLabelFontSize());

    expFile << "4 1 0 90 0 " << arcFontType << " " << arclabelFontSize << " 0.000 4 "
        << int(2*DP.CanvasNodeWidth()) << " " << int(2*DP.CanvasNodeHeight()) << " "
        << int(xm) << " " << int(ym)+5*arclabelFontSize << " "
        << tmpLabelBuffer << "\\001" << endl;
}


void exportToXFig::DisplayNode(TNode v) throw()
{
    TIndex canvasColourIndex = DP.CanvasNodeColour(v);

    int outlineColour = OFS_COLOUR_TABLE+OUTLINE_COLOUR;
    int fillColour;

    if (canvasColourIndex<ZERO_COLOUR)
    {
        fillColour = OFS_COLOUR_TABLE+canvasColourIndex;
    }
    else if (DP.nodeColourMode!=graphDisplayProxy::NODES_FLOATING_COLOURS)
    {
        fillColour = (canvasColourIndex<MAX_COLOUR)
            ? OFS_NODE_COLOURS+canvasColourIndex-ZERO_COLOUR
            : OFS_COLOUR_TABLE+VAGUE_COLOUR;
    }
    else
    {
        fillColour = (canvasColourIndex<ZERO_COLOUR+NUM_NODE_COLOURS)
            ? OFS_NODE_COLOURS+canvasColourIndex-ZERO_COLOUR
            : OFS_COLOUR_TABLE+VAGUE_COLOUR;
    }

    int x = DP.CanvasCXOfPoint(v);
    int y = DP.CanvasCYOfPoint(v);

    switch (DP.NodeShapeMode(v))
    {
        case NODE_SHAPE_POINT:
        {
            WriteSmallNode(v,x,y,outlineColour,fillColour);
            TNode p = G.ThreadSuccessor(v);

            if (p==NoNode)
            {
                WriteNodeLabel(v,x+int(DP.CanvasNodeWidth(v)),y+int(DP.CanvasNodeHeight(v)));
            }
            else
            {
                WriteNodeLabel(v,DP.CanvasCXOfPoint(p),DP.CanvasCYOfPoint(p));
            }

            return;
         }
         case NODE_SHAPE_CIRCULAR:
         {
             WriteCircularNode(v,x,y,outlineColour,fillColour);
             WriteNodeLabel(v,x,y);

             return;
         }
         case NODE_SHAPE_BOX:
         {
             WriteRectangularNode(v,x,y,outlineColour,fillColour);
             WriteNodeLabel(v,x,y);

             return;
         }
         case NODE_SHAPE_BY_COLOUR:
         {
             WriteRegularNode(v,x,y,outlineColour,fillColour);
             WriteNodeLabel(v,x,y);

             return;
         }
    }
}


void exportToXFig::WriteSmallNode(TNode v,int x,int y,int outlineColour,int fillColour) throw()
{
    int radius = 60;

    expFile << "1 3 0 4 " << outlineColour << " " << fillColour
        << " 50 0 20 0.000 1 0.0000 "
        << x << " " << y << " "
        << radius << " " << radius << " "
        << x-radius << " " << y << " "
        << x+radius  << " " << y << endl;
}


void exportToXFig::WriteCircularNode(TNode v,int x,int y,int outlineColour,int fillColour) throw()
{
    int radiusX = int(DP.CanvasNodeWidth(v));
    int radiusY = int(DP.CanvasNodeHeight(v));

    expFile << "1 3 0 1 " << outlineColour << " " << fillColour
        << " 50 0 20 0.000 1 0.0000 "
        << x           << " " << y << " "
        << radiusX     << " " << radiusY << " "
        << (x-radiusX) << " " << y << " "
        << (x+radiusX) << " " << y << endl;
}


void exportToXFig::WriteRectangularNode(TNode v,int x,int y,int outlineColour,int fillColour) throw()
{
    int width  = int(DP.CanvasNodeWidth(v));
    int height = int(DP.CanvasNodeHeight(v));

    expFile << "2 2 0 1 " << outlineColour << " " << fillColour
        << " 50 0 20 0.000 0 0 0 0 0 5 " << endl << "   "
        << (x-width) << " " << (y-height) << " "
        << (x-width) << " " << (y+height) << " "
        << (x+width) << " " << (y+height) << " "
        << (x+width) << " " << (y-height) << " "
        << (x-width) << " " << (y-height) << endl;
}


void exportToXFig::WriteRegularNode(TNode v,int x,int y,int outlineColour,int fillColour) throw()
{
    if (G.NodeColour(v)==0 || G.NodeColour(v)>G.N())
    {
        return WriteCircularNode(v,x,y,outlineColour,fillColour);
    }

    unsigned nVertices = G.NodeColour(v)+3;
    double radiusX = DP.CanvasNodeWidth(v)/cos(PI/nVertices);
    double radiusY = DP.CanvasNodeHeight(v)/cos(PI/nVertices);

    expFile << "2 3 0 1 " << outlineColour << " " << fillColour
        << " 50 0 20 0.000 0 0 0 0 0 " << (nVertices+1) << endl << "   ";

    for (unsigned i=0;i<=nVertices;++i)
    {
        double xRel = radiusX*sin((i+0.5)*2*PI/nVertices);
        double yRel = radiusY*cos((i+0.5)*2*PI/nVertices);

        expFile << RoundToInt(x-xRel) << " " << RoundToInt(y+yRel) << " ";
    }

    expFile << endl;
}


void exportToXFig::WriteNodeLabel(TNode v,int x,int y) throw()
{
    DP.CompoundNodeLabel(tmpLabelBuffer,LABEL_BUFFER_SIZE,v);

    if (tmpLabelBuffer[0]==0) return;

    int radius = 100;
    int nodelabelFontSize = int(DP.NodeLabelFontSize());

    expFile << "4 1 0 30 0 " << nodeFontType << " " << nodelabelFontSize << " 0.000 4 "
        << 2*radius << " " << 2*radius << " "
        << int(x) << " " << int(y)+5*nodelabelFontSize << " "
        << tmpLabelBuffer << "\\001" << endl;
}


exportToXFig::~exportToXFig() throw()
{
    expFile << "-6" << endl;
    expFile << endl;
    expFile.close();
}

//  This file forms part of the GOBLIN C++ Class Library.
//
//  Initially written by Christian Fremuth-Paeger, August 2000
//
//  Copying, compiling, distribution and modification
//  of this source code is permitted only in accordance
//  with the GOBLIN general licence information.

/// \file   exportToXFig.cpp
/// \brief  #exportToXFig class implementation

#include "exportToXFig.h"


static TIndex OFS_COLOUR_TABLE = 32;
static TIndex OFS_NODE_COLOURS = OFS_COLOUR_TABLE+ZERO_COLOUR;
static TIndex NUM_NODE_COLOURS = 254;
static TIndex OFS_EDGE_COLOURS = OFS_NODE_COLOURS+NUM_NODE_COLOURS;
static TIndex NUM_EDGE_COLOURS = 254;

static const double MM_PER_INCH = 25.4;
static const int PIXELS_PER_INCH = 1200;

exportToXFig::exportToXFig(const abstractMixedGraph& _G,const char* expFileName) throw(ERFile) :
    canvasBuilder(_G,MM_PER_INCH/PIXELS_PER_INCH,MM_PER_INCH/PIXELS_PER_INCH),
    expFile(expFileName, ios::out)
{
    if (!expFile)
    {
        sprintf(CT.logBuffer,"Could not open export file %s, io_state %d",
            expFileName,expFile.rdstate());
        Error(ERR_FILE,"exportToXFig",CT.logBuffer);
    }

    expFile.flags(expFile.flags() | ios::right);
    expFile.setf(ios::floatfield);
    expFile.precision(5);

    expFile << "#FIG 3.2" << endl;
    expFile << "#File generated by the GOBLIN library (Release "
        << CT.MajorVersion() << "." << CT.MinorVersion()
        << CT.PatchLevel() << ")" << endl;

    if (DP.CanvasWidth()<=DP.CanvasHeight())
    {
        expFile << "Portrait" << endl;
    }
    else
    {
        expFile << "Landscape" << endl;
    }

    expFile << "Center" << endl;
    expFile << "Inches" << endl;  // "Metric" or "Inches"
    expFile << "A4" << endl;      // Paper format
    expFile << "100.0" << endl;   // Magnification in %
    expFile << "Single" << endl;  // Single page
    expFile << (OFS_COLOUR_TABLE+NO_COLOUR) << endl;      // Transparent background
    expFile << PIXELS_PER_INCH << " 2" << endl;  // 2 = origin at upper left corner

    // Colours

    expFile << endl << "# Colour table:" << endl;

    char rgbBuffer[8];

    for (TIndex i=0;i<ZERO_COLOUR;++i)
    {
        sprintf(rgbBuffer,"#%06lX",DP.RGBFixedColour(i));
        expFile << "0 " << (OFS_COLOUR_TABLE+i) << " " << rgbBuffer << endl;
    }

    if (DP.nodeColourMode==graphDisplayProxy::NODES_FLOATING_COLOURS)
    {
        TIndex effectiveNumColours =
            (DP.maxNodeColour<NUM_NODE_COLOURS) ? DP.maxNodeColour+1 : NUM_NODE_COLOURS;

        for (TIndex i=0;i<effectiveNumColours;++i)
        {
            sprintf(rgbBuffer,"#%06lX",DP.RGBSmoothColour(ZERO_COLOUR+i,ZERO_COLOUR+effectiveNumColours-1));
            expFile << "0 " << (OFS_NODE_COLOURS+i) << " " << rgbBuffer << endl;
        }
    }
    else
    {
        for (TIndex i=0;i<=MAX_COLOUR-ZERO_COLOUR;++i)
        {
            sprintf(rgbBuffer,"#%06lX",DP.RGBFixedColour(ZERO_COLOUR+i));
            expFile << "0 " << (OFS_NODE_COLOURS+i) << " " << rgbBuffer << endl;
        }
    }

    if (DP.arcColourMode==graphDisplayProxy::ARCS_FLOATING_COLOURS)
    {
        TIndex effectiveNumColours =
            (DP.maxEdgeColour<NUM_EDGE_COLOURS) ? DP.maxEdgeColour+1 : NUM_EDGE_COLOURS;

        for (TIndex i=0;i<effectiveNumColours;++i)
        {
            sprintf(rgbBuffer,"#%06lX",DP.RGBSmoothColour(ZERO_COLOUR+i,ZERO_COLOUR+effectiveNumColours-1));
            expFile << "0 " << (OFS_EDGE_COLOURS+i) << " " << rgbBuffer << endl;
        }
    }
    else
    {
        for (TIndex i=0;i<=MAX_COLOUR-ZERO_COLOUR;++i)
        {
            sprintf(rgbBuffer,"#%06lX",DP.RGBFixedColour(ZERO_COLOUR+i));
            expFile << "0 " << (OFS_EDGE_COLOURS+i) << " " << rgbBuffer << endl;
        }
    }


    // Invisible bounding Box
    int minXNodeGrid = int(floor(minX/nodeSpacing-0.5));
    int maxXNodeGrid = int(ceil(maxX/nodeSpacing+0.5));
    int minYNodeGrid = int(floor(minY/nodeSpacing-0.5));
    int maxYNodeGrid = int(ceil((maxY+CFG.legenda)/nodeSpacing+0.5));

    expFile << "2 1 0 0 0 7 150 0 -1 0.000 0 0 -1 0 0 5" << endl;
    expFile << "     "
        << int(DP.CanvasCX(minXNodeGrid*nodeSpacing)) << " "
        << int(DP.CanvasCY(minYNodeGrid*nodeSpacing)) << " "
        << int(DP.CanvasCX(maxXNodeGrid*nodeSpacing)) << " "
        << int(DP.CanvasCY(minYNodeGrid*nodeSpacing)) << " "
        << int(DP.CanvasCX(maxXNodeGrid*nodeSpacing)) << " "
        << int(DP.CanvasCY(maxYNodeGrid*nodeSpacing)) << " "
        << int(DP.CanvasCX(minXNodeGrid*nodeSpacing)) << " "
        << int(DP.CanvasCY(maxYNodeGrid*nodeSpacing)) << " "
        << int(DP.CanvasCX(minXNodeGrid*nodeSpacing)) << " "
        << int(DP.CanvasCY(minYNodeGrid*nodeSpacing)) << endl;

    // Merge the graph into a compound object

    expFile << "6 "
        << int(DP.CanvasCX(minXNodeGrid*nodeSpacing)) << " "
        << int(DP.CanvasCY(minYNodeGrid*nodeSpacing)) << " "
        << int(DP.CanvasCX(maxXNodeGrid*nodeSpacing)) << " "
        << int(DP.CanvasCY(maxYNodeGrid*nodeSpacing))
        << endl;
}


unsigned long exportToXFig::Size() const throw()
{
    return
          sizeof(exportToXFig)
        + managedObject::Allocated();
}


unsigned long exportToXFig::Allocated() const throw()
{
    return 0;
}


void exportToXFig::DisplayLegenda(long xm,long ym,long radius) throw()
{
    int xl = int(xm-radius);
    int xr = int(xm+radius);
    int outlineColour = OFS_COLOUR_TABLE+OUTLINE_COLOUR;
    int fillColour    = OFS_COLOUR_TABLE+NO_COLOUR;
    int forwardArrow = arrowPosMode!=ARROWS_CENTERED && G.Blocking(1);

    // Complete the graph compound object

    expFile << "-6" << endl;


    // Merge the legenda into a nested compound object

    int nodeWidth = int(DP.CanvasNodeWidth());
    int nodeHeight = int(DP.CanvasNodeHeight());
    int arrowSize = int(DP.CanvasArrowSize());
    int textShift = arrowSize;

    expFile << "6 "
        << xl-nodeWidth << " " << ym-nodeHeight << " "
        << xr+nodeWidth << " " << ym+nodeHeight << " "
        << endl;

    expFile << "2 1 0 2 0 7 100 0 -1 0.000 0 0 -1 " << forwardArrow
        << " 0 2" << endl;
    if (forwardArrow) expFile << "2 1 3.00 120.00 150.00" << endl;
    expFile << "     "
        << xl+nodeWidth << " " << ym << " "
        << xr-nodeWidth << " " << ym << " " << endl;

    if (!G.IsUndirected() && arrowPosMode==ARROWS_CENTERED)
    {
        WriteArrow(NoArc,xm+arrowSize,ym,1,0);
        textShift = 2*arrowSize;
    }

    DP.ArcLegenda(tmpLabelBuffer,LABEL_BUFFER_SIZE,"a");

    if (tmpLabelBuffer[0]!=0)
    {
        int arcLabelFontSize = int(DP.ArcLabelFontSize());

        expFile << "4 1 0 30 0 " << arcFontType << " " << arcLabelFontSize << " 0.000 4 "
            << int(2*DP.CanvasNodeWidth()) << " " << int(2*DP.CanvasNodeHeight()) << " "
            << int(xm) << " " << int(ym)+5*arcLabelFontSize-textShift << " "
            << tmpLabelBuffer << "\\001" << endl;
    }

    switch (DP.NodeShapeMode())
    {
        case NODE_SHAPE_POINT:
        {
            WriteSmallNode(NoNode,xl,ym,outlineColour,fillColour);
            WriteSmallNode(NoNode,xr,ym,outlineColour,fillColour);
            WriteNodeLegenda(xl,ym+nodeHeight,"u");
            WriteNodeLegenda(xr,ym+nodeHeight,"v");

            return;
        }
        case NODE_SHAPE_BY_COLOUR:
        case NODE_SHAPE_CIRCULAR:
        {
            WriteCircularNode(NoNode,xl,ym,outlineColour,fillColour);
            WriteCircularNode(NoNode,xr,ym,outlineColour,fillColour);
            WriteNodeLegenda(xl,ym,"u");
            WriteNodeLegenda(xr,ym,"v");

            return;
        }
        case NODE_SHAPE_BOX:
        {
            WriteRectangularNode(NoNode,xl,ym,outlineColour,fillColour);
            WriteRectangularNode(NoNode,xr,ym,outlineColour,fillColour);
            WriteNodeLegenda(xl,ym,"u");
            WriteNodeLegenda(xr,ym,"v");

            return;
        }
    }
}


void exportToXFig::WriteNodeLegenda(int x,int y,char* nodeLabel) throw()
{
    DP.NodeLegenda(tmpLabelBuffer,LABEL_BUFFER_SIZE,nodeLabel);

    if (tmpLabelBuffer[0]==0) return;

    int radius = 100;
    int nodeLabelFontSize = int(DP.NodeLabelFontSize());

    expFile << "4 1 0 30 0 " << nodeFontType << " " << nodeLabelFontSize << " 0.000 4 "
        << 2*radius << " " << 2*radius << " "
        << int(x) << " " << int(y)+5*nodeLabelFontSize << " "
        << tmpLabelBuffer << "\\001" << endl;
}


void exportToXFig::WritePolyLine(vector<double>& cx,vector<double>& cy,
    TDashMode dashMode,int width,TArrowDir displayedArrows,
    TIndex colourIndex,int depth) throw()
{
    int figColour;

    if (colourIndex<ZERO_COLOUR)
    {
        figColour = OFS_COLOUR_TABLE+colourIndex;
    }
    else
    {
        TIndex edgeColour = colourIndex - ZERO_COLOUR;

        if (   ( DP.arcColourMode==graphDisplayProxy::ARCS_FLOATING_COLOURS && edgeColour>=NUM_EDGE_COLOURS )
            || ( DP.arcColourMode==graphDisplayProxy::ARCS_FIXED_COLOURS && colourIndex>=MAX_COLOUR )
           )
        {
            figColour = OFS_COLOUR_TABLE+VAGUE_COLOUR;
        }
        else
        {
            figColour = OFS_EDGE_COLOURS+edgeColour;
        }
    }

    float gap = ((dashMode%4)==0) ? 0.000 : 3.000;

    int forwardArrow = (displayedArrows & ARROW_FORWARD) ? 1 : 0;
    int backwardArrow = (displayedArrows & ARROW_BACKWARD) ? 1 : 0;
    double arrowSize = DP.CanvasArrowSize();

    int nPoints = cx.size();

    if (arcShapeMode==ARC_SHAPE_SMOOTH)
    {
        expFile << "3 4 " << int(dashMode) << " " << width << " "
            << figColour << " 7 " << depth << " 0 -1 "
            << gap << " 0 " << forwardArrow << " " << backwardArrow << " "
            << nPoints << endl;
    }
    else // if (arcShapeMode==ARC_SHAPE_POLYGONES)
    {
        expFile << "2 1 " << int(dashMode) << " " << width << " "
            << figColour << " 7 " << depth << " 0 -1 "
            << gap << " 0 0 -1 " << forwardArrow << " " << backwardArrow << " "
            << nPoints << endl;
    }

    if (forwardArrow)
    {
        expFile << "2 1 " << width << " "
            <<       int(arrowSize*1.8/sqrt(double(width)))
            << " " << int(arrowSize*1.8/sqrt(double(width))) << endl;
    }

    if (backwardArrow)
    {
        expFile << "2 1 " << width << " "
            <<       int(arrowSize*1.8/sqrt(double(width)))
            << " " << int(arrowSize*1.8/sqrt(double(width))) << endl;
    }

    expFile << "    ";


    // Write coordinates
    vector<double>::iterator px = cx.begin();
    vector<double>::iterator py = cy.begin();

    while (px!=cx.end() && py!=cy.end())
    {
        expFile << " " << long(*px) << " " << long(*py);
        ++px;
        ++py;
    }

    expFile << endl;


    if (arcShapeMode==ARC_SHAPE_SMOOTH)
    {
        expFile << "     0.000";

        for (size_t i=0;i<cx.size()-2;++i) expFile << " 1.000";

        expFile << " 0.000" << endl;
    }
}


void exportToXFig::WriteArrow(TArc,long xtop,long ytop,double dx,double dy) throw()
{
    double arrowSize = DP.CanvasArrowSize();
    double ox =  dy;
    double oy = -dx;

    expFile << "2 3 0 1 0 0 100 0 20 0.000 1 0 -1 0 0 5" << endl;
    expFile << "     "
        << int(xtop) << " "
        << int(ytop) << " "
        << int(xtop-(1.8*dx+0.7*ox)*arrowSize) << " "
        << int(ytop-(1.8*dy+0.7*oy)*arrowSize) << " "
        << int(xtop- 1.4*dx        *arrowSize) << " "
        << int(ytop- 1.4*dy        *arrowSize) << " "
        << int(xtop-(1.8*dx-0.7*ox)*arrowSize) << " "
        << int(ytop-(1.8*dy-0.7*oy)*arrowSize) << " "
        << int(xtop) << " "
        << int(ytop) << endl;
}


void exportToXFig::WriteArcLabel(TArc a,long xm,long ym) throw()
{
    DP.CompoundArcLabel(tmpLabelBuffer,LABEL_BUFFER_SIZE,2*a);

    if (tmpLabelBuffer[0]==0) return;

    int arclabelFontSize = int(DP.ArcLabelFontSize());

    expFile << "4 1 0 90 0 " << arcFontType << " " << arclabelFontSize << " 0.000 4 "
        << int(2*DP.CanvasNodeWidth()) << " " << int(2*DP.CanvasNodeHeight()) << " "
        << int(xm) << " " << int(ym)+5*arclabelFontSize << " "
        << tmpLabelBuffer << "\\001" << endl;
}


void exportToXFig::DisplayNode(TNode v) throw()
{
    TIndex canvasColourIndex = DP.CanvasNodeColour(v);

    int outlineColour = OFS_COLOUR_TABLE+OUTLINE_COLOUR;
    int fillColour;

    if (canvasColourIndex<ZERO_COLOUR)
    {
        fillColour = OFS_COLOUR_TABLE+canvasColourIndex;
    }
    else if (DP.nodeColourMode!=graphDisplayProxy::NODES_FLOATING_COLOURS)
    {
        fillColour = (canvasColourIndex<MAX_COLOUR)
            ? OFS_NODE_COLOURS+canvasColourIndex-ZERO_COLOUR
            : OFS_COLOUR_TABLE+VAGUE_COLOUR;
    }
    else
    {
        fillColour = (canvasColourIndex<ZERO_COLOUR+NUM_NODE_COLOURS)
            ? OFS_NODE_COLOURS+canvasColourIndex-ZERO_COLOUR
            : OFS_COLOUR_TABLE+VAGUE_COLOUR;
    }

    int x = DP.CanvasCXOfPoint(v);
    int y = DP.CanvasCYOfPoint(v);

    switch (DP.NodeShapeMode(v))
    {
        case NODE_SHAPE_POINT:
        {
            WriteSmallNode(v,x,y,outlineColour,fillColour);
            TNode p = G.ThreadSuccessor(v);

            if (p==NoNode)
            {
                WriteNodeLabel(v,x+int(DP.CanvasNodeWidth(v)),y+int(DP.CanvasNodeHeight(v)));
            }
            else
            {
                WriteNodeLabel(v,DP.CanvasCXOfPoint(p),DP.CanvasCYOfPoint(p));
            }

            return;
         }
         case NODE_SHAPE_CIRCULAR:
         {
             WriteCircularNode(v,x,y,outlineColour,fillColour);
             WriteNodeLabel(v,x,y);

             return;
         }
         case NODE_SHAPE_BOX:
         {
             WriteRectangularNode(v,x,y,outlineColour,fillColour);
             WriteNodeLabel(v,x,y);

             return;
         }
         case NODE_SHAPE_BY_COLOUR:
         {
             WriteRegularNode(v,x,y,outlineColour,fillColour);
             WriteNodeLabel(v,x,y);

             return;
         }
    }
}


void exportToXFig::WriteSmallNode(TNode v,int x,int y,int outlineColour,int fillColour) throw()
{
    int radius = 60;

    expFile << "1 3 0 4 " << outlineColour << " " << fillColour
        << " 50 0 20 0.000 1 0.0000 "
        << x << " " << y << " "
        << radius << " " << radius << " "
        << x-radius << " " << y << " "
        << x+radius  << " " << y << endl;
}


void exportToXFig::WriteCircularNode(TNode v,int x,int y,int outlineColour,int fillColour) throw()
{
    int radiusX = int(DP.CanvasNodeWidth(v));
    int radiusY = int(DP.CanvasNodeHeight(v));

    expFile << "1 3 0 1 " << outlineColour << " " << fillColour
        << " 50 0 20 0.000 1 0.0000 "
        << x           << " " << y << " "
        << radiusX     << " " << radiusY << " "
        << (x-radiusX) << " " << y << " "
        << (x+radiusX) << " " << y << endl;
}


void exportToXFig::WriteRectangularNode(TNode v,int x,int y,int outlineColour,int fillColour) throw()
{
    int width  = int(DP.CanvasNodeWidth(v));
    int height = int(DP.CanvasNodeHeight(v));

    expFile << "2 2 0 1 " << outlineColour << " " << fillColour
        << " 50 0 20 0.000 0 0 0 0 0 5 " << endl << "   "
        << (x-width) << " " << (y-height) << " "
        << (x-width) << " " << (y+height) << " "
        << (x+width) << " " << (y+height) << " "
        << (x+width) << " " << (y-height) << " "
        << (x-width) << " " << (y-height) << endl;
}


void exportToXFig::WriteRegularNode(TNode v,int x,int y,int outlineColour,int fillColour) throw()
{
    if (G.NodeColour(v)==0 || G.NodeColour(v)>G.N())
    {
        return WriteCircularNode(v,x,y,outlineColour,fillColour);
    }

    unsigned nVertices = G.NodeColour(v)+3;
    double radiusX = DP.CanvasNodeWidth(v)/cos(PI/nVertices);
    double radiusY = DP.CanvasNodeHeight(v)/cos(PI/nVertices);

    expFile << "2 3 0 1 " << outlineColour << " " << fillColour
        << " 50 0 20 0.000 0 0 0 0 0 " << (nVertices+1) << endl << "   ";

    for (unsigned i=0;i<=nVertices;++i)
    {
        double xRel = radiusX*sin((i+0.5)*2*PI/nVertices);
        double yRel = radiusY*cos((i+0.5)*2*PI/nVertices);

        expFile << RoundToInt(x-xRel) << " " << RoundToInt(y+yRel) << " ";
    }

    expFile << endl;
}


void exportToXFig::WriteNodeLabel(TNode v,int x,int y) throw()
{
    DP.CompoundNodeLabel(tmpLabelBuffer,LABEL_BUFFER_SIZE,v);

    if (tmpLabelBuffer[0]==0) return;

    int radius = 100;
    int nodelabelFontSize = int(DP.NodeLabelFontSize());

    expFile << "4 1 0 30 0 " << nodeFontType << " " << nodelabelFontSize << " 0.000 4 "
        << 2*radius << " " << 2*radius << " "
        << int(x) << " " << int(y)+5*nodelabelFontSize << " "
        << tmpLabelBuffer << "\\001" << endl;
}


exportToXFig::~exportToXFig() throw()
{
    expFile << "-6" << endl;
    expFile << endl;
    expFile.close();
}
