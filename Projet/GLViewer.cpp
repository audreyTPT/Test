// *********************************************************
// OpenGL Viewer Class, based on LibQGLViewer, compatible
// with most hardware (OpenGL 1.2).
// Author : Tamy Boubekeur (boubek@gmail.com).
// Copyright (C) 2012 Tamy Boubekeur.
// All rights reserved.
// *********************************************************

#include "GLViewer.h"
#include "GLUT/glut.h"
#include "QtGui/QMenu"

#include <iostream>
#include <cstdlib>
#include <cstdio>
#include <cassert>
#include <string>

using namespace std;

GLViewer::GLViewer () : QGLViewer () {
    wireframe = false;
    //mode_selected = false;
    bone_selected = false;
    renderingMode = Smooth;
    selectionMode = Standard;
    
    Mesh ramMesh;
    ramMesh.loadOBJ ("models/skelgros.obj");
    //ramMesh.rotateAroundX(M_PI/2);
    object  = Object(ramMesh);

}

GLViewer::~GLViewer () {
}

void GLViewer::setWireframe (bool b) {
    wireframe = b;
    if (wireframe) 
        glPolygonMode (GL_FRONT_AND_BACK, GL_LINE);
    else
        glPolygonMode (GL_FRONT_AND_BACK, GL_FILL);
    updateGL ();
}

void GLViewer::setRenderingMode (RenderingMode m) {
    renderingMode = m;
    updateGL ();
}

void GLViewer::setSelectionMode(SelectionMode m){
    selectionMode = m;
    updateGL();
}

QString GLViewer::helpString() const {
  QString text("<h2>Raymini</h2>");
  text += "Author: <b>Tamy Boubekeur</b> (boubek@gmail.com)<br>Version: 0.1<br<br>";
  text += "<h3>Disclaimer</h3>";
  text += "This code is not bug-free, use it at your own risks.";
  text += "<h3>Controls</h3>";
  text += "Use the right control panel to setup rendering options.";
  text += "You can respectively zoom and translate with the left and middle mouse buttons. ";
  text += "Pressing <b>Alt</b> and one of the function keys (<b>F1</b>..<b>F12</b>) defines a camera keyFrame. ";
  text += "Simply press the function key again to restore it. Several keyFrames define a ";
  text += "camera path. Paths are saved when you quit the application and restored at next start.<br><br>";
  text += "Press <b>F</b> to display the frame rate, <b>A</b> for the world axis, ";
  text += "<b>Alt+Return</b> for full screen mode and <b>Control+S</b> to save a snapshot. ";
  text += "See the <b>Keyboard</b> tab in this window for a complete shortcut list.<br><br>";
  text += "Double clicks automates single click actions: A left button double click aligns the closer axis with the camera (if close enough). ";
  text += "A middle button double click fits the zoom of the camera and the right button re-centers the scene.<br><br>";
  text += "A left button double click while holding right button pressed defines the camera <i>Revolve Around Point</i>. ";
  text += "See the <b>Mouse</b> tab and the documentation web pages for details.<br><br>";
  text += "Press <b>Escape</b> to exit the viewer.";
  return text;
}

void GLViewer::keyPressEvent (QKeyEvent * /*event*/) {

    /*if (event->key() == Qt::Key_S){
        cout << "je suis en mode selection" << endl;
        mode_selected = !mode_selected;
    }*/

}

void GLViewer::keyReleaseEvent (QKeyEvent * /*event*/) {

}

void GLViewer::mousePressEvent (QMouseEvent * event) {
    
    /*if (mode_selected){
        cout << "je ne sais pas encore quoi faire" << endl;
        // créer une fonction dans object qui renvoie le bone qui a été sélectionné
        selection(event->pos().x(),event->pos().y());
        
    }else{
        QGLViewer::mousePressEvent(event);
    }*/
    
    if (selectionMode == Standard){
        QGLViewer::mousePressEvent(event);
        
    }else{
        
        if (event->button() == Qt::LeftButton){
            //on sélectionne le Bone cliqué si il existe un bone autour
            
            //on repère la position de la souris et lance un rayon
            qglviewer::Vec orig, dir;
            camera()->convertClickToLine(event->pos(), orig, dir);
            origin = Vec3Df(orig[0], orig[1], orig[2]);
            direction = Vec3Df(dir[0], dir[1], dir[2]);
            Ray ray = Ray(origin, direction);
            Bone bone;
            Vec3Df intersectionPoint;
            
            //on appelle une fonction pour trouver le bone d'intersection avec la BoundingBox
            bool intersection = object.getBoneSelected(ray, bone, intersectionPoint);
            
            if(intersection){
                //on a trouvé un bone intersecté, alors on le stock
                //il faudrait marqué dans le menu, ou afficher que l'on a sélectionné un bone ou sinon l'encadrer en orange par exemple (cf blender)
                cout << "BONE !!! " << endl;
                bone_selected = true;
                mouse_x = event->pos().x();
                mouse_y = event->pos().y();
                
                //coloration orange du bone à faire, créer une fonction dans mesh.cpp
                
            }else{
                cout << "je n'ai rien touché" << endl;
                bone_selected = false;
            }
            
        }
    }
    
}

void GLViewer::mouseMoveEvent(QMouseEvent *event){
    
    if (selectionMode == Standard){
        QGLViewer::mouseMoveEvent(event);
    }else{
        
        if (bone_selected){
            //on va commencer à déplacer le bone, on prend le plan dans lequel se trouve le bone
            //on définit le plan : le point d'intersection du bone et le vecteur normal qui est celui de la caméra !
            //cout << "je suis dans le move" << endl;
            
            //la translation selon x et y vaut :
            float dx = -(event->pos().x() - mouse_x);
            float dy = -(event->pos().y() - mouse_y);
            
            dy /= camera()->screenHeight()*0.2;
            dx /= camera()->screenWidth()*0.2;
            
            //on réactualise les valeurs de mouse_x, mouse_y
            mouse_x = event->pos().x();
            mouse_y = event->pos().y();
            
            //on déplace de dx et dy dans le plan les deux vertex du bones !
            //on définit le plan
            Ray ray = Ray(origin, direction);
            Bone bone;
            Vec3Df intersectionPoint;
            object.getBoneSelected(ray, bone, intersectionPoint);
            
            Vec3Df x, y;
            direction.getTwoOrthogonals(y, x);
            
            object.getMesh().modifyMesh(bone, x*dx, y*dy);
            
            //ancienne méthode
            //std::vector<Vertex> bones_vertices = object.getMesh().getBonesVertices();
            //Vertex vert0 = bones_vertices[bone.getVertex(0)];
            //Vertex vert1 = bones_vertices[bone.getVertex(1)];
            
            //dx = dy = 0.1;
            /*Vertex new0 = Vertex(vert0.getPos() + dx*x + dy*y);
            Vertex new1 = Vertex( vert1.getPos() + dx*x + dy*y);
            
            object.getMesh().setVertices(bone.getVertex(0), new0);
            object.getMesh().setVertices(bone.getVertex(1), new1);*/
            
        }
    }
    
}

void GLViewer::mouseReleaseEvent(QMouseEvent *event){
    
    //quand on relache le bouton de gauche, on ne sélectionne plus le bone si on est en mode select
    if (selectionMode == Select){
        
        if ( event->button() == Qt::LeftButton){
            bone_selected = false;
            updateGL();
        }
    }
    
}
void GLViewer::wheelEvent (QWheelEvent * e) {
    QGLViewer::wheelEvent (e);
}

// -----------------------------------------------
// Drawing functions
// -----------------------------------------------

void GLViewer::init() {
    glClearColor (0.f, 0.f, 0.f, 0.0);
    glCullFace (GL_BACK);
    glEnable (GL_CULL_FACE);
    glDepthFunc (GL_LEQUAL);
    glHint (GL_POLYGON_SMOOTH_HINT, GL_NICEST);
    glEnable (GL_POINT_SMOOTH);

    glLoadIdentity ();

    const BoundingBox & box = object.getBoundingBox();
    Vec3Df c = box.getCenter();
    float r = box.getRadius() + 1;
    setSceneCenter (qglviewer::Vec (c[0], c[1], c[2]));
    setSceneRadius (r);
    showEntireScene ();
    
}

void GLViewer::draw () {
    
    const Vec3Df & trans = object.getTrans();
    glPushMatrix();
    glTranslatef(trans[0], trans[1], trans[2]);
    object.getMesh().renderGL(renderingMode == Flat);
    glPopMatrix();

}

void GLViewer::selection(int x, int y){
    
    GLuint buff[64] = {0};
    GLint hits, view[4];
    int id;
    //int dy = glutGet(GLUT_WINDOW_HEIGHT);
    int dy = camera()->screenHeight();
    
    //the buffer where store the info of the selected object
    glSelectBuffer(64, buff);
    glGetIntegerv(GL_VIEWPORT, view);
    //switching into selection mode and initialize the stack name
    glRenderMode(GL_SELECT);
    glInitNames();
    glPushName(0);
    
    //modify the viewing volume (only focusing on the mouse click)
    glMatrixMode(GL_PROJECTION);
 	glPushMatrix();
    glLoadIdentity();
    gluPickMatrix(x, dy - y, 1.0, 1.0, view);
    gluPerspective(60, 1.0, 0.0001, 1000.0);
    glMatrixMode(GL_MODELVIEW);
    const Vec3Df & trans = object.getTrans();
    glTranslatef(trans[0], trans[1], trans[2]);
    object.getMesh().renderGL(renderingMode == Flat);
    glMatrixMode(GL_PROJECTION);
 	glPopMatrix();
    
    hits = glRenderMode(GL_RENDER);
    list_hits(hits, buff);
    
    
}

void GLViewer::list_hits(GLint hits, GLuint *names)
{
 	int i;
    
 	/*
     For each hit in the buffer are allocated 4 bytes:
     1. Number of hits selected (always one,
     beacuse when we draw each object
     we use glLoadName, so we replace the
     prevous name in the stack)
     2. Min Z
     3. Max Z
     4. Name of the hit (glLoadName)
     */
    if (hits == 0){
        cout << "pas d''objet selectionne " << endl;

    }else{
        for (i = 0; i < hits; i++)
            printf(	"Name on stack: %d\n",(GLubyte)names[i * 4 + 3]);

    }
}

