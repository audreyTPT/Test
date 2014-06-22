// *********************************************************
// OpenGL Viewer Class, based on LibQGLViewer, compatible
// with most hardware (OpenGL 1.2).
// Author : Tamy Boubekeur (boubek@gmail.com).
// Copyright (C) 2010 Tamy Boubekeur.
// All rights reserved.
// *********************************************************

#ifndef GLVIEWER_H
#define GLVIEWER_H

#include <OpenGL/gl.h>
#include <QGLViewer/qglviewer.h>
#include <QtGui/QKeyEvent>
#include <vector>
#include <string>

#include "Object.h"

class GLViewer : public QGLViewer  {
    Q_OBJECT
public:
    
    typedef enum {Flat=0, Smooth=1} RenderingMode;
    typedef enum {Standard=0, Select=1, Edit=2} SelectionMode;
    
    GLViewer ();
    virtual ~GLViewer ();
    
    inline bool isWireframe () const { return wireframe; }
    inline int getRenderingMode () const { return renderingMode; }
    
    class Exception  {
    public:
        Exception (const std::string & msg) : message ("[GLViewer]"+msg) {}
        virtual ~Exception () {}
        const std::string & getMessage () const { return message; }
    private:
        std::string message;
    }; 
     
public slots :
    void setWireframe (bool b);
    void setRenderingMode (RenderingMode m);
    void setRenderingMode (int m) { setRenderingMode (static_cast<RenderingMode>(m)); }
    void setSelectionMode (SelectionMode m);
    void setSelectionMode (int m) { setSelectionMode (static_cast<SelectionMode> (m)); }
    void reinit();
    void exportMesh();
    void loadMesh();
    void supprBone();
    void setInfluenceArea(bool);
    void setBoneVisualisation(bool);
    
protected :
    void init();
    void initMesh();
    void draw ();
    QString helpString() const;
    void selection(int x, int y);
    void list_hits(GLint hits, GLuint *names);
    bool computeBonesIntersected(QPoint pos, std::map< int, std::pair <int, Vec3Df> > & intersectionList );

    virtual void keyPressEvent (QKeyEvent * event);
    virtual void keyReleaseEvent (QKeyEvent * event);
    virtual void mousePressEvent (QMouseEvent * event);
    virtual void mouseMoveEvent (QMouseEvent * event);
    virtual void mouseReleaseEvent (QMouseEvent * event);
    virtual void wheelEvent (QWheelEvent * e);

private:
    bool wireframe;
    bool influenceArea;
    bool boneVisualisation;
    //bool mode_selected;
    SelectionMode selectionMode;
    RenderingMode renderingMode;
    bool bone_selected, suppr_selected;
    Vec3Df origin, direction; //origin et direction de la caméra vers le point sélectionné
    float mouse_x, mouse_y, mouse_interm_x, mouse_interm_y;
    Object object;
};

#endif // GLVIEWER_H

// Some Emacs-Hints -- please don't remove:
//
//  Local Variables:
//  mode:C++
//  tab-width:4
//  End:
