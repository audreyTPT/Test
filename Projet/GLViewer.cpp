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
#include <fstream>
#include <cstdlib>
#include <cstdio>
#include <cassert>
#include <string>
#include <QFileDialog>
#include <QTextStream>

#include <opencv.hpp>

static float depth_map = false;

using namespace std;

GLViewer::GLViewer () : QGLViewer () {
    wireframe = false;
    influenceArea = false;
    boneVisualisation = true;
    bone_selected = false;
    suppr_selected = false;
    renderingMode = Smooth;
    selectionMode = Standard;
    initMesh();
    updateGL();
    
}

void GLViewer::initMesh() {
    
    Mesh ramMesh;
    QString string = "models/bone.obj";
    ramMesh.loadOBJ (string.toStdString());
    object  = Object(ramMesh);
    
}

void GLViewer::loadMesh(){
    
    QString name = QFileDialog::getOpenFileName(this,"Ouvrir un mesh");
    Mesh mesh;
    //hypothèse : le mesh se trouve dans le répertoire /models.
    QStringList listName = name.split("/");
    string finalName = "models/" + listName[listName.size()-1].toStdString();
    mesh.loadOBJ(finalName);
    object = Object(mesh);
    //dans le cas où l bouton areainfluence est enclenché, il faut tout de suite calculer les poids !
    if (influenceArea){
        object.getMesh().initWeights();
    }
    updateGL();
    
}

void GLViewer::supprBone(){
    
    // on ne peut supprimer un bone que quand on est dans le mode Edit
    if (bone_selected && selectionMode == Edit){
        //alors origin et direction sont à jour et permettent d'obtenir le bone selectionné
        Ray ray = Ray(origin, direction);
        int idx_bone;
        Vec3Df intersectionPoint;
        object.getBoneSelected(ray, idx_bone, intersectionPoint);
        
        object.getMesh().suppr(idx_bone);
        //le bone est suuprimé donc n'est plus sélectionné
        bone_selected = false;
        
        //dans le cas ou le bouton areainfluence est enclenché, il faut tout de suite calculer les poids !
        if (influenceArea){
            object.getMesh().initWeights();
        }
        updateGL();
    }
    
}

void GLViewer::exportMesh(){
    //on enregistre le mesh ainsi que les bones dans un .obj (les bones sont des objets qui commencent par S)
    //attention, lors de l'enregistrement, on enregistre d'abord les vertices du mesh et ensuite les vertices du bones (dont les indices sont ceux consécutifs aux vertices du mesh).
    
    //demander le nom sous lequel est enregistré le mesh et où
    QString name = QFileDialog::getSaveFileName(this, "Enregistrer un fichier en .obj", QString(), "Mesh (*.obj)");
    QFileInfo f( name);
    if (f.suffix().isEmpty()){
        name +=".obj";
    }
    
    QFile file(name);
    
    if (!file.open(QIODevice::WriteOnly)){
        cout << "c'est une erreur de fichier" << endl;
    }else{
        
        QTextStream stream(&file);
        //on enregistre d'abord l'objet
        stream << "o face" << endl;
        
        //on enregistre tous les vertices
        std::vector<Vertex> vertices = object.getMesh().getVertices();
        
        for (unsigned int i = 0; i< vertices.size(); i++){
            stream << "v " << vertices[i].getPos()[0] << " " << vertices[i].getPos()[1] << " " << vertices[i].getPos()[2] << endl;
        }
        
        //on enregistre toutes les faces (ici triangles)
        //attention, si on veut rajouter la texture, il faut rajouter des / avec le numéro de la texture
        //il faudrait rajouter aussi une autre boucle pour écrire les textures avec vt au début
        std::vector<Triangle> triangles = object.getMesh().getTriangles();
        
        for (unsigned int i = 0; i< triangles.size() ; i++){
            stream << "f " << triangles[i].getVertex(0)+1 << " " << triangles[i].getVertex(1)+1 << " " << triangles[i].getVertex(2)+1 << endl;
        }
        
        //on enregistre ensuite les bones
        stream << "s bone" << endl;
        
        //on enregistre les vertices des bones
        std::vector<Vertex> bones_vertices = object.getMesh().getBonesVertices();
        for (unsigned int i =0; i< bones_vertices.size(); i++){
            stream << "v " << bones_vertices[i].getPos()[0] << " " << bones_vertices[i].getPos()[1] << " " << bones_vertices[i].getPos()[2] << endl;
        }
        
        //on enregistre les lignes ou les poignées des bones
        //les index des vertices des bones commencent à partir de la fin des index des vertices du mesh !
        std::vector< Armature* > armatures = object.getMesh().getBones();
        
        for (unsigned int i =0; i< armatures.size(); i++){
            
            if (armatures[i]->getType() =="bone"){
                // alors il s'agit d'un bone et on trace une ligne -> "l"
                stream << "l " << armatures[i]->getVertex(0) + vertices.size()+1 << " " << armatures[i]->getVertex(1) + vertices.size()+1 << endl;
            }
            
            if (armatures[i]->getType() == "handle"){
                //alors il s'agit d'un handle, donc d'un unique point
                stream << "lv " << armatures[i]->getVertex(0) + vertices.size()+1 << endl;
            }
        }
        
    }
        
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

void GLViewer::setInfluenceArea(bool b){
    influenceArea = b;
    //je calcule le poids seulement si je veux afficher les zone d'influence !
    if (b){
        object.getMesh().initWeights();
    }
    updateGL();
}

void GLViewer::setBoneVisualisation(bool b){
    boneVisualisation = !b;
    
    updateGL();
}

void GLViewer::setRenderingMode (RenderingMode m) {
    renderingMode = m;
    updateGL ();
}

void GLViewer::setSelectionMode(SelectionMode m){
    selectionMode = m;
    //on réinitialise
    bone_selected = false;
    updateGL();
}

void GLViewer::reinit(){
    
    bone_selected = false;
    initMesh();
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
    
    //on va sélectionner un bone donc on réinitialise la variable
    bone_selected = false;
    
    if (selectionMode == Standard){
        QGLViewer::mousePressEvent(event);
        
    }else if (selectionMode == Select){
        
        if (event->button() == Qt::LeftButton){
            //on sélectionne le Bone cliqué si il existe un bone autour
            
            std::map<int, std::pair<int, Vec3Df> > intersectionList;
            
            bool intersection = computeBonesIntersected(event->pos(), intersectionList);
            
            if (intersection){
                
                //il faut trouver le bone qui a été toucher le plus de fois pour ensuite modifier direction et prendre la direction du bone le plus touché (utile pour la suite d'avoir la direction...)
                
                int max = 0;
                for (map<int, pair<int, Vec3Df> >::iterator it = intersectionList.begin(); it != intersectionList.end(); it++){
                    //le deuxième int correspond au nombre de fois où le bone a été touché par les rayons.
                    if ( (*it).second.first > max){
                        direction = (*it).second.second;
                    }
                }
                
                cout << "BONE !!! " << endl;
                bone_selected = true;
                mouse_x = mouse_interm_x = event->pos().x();
                mouse_y = mouse_interm_y = event->pos().y();
                updateGL();
                
                
            }else{
                
                cout << "je n'ai rien touché" << endl;
                bone_selected = false;
                updateGL();
            }
            
        }
    }else if (selectionMode == Edit){
        
        bone_selected = false;
        
        //il faut que l'on puisse déplacer des poignées sans que le mesh ne soit déformer ou rajouter des poignées
        //on fait l'hypothèse qu'une poignée est nécessairement sur la surface du mesh.
        
        if ( event->button() == Qt::LeftButton){
            //cout << "je suis bouton gauche" << endl;
            //je fais le déplacement des handles
            
            //je regarde si un bone ou handle a été sélectionné
            std::map<int, std::pair<int, Vec3Df> > intersectionList;
            bool intersection = computeBonesIntersected(event->pos(), intersectionList);
            
            if (intersection){
                //il faut choisir le bone qui a été le plus touché par les différents rayons lancés.
                //on change la direction pour pouvoir retrouver ensuite quelle direction a touché le bone parmis toutes les directions de l'angle solide.
                int max = 0;
                for (map<int, pair<int, Vec3Df> >::iterator it = intersectionList.begin(); it != intersectionList.end(); it++){
                    //le deuxième int correspond au nombre de fois où le bone a été touché par les rayons.
                    if ( (*it).second.first > max){
                        direction = (*it).second.second;
                    }
                }
                
                cout << "BONE here !!! " << endl;
                bone_selected = true;
                mouse_x = mouse_interm_x = event->pos().x();
                mouse_y = mouse_interm_y = event->pos().y();
                updateGL(); //pour colorer le bone sélectionné
                
                
            }else{
                cout << "je n'ai rien touché" << endl;
                updateGL(); // pour décolorer un éventuel ancien bone sélectionné !
            }
            
            
        }else{
            //cout << "je suis bouton droit" << endl;
            //je fais l'ajout des handles dans le mesh
            
            bool found;
            QPoint pixel = QPoint(event->pos().x(), event->pos().y());
            qglviewer::Vec vec = camera()->pointUnderPixel(pixel, found);
            
            if (found){
                
                //ajout du handle dans le mesh
                Vertex vert = Vertex(Vec3Df(vec[0], vec[1], vec[2]) );
                object.getMesh().addHandle(vert, influenceArea);
                updateGL();
            }else{
                cout << " impossible de rajouter un handle sur le mesh " << endl;
            }

        }
        
    }
    
}

void GLViewer::mouseMoveEvent(QMouseEvent *event){
    
    if (selectionMode == Standard){
        QGLViewer::mouseMoveEvent(event);
        
    }else if (selectionMode == Select){
        
        if (bone_selected){
            //on va commencer à déplacer le bone, on prend le plan dans lequel se trouve le bone
            //on définit le plan : le point d'intersection du bone et le vecteur normal qui est celui de la caméra !
            
            //la translation selon x et y vaut :
            float dx = -(event->pos().x() - mouse_interm_x);
            float dy = -(event->pos().y() - mouse_interm_y);
            
            dy /= camera()->screenHeight()*0.2;
            dx /= camera()->screenWidth()*0.2;
            
            //on réactualise les valeurs de mouse_x, mouse_y
            mouse_interm_x = event->pos().x();
            mouse_interm_y = event->pos().y();
            
            //on déplace de dx et dy dans le plan les deux vertex du bones !
            //on définit le plan
            Ray ray = Ray(origin, direction);
            int idx_bone;
            Vec3Df intersectionPoint;
            object.getBoneSelected(ray, idx_bone, intersectionPoint);
            
            //pour déplacer le bone, je le déplace seulement dans le plan de vue de la caméra
            qglviewer::Vec dir= camera()->viewDirection();
            Vec3Df dire = Vec3Df(dir[0], dir[1], dir[2]);
            //Vec3Df x,y;
            //dire.getTwoOrthogonals(x, y);
            qglviewer::Vec xcam = - camera()->rightVector();
            qglviewer::Vec ycam = camera()->upVector();
            Vec3Df x = Vec3Df(xcam[0], xcam[1], xcam[2]);
            Vec3Df y = Vec3Df(ycam[0], ycam[1], ycam[2]);
            
            //on déplace uniquement le sommet du bone mais pas sa boundingBox, donc il est toujours repéré au même endroit par le rayon origin et dir
            //on changera sa boundingbox une seule fois dans le mouserelease, alors dir et origin ne correspondront plus à ce bone mais c'est pas grave car on est obligé de recliquer sur mousepress pour sélectionner un bone et alors dir et origin seront mis à jour !
            object.getMesh().modifyBone(idx_bone, x*dx, y*dy);
            updateGL();
            
        }
    }else{
        //on est dans le mode Edit
        
        if (bone_selected){
            //alors un bone a été sélectionné et on a cliqué avec le clic gauche -> déplacmeent des handles
            
            //la translation selon x et y vaut :
            float dx = -(event->pos().x() - mouse_interm_x);
            float dy = -(event->pos().y() - mouse_interm_y);
            
            dy /= camera()->screenHeight()*0.2;
            dx /= camera()->screenWidth()*0.2;
            
            //on réactualise les valeurs de mouse_x, mouse_y
            mouse_interm_x = event->pos().x();
            mouse_interm_y = event->pos().y();
            
            //on déplace de dx et dy dans le plan les deux vertex du bones !
            //on définit le plan
            Ray ray = Ray(origin, direction);
            int idx_bone;
            Vec3Df intersectionPoint;
            object.getBoneSelected(ray, idx_bone, intersectionPoint);
            
            //pour déplacer le bone, je le déplace seulement dans le plan de vue de la caméra
            qglviewer::Vec dir= camera()->viewDirection();
            Vec3Df dire = Vec3Df(dir[0], dir[1], dir[2]);
            //Vec3Df x,y;
            //dire.getTwoOrthogonals(x, y);
            qglviewer::Vec xcam = - camera()->rightVector();
            qglviewer::Vec ycam = camera()->upVector();
            Vec3Df x = Vec3Df(xcam[0], xcam[1], xcam[2]);
            Vec3Df y = Vec3Df(ycam[0], ycam[1], ycam[2]);
            object.getMesh().modifyBone(idx_bone, x*dx, y*dy);
            updateGL();
            
        }
        
    }
    
}

void GLViewer::mouseReleaseEvent(QMouseEvent *event){
    
    //quand on relache le bouton de gauche, on ne sélectionne plus le bone si on est en mode select
    // on modifie le mesh pour select mode mais on ne modifie pas le mesh sinon
    if (selectionMode == Select){
        
        if (bone_selected){
            
            if ( event->button() == Qt::LeftButton){
                
                bone_selected = false;
                //la translation selon x et y vaut :
                float dx = -(event->pos().x() - mouse_x);
                float dy = -(event->pos().y() - mouse_y);
                
                dy /= camera()->screenHeight()*0.2;
                dx /= camera()->screenWidth()*0.2;
                
                //on déplace de dx et dy dans le plan les deux vertex du bones !
                //on définit le plan
                Ray ray = Ray(origin, direction);
                int idx_bone;
                Vec3Df intersectionPoint;
                object.getBoneSelected(ray, idx_bone, intersectionPoint);
                
                qglviewer::Vec dir= camera()->viewDirection();
                Vec3Df dire = Vec3Df(dir[0], dir[1], dir[2]);
                //Vec3Df x,y;
                //dire.getTwoOrthogonals(x, y);
                qglviewer::Vec xcam = - camera()->rightVector();
                qglviewer::Vec ycam = camera()->upVector();
                Vec3Df x = Vec3Df(xcam[0], xcam[1], xcam[2]);
                Vec3Df y = Vec3Df(ycam[0], ycam[1], ycam[2]);
                
                //le bone est déjà déplace avec le mousemove mais il faut que j'actualise la boundingbox
                object.getMesh().modifyBone(idx_bone, Vec3Df(0,0,0), Vec3Df(0,0,0), true);
                object.getMesh().modifyMesh(idx_bone, x*dx, y*dy);
                updateGL();
            }
        }
    }else if (selectionMode == Edit){
        
        if (bone_selected){
            
            if ( event->button() == Qt::LeftButton){
                
                //dans le cas où je suis dans le mode edit et je déplace les bones.
                //Quand j'ai fini de déplacer les bones, il faut que je mette à jour sa boundingBox
                Ray ray = Ray(origin, direction);
                int idx_bone;
                Vec3Df intersectionPoint;
                object.getBoneSelected(ray, idx_bone, intersectionPoint);
                object.getMesh().modifyBone(idx_bone, Vec3Df(0,0,0), Vec3Df(0,0,0), true);
                updateGL();
            }
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
    //pour colorier le bone sélectionné
    if (bone_selected){
        
        //on obtient l'index du bone sélectionné
        Ray ray = Ray(origin, direction);
        int idx_bone;
        Vec3Df intersectionPoint;
        object.getBoneSelected(ray, idx_bone, intersectionPoint);
        
        //puis on le dessine en coloré
        object.getMesh().renderGL(boneVisualisation, influenceArea, renderingMode == Flat, idx_bone);
        
        
    }else{
        //pas de bone sélectionné
        object.getMesh().renderGL(boneVisualisation, influenceArea, renderingMode == Flat, -1);
    }
    
    
    //calcul de la depth map la première fois
    if (depth_map){
        cv::Mat depthMap(camera()->screenHeight(),camera()->screenWidth(), CV_32FC1);
        
        for (unsigned int i=0; i< camera()->screenHeight(); i++){
            
            for (unsigned int j = 0; j< camera()->screenWidth(); j++){
                //on inverse pour que ca soit dans le bon sens...
                QPoint pixel = QPoint(j,i);
                bool found;
                qglviewer::Vec vec = camera()->pointUnderPixel(pixel, found);
                if (found){
                    depthMap.at<float>(i,j) = -(vec[2] - camera()->position()[2]);
                }else{
                    depthMap.at<float>(i,j) = MAXFLOAT;
                }
            }
        }
        
        double min, max;
        cv::minMaxIdx(depthMap, &min, &max);
        
        for (unsigned int i = 0; i< depthMap.size().height; i++){
            for (unsigned int j = 0; j< depthMap.size().width; j++){
                if (depthMap.at<float>(i,j) == MAXFLOAT){
                    depthMap.at<float>(i,j) = min;
                }
            }
        }
        
        cv::normalize(depthMap, depthMap, 0, 1, CV_MINMAX);
        //cv::Mat adjMap;
        //cv::convertScaleAbs(depthMap, adjMap, 255/max);
        cv::imshow("out", depthMap);
        
        depth_map = false;
    }
    
    glPopMatrix();
        

}

bool GLViewer::computeBonesIntersected(QPoint pos, std::map<int, std::pair<int, Vec3Df> > &intersectionList){
    
    //récupération du rayon passant par la caméra vers le pixel de la souris.
    qglviewer::Vec orig, dir;
    camera()->convertClickToLine(pos, orig, dir);
    origin = Vec3Df(orig[0], orig[1], orig[2]);
    direction = Vec3Df(dir[0], dir[1], dir[2]);
    direction.normalize();
    
    //on lance nb_rays rayons pour les boundingbox
    bool intersection = false;
    Vec3Df x, y;
    direction.getTwoOrthogonals(x, y);
    int nb_rays = 10; // dans chaque direction
    float angle = 0.7; // en degré !
    
    for (unsigned int i = 0; i< nb_rays ; i++){
        
        float phi = 2 * M_PI/nb_rays * i;
        
        for (unsigned int j = 0; j< nb_rays ; j++){
            
            float teta = angle * j / nb_rays;
            
            Vec3Df newDirection = direction * cos(teta * M_PI / 180) + sin(teta * M_PI /180)*cos(phi)* x + sin(teta * M_PI / 180) * sin(phi) * y;
            
            Ray ray = Ray(origin, newDirection);
            int idx_bone;
            Vec3Df intersectionPoint;
            bool intersect = object.getBoneSelected(ray, idx_bone, intersectionPoint);
            
            if (intersect){
                
                intersection = true;
                map< int, pair< int, Vec3Df> >::iterator it;
                it = intersectionList.find(idx_bone);
                
                if (it == intersectionList.end()){
                    //l'idx_bone n'est pas présent dans la map
                    intersectionList[idx_bone] = pair<int, Vec3Df> (1, newDirection);
                }else{
                    //l'idx est déjà présent dans la map et on augmente le nombre de fois que le bone a été touché
                    pair<int, Vec3Df > bone_select = (*it).second;
                    (*it).second = pair<int, Vec3Df >(bone_select.first + 1 , bone_select.second);
                    
                }
            }
        }
    }
    
    return intersection;

    
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
    object.getMesh().renderGL(boneVisualisation, influenceArea, renderingMode == Flat);
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
