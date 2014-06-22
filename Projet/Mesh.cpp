// ---------------------------------------------------------
// Mesh Class
// Author : Tamy Boubekeur (boubek@gmail.com).
// Copyright (C) 2008 Tamy Boubekeur.
// All rights reserved.
// ---------------------------------------------------------

#include "Mesh.h"
#include <algorithm>
#include <iostream>
#include <fstream>
#include <sstream>
#include <OpenGL/gl.h>

#include <opencv.hpp>

using namespace std;

static inline float cotan(float i)
{
    return 1/tan(i);
}

void Mesh::clear () {
    clearTopology ();
    clearGeometry ();
}

void Mesh::clearGeometry () {
    vertices.clear ();
    vertices_bones.clear();
}

void Mesh::clearTopology () {
    triangles.clear ();
    bones.clear();
}

void Mesh::unmarkAllVertices () {
    for (unsigned int i = 0; i < vertices.size (); i++)
        vertices[i].unmark ();
    for (unsigned int i=0; i< vertices_bones.size(); i++)
        vertices_bones[i].unmark();
}

void Mesh::computeTriangleNormals (vector<Vec3Df> & triangleNormals) {
    for (vector<Triangle>::const_iterator it = triangles.begin ();
         it != triangles.end ();
         it++) {
        Vec3Df e01 (vertices[it->getVertex (1)].getPos () - vertices[it->getVertex (0)].getPos ());
        Vec3Df e02 (vertices[it->getVertex (2)].getPos () - vertices[it->getVertex (0)].getPos ());
        Vec3Df n (Vec3Df::crossProduct (e01, e02));
        n.normalize ();
        triangleNormals.push_back (n);
    }
}

void Mesh::recomputeSmoothVertexNormals (unsigned int normWeight) {
    vector<Vec3Df> triangleNormals;
    computeTriangleNormals (triangleNormals);
    for (std::vector<Vertex>::iterator it = vertices.begin (); it != vertices.end (); it++)
        it->setNormal (Vec3Df (0.0, 0.0, 0.0));
    vector<Vec3Df>::const_iterator itNormal = triangleNormals.begin ();
    vector<Triangle>::const_iterator it = triangles.begin ();
    for ( ; it != triangles.end (); it++, itNormal++) 
        for (unsigned int  j = 0; j < 3; j++) {
            Vertex & vj = vertices[it->getVertex (j)];
            float w = 1.0; // uniform weights
            Vec3Df e0 = vertices[it->getVertex ((j+1)%3)].getPos () - vj.getPos ();
            Vec3Df e1 = vertices[it->getVertex ((j+2)%3)].getPos () - vj.getPos ();
            if (normWeight == 1) { // area weight
                w = Vec3Df::crossProduct (e0, e1).getLength () / 2.0;
            } else if (normWeight == 2) { // angle weight
                e0.normalize ();
                e1.normalize ();
                w = (2.0 - (Vec3Df::dotProduct (e0, e1) + 1.0)) / 2.0;
            } 
            if (w <= 0.0)
                continue;
            vj.setNormal (vj.getNormal () + (*itNormal) * w);
        }
    Vertex::normalizeNormals (vertices);
}

void Mesh::collectOneRing (vector<vector<unsigned int> > & oneRing) const {
    oneRing.resize (vertices.size ());
    for (unsigned int i = 0; i < triangles.size (); i++) {
        const Triangle & ti = triangles[i];
        for (unsigned int j = 0; j < 3; j++) {
            unsigned int vj = ti.getVertex (j);
            for (unsigned int k = 1; k < 3; k++) {
                unsigned int vk = ti.getVertex ((j+k)%3);
                if (find (oneRing[vj].begin (), oneRing[vj].end (), vk) == oneRing[vj].end ())
                    oneRing[vj].push_back (vk);
            }
        }
    }
}

void Mesh::collectOrderedOneRing (vector<vector<unsigned int> > & oneRing) const {
    oneRing.resize (vertices.size ());
    for (unsigned int t = 0; t < triangles.size (); t++) {
        const Triangle & ti = triangles[t];
        for (unsigned int i = 0; i < 3; i++) {
            unsigned int vi = ti.getVertex (i);
            unsigned int vj = ti.getVertex ((i+1)%3);
            unsigned int vk = ti.getVertex ((i+2)%3);
            vector<unsigned int> & oneRingVi = oneRing[vi];
            vector<unsigned int>::iterator begin = oneRingVi.begin ();
            vector<unsigned int>::iterator end = oneRingVi.end ();
            vector<unsigned int>::iterator nj = find (begin, end, vj);
            vector<unsigned int>::iterator nk = find (begin, end, vk);
            if (nj != end && nk == end) {
                if (nj == begin)
                    nj = end;
                nj--;
                oneRingVi.insert (nj, vk);
            } else if (nj == end && nk != end) 
                oneRingVi.insert (nk, vj);
            else if (nj == end && nk == end) {
                oneRingVi.push_back (vk);
                oneRingVi.push_back (vj);
            }
        }
    }
}

void Mesh::computeDualEdgeMap (EdgeMapIndex & dualVMap1, EdgeMapIndex & dualVMap2) {
    for (vector<Triangle>::iterator it = triangles.begin ();
         it != triangles.end (); it++) {
        for (unsigned int i = 0; i < 3; i++) {
            Edge eij (it->getVertex (i), it->getVertex ((i+1)%3)); 
            if (dualVMap1.find (eij) == dualVMap1.end ())
                dualVMap1[eij] = it->getVertex ((i+2)%3);
            else
                dualVMap2[eij] = it->getVertex ((i+2)%3);
        }
    } 
}

void Mesh::markBorderEdges (EdgeMapIndex & edgeMap) {
    for (vector<Triangle>::iterator it = triangles.begin ();
         it != triangles.end (); it++) {
        for (unsigned int i = 0; i < 3; i++) {
            unsigned int j = (i+1)%3;
            Edge eij (it->getVertex (i), it->getVertex (j)); 
            if (edgeMap.find (eij) == edgeMap.end ())
                edgeMap[eij] = 0;
            else 
                edgeMap[eij] += 1;
        }
    } 
}

inline void glVertexVec3Df (const Vec3Df & v) {
    glVertex3f (v[0], v[1], v[2]);
}

inline void glNormalVec3Df (const Vec3Df & n) {
    glNormal3f (n[0], n[1], n[2]);
}
 
inline void glDrawPoint (const Vec3Df & pos, const Vec3Df & normal) {
    glNormalVec3Df (normal);
    glVertexVec3Df (pos);
}

inline void glDrawPoint (const Vertex & v) { 
    glDrawPoint (v.getPos (), v.getNormal ()); 
}

void Mesh::renderGL (bool boneVisu, bool area, bool flat, int idx_bone) const {
    
    glColor3ub(232, 183, 155);
    //glLoadName(7); plus besoin car je n'utilise plus le picking d'openGL.
    
    //si on est en area et qu'on a sélectionné un bone, alors on monte sa zone d'influence !
    //seulement si on voit les bones !
    if (boneVisu && area && idx_bone != -1){
        
        glBegin(GL_TRIANGLES);
        
        for (unsigned int i = 0; i<triangles.size(); i++){
            const Triangle & t = triangles[i];
            Vertex v[3];
            for (unsigned int j = 0; j < 3; j++)
                v[j] = vertices[t.getVertex(j)];
            if (flat) {
                Vec3Df normal = Vec3Df::crossProduct (v[1].getPos () - v[0].getPos (),
                                                      v[2].getPos () - v[0].getPos ());
                normal.normalize ();
                glNormalVec3Df (normal);
            }
            for (unsigned int j = 0; j < 3; j++){
                if (weights[idx_bone](t.getVertex(j)) >0){
                    glColor3f(1.0, 0.0, 0.0);
                }else{
                    glColor3ub(232, 183, 155);
                }
                if (!flat)
                    glDrawPoint (v[j]);
                else
                    glVertexVec3Df (v[j].getPos ());
            }
                
        }
        glEnd ();

        
    }else{
        //on ne montre pas la zone d'influence des bones, donc on affiche le mesh classique
        glBegin (GL_TRIANGLES);
        for (unsigned int i = 0; i < triangles.size (); i++) {
            const Triangle & t = triangles[i];
            Vertex v[3];
            for (unsigned int j = 0; j < 3; j++)
                v[j] = vertices[t.getVertex(j)];
            if (flat) {
                Vec3Df normal = Vec3Df::crossProduct (v[1].getPos () - v[0].getPos (),
                                                      v[2].getPos () - v[0].getPos ());
                normal.normalize ();
                glNormalVec3Df (normal);
            }
            for (unsigned int j = 0; j < 3; j++)
                if (!flat)
                    glDrawPoint (v[j]);
                else
                    glVertexVec3Df (v[j].getPos ());
        }
        glEnd ();
    }
    
    //seulement si on veut voir les bones !
    if (boneVisu){
        
        //dessiner le skelette
        glColor3f(0.0, 0.0, 1.0);
        glLineWidth(2);
        glBegin (GL_LINES);
        
        for (unsigned int i=0; i< bones.size(); i++){
            //on dessine seulement les bones non sélectionnés
            if (i != idx_bone){
                Armature* b = bones[i];
                //on dessine une ligne seulement si c'est un bone !
                if (b->getType() == "bone"){
                    Vertex v[2];
                    for (unsigned int j=0; j<2; j++)
                        v[j] = vertices_bones[b->getVertex(j)];
                    
                    for (unsigned int j=0; j<2; j++){
                        glVertexVec3Df (v[j].getPos ());
                    }
                }
            }
            
        }
        glEnd();
        
        for (unsigned int i =0; i<bones.size(); i++){
            if (i != idx_bone){
                
                Armature * h = bones[i];
                //on dessine le point seulement si c'est une handle
                if (h->getType() == "handle"){
                    Vertex v = vertices_bones[h->getVertex()];
                    this->drawSphere(5, 5, v.getPos());
                    //this->drawBoundingBox(i);
                }
            }
        }
        
        //ancienne version, les points pas bons car leur taille est défini selon la taille écran !
        /*glPointSize(7);
         glBegin(GL_POINTS);
         
         for (unsigned int i = 0; i< bones.size(); i++){
         if (i != idx_bone){
         Armature * h = bones[i];
         //on dessine le point seulement si c'est une handle
         if (h->getType() == "handle"){
         Vertex v = vertices_bones[h->getVertex()];
         glVertexVec3Df (v.getPos());
         }
         }
         
         }
         glEnd();*/
        
        
        // si idx_bone !=-1, on a sélectionné un bone et on le colorie d'une couleur différente
        if(idx_bone != -1){
            
            if (bones[idx_bone]->getType() == "bone"){
                //c'est un bone donc une ligne
                glColor3f(1., 0., 0.);
                glLineWidth(2);
                glBegin (GL_LINES);
                
                Vertex v[2];
                for (unsigned int j=0; j<2; j++)
                    v[j] = vertices_bones[bones[idx_bone]->getVertex(j)];
                
                for (unsigned int j=0; j<2; j++){
                    glVertexVec3Df (v[j].getPos ());
                }
                glEnd();
                
            }else{
                //c'est un handle donc un point
                glColor3f(1., 0., 0.);
                Vertex v = vertices_bones[bones[idx_bone]->getVertex()];
                this->drawSphere(5, 5, v.getPos());
                //this->drawBoundingBox(idx_bone);
                
                /*glPointSize(7);
                 glBegin (GL_POINTS);
                 Vertex v = vertices_bones[bones[idx_bone]->getVertex()];
                 glVertexVec3Df (v.getPos());
                 glEnd();*/
            }
        }

    }
        
}

void Mesh::drawSphere(unsigned int resU, unsigned int resV, Vec3Df pos) const{
    
    std::vector<Vertex> V(resU * (resV-2) + 2);
    std::vector<Triangle> T(resU * (resV-2)*2 + 2*resU);
    
    // resU pas en theta
    // resV pas en phi
    int count = 0;
    float x,y,z,theta,phi;
    // en coordonnées sphériques, avec r = 1;
    // on ajoute à la main le pole nord
    V[count].setPos(Vec3Df(0,1,0));
    count++;
    for(unsigned int i = 0; i < resU; i++)
    {
        theta = 2*M_PI*i/resU;
        //cout << "theta = " << theta << endl;
        for(unsigned int j = 1 ; j < (resV-1) ; j++)
        {
            phi = M_PI*j/(resV-1);
            //cout << "Phi " << phi << endl;
            x = sin(phi)*sin(theta);
            y = cos(phi);
            z = sin(phi)*cos(theta);
            
            V[count].setPos(Vec3Df(x,y,z));
            count ++;
        }
    }
    // on ajoute à la main le pole sud
    int max = count;
    V[count].setPos(Vec3Df(0,-1,0));
    
    //construction voisinnage pole nord
    count = 0;
    for (unsigned int i = 0; i < resU; i++)
    {
        T[i].setVertex(0, 0 %(resU*(resV-2)));
        T[i].setVertex(1, (count + 1)%(resU*(resV-2)));
        T[i].setVertex(2, (count + 1 + resV-2)%(resU*(resV-2)));
        
        count += resV-2;
    }
    
    //on décompose les quadrilatere logiquement obtenu par la construction des points de la sphere
    // en 2 triangles.
    for(unsigned int j = 0 ; j < resU ; j++)//resU ppour valeur finale j
    {
        for (unsigned int i = 1; i < resV-2; i++)
        {
            if(j != resU-1){
                //premier triangle du carré
                T[2*(i-1)+2*j*(resV-3)+resU].setVertex(0, (i + j*(resV-2)));
                T[2*(i-1)+2*j*(resV-3)+resU].setVertex(1, (i + j*(resV-2)+ 1));
                T[2*(i-1)+2*j*(resV-3)+resU].setVertex(2, ( i + j*(resV-2)+ 1 + resV-2));
                
                //2eme triangle du carré
                T[2*(i-1)+1+2*j*(resV-3)+resU].setVertex(0, (i + j*(resV-2)+ 1+ (resV-2)));
                T[2*(i-1)+1+2*j*(resV-3)+resU].setVertex(1, (i + j*(resV-2) + (resV-2)));
                T[2*(i-1)+1+2*j*(resV-3)+resU].setVertex(2, (i + j*(resV-2)));
                
            }
            //On fait attention au raccordement entre derniers indices et premiers indices
            else{
                //premier triangle du carré
                T[2*(i-1)+2*j*(resV-3)+resU].setVertex(0, (i + j*(resV-2)));
                T[2*(i-1)+2*j*(resV-3)+resU].setVertex(1, (i + j*(resV-2)+ 1));
                T[2*(i-1)+2*j*(resV-3)+resU].setVertex(2, ( i +1));
                
                //2eme triangle du carré
                T[2*(i-1)+1+2*j*(resV-3)+resU].setVertex(0, ( i +1));
                T[2*(i-1)+1+2*j*(resV-3)+resU].setVertex(1, i);
                T[2*(i-1)+1+2*j*(resV-3)+resU].setVertex(2, (i + j*(resV-2)));
                
            }
        }
    }
    
    //construction voisinnage pole sud
    count = 0;
    for (unsigned int i = T.size()-resU; i < T.size(); i++)
    {
        T[i].setVertex(0, max);
        T[i].setVertex(1, (count + resV-2)%(resU*(resV-2)) + resV-2);
        T[i].setVertex(2, (count)%(resU*(resV-2)) + resV-2);
        
        count += resV-2;
    }
    
    Mesh mesh = Mesh(V, T);
    mesh.recomputeSmoothVertexNormals(0);
    mesh.centerToCandScaleToF(pos, 0.3);
    
    //je dessine ensuite ces vertices et triangles
    //glTranslatef(pos[0], pos[1], pos[2]);
    glBegin (GL_TRIANGLES);
    for (unsigned int i = 0; i < mesh.getTriangles().size(); i++) {
        const Triangle & t = mesh.getTriangles()[i];
        Vertex v[3];
        for (unsigned int j = 0; j < 3; j++)
            v[j] = mesh.getVertices()[t.getVertex(j)];
        for (unsigned int j = 0; j < 3; j++)
            glDrawPoint (v[j]);
    }
    glEnd ();
    //glPopMatrix();
    
}

void Mesh::centerToCandScaleToF(Vec3Df c, float f){
    Vec3Df center;
    for (unsigned int i = 0; i< vertices.size(); i++){
        center +=vertices[i].getPos();
    }
    center /= vertices.size();
    
    float max = 0;
    for (unsigned int i =0; i< vertices.size(); i++){
        float dist = Vec3Df::distance(vertices[i].getPos(), center);
        if ( dist > max){
            max = dist;
        }
    }
    
    for (unsigned int i =0; i< vertices.size(); i++){
        Vec3Df pos = vertices[i].getPos();
        vertices[i].setPos( (pos -center)/max * f + c);
    }
    
}

void Mesh::loadOFF (const std::string & filename) {
    clear ();
    ifstream input (filename.c_str ());
    if (!input)
        throw Exception ("Failing opening the file.");
    string magic_word;
    input >> magic_word;
    if (magic_word != "OFF")
        throw Exception ("Not an OFF file.");
    unsigned int numOfVertices, numOfTriangles, numOfWhat;
    input >> numOfVertices >> numOfTriangles >> numOfWhat;
    for (unsigned int i = 0; i < numOfVertices; i++) {
        Vec3Df pos;
        Vec3Df col;
        input >> pos;
        vertices.push_back (Vertex (pos, Vec3Df (1.0, 0.0, 0.0)));
    }
    for (unsigned int i = 0; i < numOfTriangles; i++) {
        unsigned int polygonSize;
        input >> polygonSize;
        vector<unsigned int> index (polygonSize);
        for (unsigned int j = 0; j < polygonSize; j++)
            input >> index[j];
        for (unsigned int j = 1; j < (polygonSize - 1); j++)
            triangles.push_back (Triangle (index[0], index[j], index[j+1]));
    }
    input.close ();
    recomputeSmoothVertexNormals (0);
}

void Mesh::loadOBJ(const std::string &filename) {
    
    clear();
    ifstream input (filename.c_str ());
    if (!input)
        throw Exception ("Failing opening the file.");

    if (filename.find("obj") == 0)
        throw Exception ("Not an OBJ file");

    string word;
    std::getline(input, word);
    //permet de savoir si on est dans un objet ou dans un skelette. (si objet -> true)
    bool obj;
    
    while ( word.length() != 0){

        istringstream iss(word);
        
        vector<string> line{istream_iterator<string>{iss},
            istream_iterator<string>{}};
        
        //avant de remplir les vertices, on regarde si on a un skelette ou un objet.
        if (line[0] == "o"){
            obj = true;
            std::getline(input, word);
            line.clear();
        }else if (line [0] == "s") {
           obj = false;
            std::getline(input, word);
            line.clear();
            
        }else if (word.find("#") != std::string::npos){
            std::getline(input, word);
            line.clear();
            
        }else if ( line[0] == "v"){
            //je lis une ligne de vertex v
            //il faut récupérer les 3 positions
            
            //si c'est un objet on remplir les vertices du mesh
            //sinon, on remplit les vertices du skelette
            if (obj){
                Vec3Df pos( atof(line[1].c_str()) , atof(line[2].c_str()), atof(line[3].c_str()) );
                vertices.push_back (Vertex (pos, Vec3Df (1.0, 0.0, 0.0)));
           }else{
                Vec3Df pos( atof(line[1].c_str()) , atof(line[2].c_str()), atof(line[3].c_str()) );
                vertices_bones.push_back( Vertex(pos, Vec3Df (1.0, 0.0, 0.0)) );
            }
            
            std::getline(input, word);
            
        }else if( line[0] == "f"){
            //je lis une ligne de face
            unsigned int polygonSize = line.size();
            
            if (polygonSize == 4){
                //c'est un triangle
                //on enlève 1 car dans un .obj, l'index des vertices commencent à 1 et non 0 !
                triangles.push_back (Triangle (atof(line[1].c_str())-1, atof(line[2].c_str())-1, atof(line[3].c_str())-1));
            }
            std::getline(input, word);
            
        }else if (line[0] == "l"){
            // c'est un bone
            
            if (line.size() == 3){
                // c'est une ligne
                //on enlève 1 car dans un .obj, l'index des vertices commencent à 1 et non 0
                //on enlève la taille des vertices précédents !
                unsigned int index1 = atof(line[1].c_str())-1-vertices.size();
                unsigned int index2 = atof(line[2].c_str())-1-vertices.size();
                Bone* bone = new Bone(index1, index2);
                bone->buildBox(vertices_bones[index1], vertices_bones[index2]);
                bones.push_back(bone);
            }
            std::getline(input, word);
            
        }else if (line[0] == "lv"){
            
            // c'est un handle
            if (line.size() == 2){
                // c'est un handle
                unsigned int index = atof(line[1].c_str())-1-vertices.size();
                Handle* handle = new Handle(index);
                handle->buildBox(vertices_bones[index]);
                bones.push_back(handle);
            }
            
            std::getline(input, word);
            
        }else{
            
            std::getline(input, word);
            
        }
        
    }
    
    input.close();
    recomputeSmoothVertexNormals (0);
    
}

void Mesh::rotateAroundZ(float angle)
{
    vector<Vertex> v;
    for (int i = 0; i < vertices.size(); i++) {
        Vec3Df newV;
        Vec3Df V = vertices[i].getPos();
        newV[0] = cos(angle) * V[0] + sin(angle) * V[1];
        newV[1] = -sin(angle) * V[0] + cos(angle) * V[1];
        newV[2] = V[2];
        vertices[i].setPos(newV);
    }
    for (int i = 0; i < vertices_bones.size(); i++) {
        Vec3Df newV;
        Vec3Df V = vertices_bones[i].getPos();
        newV[0] = cos(angle) * V[0] + sin(angle) * V[1];
        newV[1] = -sin(angle) * V[0] + cos(angle) * V[1];
        newV[2] = V[2];
        vertices_bones[i].setPos(newV);
    }
    recomputeSmoothVertexNormals(0);
}

void Mesh::rotateAroundY(float angle)
{
    vector<Vertex> v;
    for (int i = 0; i < vertices.size(); i++) {
        Vec3Df newV;
        Vec3Df V = vertices[i].getPos();
        newV[0] = cos(angle) * V[0] + sin(angle) * V[2];
        newV[1] = V[1];
        newV[2] = -sin(angle) * V[0] + cos(angle) * V[2];
        vertices[i].setPos(newV);
    }
    for (int i = 0; i < vertices_bones.size(); i++) {
        Vec3Df newV;
        Vec3Df V = vertices_bones[i].getPos();
        newV[0] = cos(angle) * V[0] + sin(angle) * V[2];
        newV[1] = V[1];
        newV[2] = -sin(angle) * V[0] + cos(angle) * V[2];
        vertices_bones[i].setPos(newV);
    }
    recomputeSmoothVertexNormals(0);
}

void Mesh::rotateAroundX(float angle)
{
    vector<Vertex> v;
    for (int i = 0; i < vertices.size(); i++) {
        Vec3Df newV;
        Vec3Df V = vertices[i].getPos();
        newV[0] = V[0];
        newV[1] = cos(angle) * V[1] + sin(angle) * V[2];
        newV[2] = -sin(angle) * V[1] + cos(angle) * V[2];
        vertices[i].setPos(newV);
    }
    for (int i = 0; i < vertices_bones.size(); i++) {
        Vec3Df newV;
        Vec3Df V = vertices_bones[i].getPos();
        newV[0] = V[0];
        newV[1] = cos(angle) * V[1] + sin(angle) * V[2];
        newV[2] = -sin(angle) * V[1] + cos(angle) * V[2];
        vertices_bones[i].setPos(newV);
    }
    recomputeSmoothVertexNormals(0);
    
}

void Mesh::makeCube(const Vec3Df & v0, const Vec3Df & v1, vector<Vec3Df> & vert, vector<Triangle> & tri) const{
    
    float width;// = 30;
    //vecteurs x, y, z qui vont donner l'orientation
    Vec3Df x;
    if (v0[0] < v1[0]){
        x = v1-v0;
        width = v1[0] - v0[0];
    }else{
        x = v0 - v1;
        width = v0[0] - v1[0];
    }
    Vec3Df y,z;
    x.getTwoOrthogonals(y, z);
    
    //vertices
    //front face
    vert.push_back(v0 + width*z - width*y);
    vert.push_back(v0 + width*z + width*y);
    vert.push_back(v1 + width*z + width*y);
    vert.push_back(v1 + width*z - width*y);
    
    //back face
    vert.push_back(v0 - width*z - width*y);
    vert.push_back(v0 - width*z + width*y);
    vert.push_back(v1 - width*z + width*y);
    vert.push_back(v1 - width*z - width*y);
    
    //triangles
    //font face
    tri.push_back(Triangle(2, 1, 0));
    tri.push_back(Triangle(3,2,0));
    
    //back face
    tri.push_back(Triangle(4, 5, 6));
    tri.push_back(Triangle(4,6,7));
    
    //right face
    tri.push_back(Triangle(7,6,2));
    tri.push_back(Triangle(7,2,3));
    
    //left face
    tri.push_back(Triangle(0,1,4));
    tri.push_back(Triangle(1,5,4));
    
    //top face
    tri.push_back(Triangle(1,2,5));
    tri.push_back(Triangle(6,5,2));
    
    //bottom face
    tri.push_back(Triangle(3,4,0));
    tri.push_back(Triangle(7,4,3));
}

void Mesh::drawBoundingBox(int idx_bone) const {
    BoundingBox box = bones[idx_bone]->getBoundingBox();
    Vec3Df center = box.getCenter();
  
    Vec3Df min = box.getMin();
    Vec3Df max = box.getMax();
    Vec3Df v2(min[0] + box.getLength(), min[1], min[2]);
    Vec3Df v3(min[0] + box.getLength(), min[1] + box.getHeight(), min[2]);
    Vec3Df v4(min[0], min[1] + box.getLength(), min[2]);
    Vec3Df v5(max[0] - box.getLength(), max[1] - box.getHeight(), max[2]);
    Vec3Df v6(max[0], max[1] - box.getHeight(), max[2]);
    Vec3Df v7(max[0] - box.getLength(), max[1], max[2]);
    
    glBegin(GL_QUADS);
        glDrawPoint (min);
        glDrawPoint (v2);
        glDrawPoint (v3);
        glDrawPoint (v4);
    
        glDrawPoint (max);
        glDrawPoint (v7);
        glDrawPoint (v4);
        glDrawPoint (v3);
    
        glDrawPoint (max);
        glDrawPoint (v3);
        glDrawPoint (v2);
        glDrawPoint (v6);
    
        glDrawPoint (v7);
        glDrawPoint (max);
        glDrawPoint (v6);
        glDrawPoint (v5);
    
        glDrawPoint (v2);
        glDrawPoint (min);
        glDrawPoint (v5);
        glDrawPoint (v6);
    
        glDrawPoint (v4);
        glDrawPoint (v7);
        glDrawPoint (v5);
        glDrawPoint (min);
    
    glEnd();
}

void Mesh::modifyMesh(const int & idx_bone, const Vec3Df & x_displacement, const Vec3Df & y_displacement){
    
    // modification du mesh
    //calcul du poids des différents bones pour chaque vertex du mesh
    
    //std::vector <Eigen::VectorXf> w;
    computeWeights(weights);
    
    //modification de la position des différents vertices du mesh selon LBS.
    // pas de sommes des contributions des différents bones car on ne modifie qu'un bone à la fois pour l'instant
    
    for (unsigned int i = 0; i< vertices.size() ; i++){
        
        if (weights[idx_bone][i] != 0){
            vertices[i].getPos();
            Vertex vert = Vertex( weights[idx_bone][i] * vertices[i].getPos() + x_displacement + y_displacement);
            setMeshVertices(i, vert);
        }
    }
    
    recomputeSmoothVertexNormals(0);
    //cout << "j'ai modifié le mesh" << endl;
    
}

void Mesh::modifyBone(const int & idx_bone, const Vec3Df & x_displacement, const Vec3Df & y_displacement, bool end_displacement){
    
    if ( bones[idx_bone]->getType() == "bone"){
        
        // modification de la position du bone
        Vertex vert0 = vertices_bones[bones[idx_bone]->getVertex(0)];
        Vertex vert1 = vertices_bones[bones[idx_bone]->getVertex(1)];
        
        Vertex new0 = Vertex(vert0.getPos() + x_displacement + y_displacement);
        Vertex new1 = Vertex( vert1.getPos() + x_displacement + y_displacement);
        
        setBoneVertices(bones[idx_bone]->getVertex(0), new0);
        setBoneVertices(bones[idx_bone]->getVertex(1), new1);
        
        if(end_displacement){
            dynamic_cast<Bone*>(bones[idx_bone])->buildBox(new0, new1);
        }
        
    }else if ( bones[idx_bone]->getType() == "handle"){
        
        //modification de la position du handle
        Vertex vert0 = vertices_bones[bones[idx_bone]->getVertex()];
        Vertex new0 = Vertex(vert0.getPos() + x_displacement + y_displacement);
        setBoneVertices(bones[idx_bone]->getVertex(), new0);
        
        if (end_displacement){
            dynamic_cast<Handle*>(bones[idx_bone])->buildBox(new0);
        }
        
    }

}

void Mesh::addHandle(Vertex vert, bool influenceArea){
    
    vertices_bones.push_back(vert);
    Handle * handle = new Handle(vertices_bones.size() - 1);
    handle->buildBox(vertices_bones[vertices_bones.size() - 1]);
    bones.push_back(handle);
    
    //si la case des influenceArea est cohé, l'utilisateur peut directement
    //recliqué sur le nouveau handle, il faut donc recalculé les poids
    //si la case influenceArea n'est pas coché, pas besoin car quand elle sera coché le calcul sera effectué !
    if (influenceArea){
        computeWeights(weights);
    }
                                 
}

void Mesh::computeAutoBones(float near, float far){
    
}

void Mesh::suppr(int idx_bone){
    
    //vérifier que les vertices du bone ne sont pas utilisés pour d'autres bones
    bool used = false;
    
    if (bones[idx_bone]->getType() == "bone"){
        
        int idx_vertices = bones[idx_bone]->getVertex(0);
        int idx_vertices1 = bones[idx_bone]->getVertex(1);
        
        for (unsigned int i =0; i< bones.size(); i++){
            
            if (bones[i]->getType() == "bone"){
                //c'est un bone il faut vérifier les deux vertices de ce bone
                if ( bones[i]->getVertex(0) == idx_vertices || bones[i]->getVertex(1) == idx_vertices || bones[i]->getVertex(0) == idx_vertices1 || bones[i]->getVertex(1) == idx_vertices1){
                    used = true;
                }
                
            }else{
                // c'est un handle, il suffit de vérifier le vertex du handle
                if (bones[i]->getVertex(0) == idx_vertices || bones[i]->getVertex(0) == idx_vertices1){
                    used = true;
                }
            }
        }
        
    }else{
        
        int idx_vertices = bones[idx_bone]->getVertex(0);
        
        for (unsigned int i =0; i<bones.size(); i++){
            
            if (bones[i]->getType() == "bone"){
                //c'est un bone il faut vérifier les deux vertices de ce bone
                if (bones[i]->getVertex(0) == idx_vertices || bones[i]->getVertex(1) == idx_vertices){
                    used = true;
                }
                                
            }else{
                //c'est un handle, il suffit de vérifier le vertex du handle
                if (bones[i]->getVertex(0) == idx_vertices){
                    used= true;
                }
            }
        }
    }
    
    if (used){
        //on supprime seulement le bone et pas ses vertices
        delete (bones[idx_bone]);
        vector<Armature * >::iterator it = bones.begin() + idx_bone;
        bones.erase(it);
        
    }else{
        //on supprime le bone et les vertices
        if (bones[idx_bone]->getType() == "bone"){
            int index = bones[idx_bone]->getVertex(0);
            int index2 = bones[idx_bone]->getVertex(1);
            
            std::vector<Vertex>::iterator it = vertices_bones.begin() + index;
            vertices_bones.erase(it);
            it = vertices_bones.begin() + index2;
            vertices_bones.erase(it);
            
            delete bones[idx_bone];
            vector<Armature * >::iterator it2 = bones.begin() + idx_bone;
            bones.erase(it2);
        }else{
            
            //c'est un handle, on supprime qu'un seul vertex
            int index = bones[idx_bone]->getVertex(0);
            
            std::vector<Vertex>::iterator it = vertices_bones.begin() + index;
            vertices_bones.erase(it);
            
            delete bones[idx_bone];
            vector<Armature *>::iterator it2 = bones.begin() + idx_bone;
            bones.erase(it2);
        }
        
    }
    
}

void Mesh::computeWeights(std::vector < Eigen::VectorXf> & w){
    
    //on calcule la matrice Laplacienne (cf article Discrete Laplace-Beltrami Operators for Shape Analysis and Segmentation pour savoir comment faire)
    
    w.clear();
    
    //il faut calculer la matrice W = wij et V
    Eigen::SparseMatrix<float> W(vertices.size(), vertices.size() );
    W.setZero();
    
    Eigen::SparseMatrix<float> V(vertices.size(), vertices.size() );
    V.setZero();
    
    for (unsigned int i=0 ; i< triangles.size() ; i++){
        
        Triangle t = triangles[i];
        
        for (unsigned int j = 0; j < 3 ; j++){
            
            Vec3Df vj = vertices[ t.getVertex(j)].getPos();
            Vec3Df vj1 = vertices[ t.getVertex((j+1)%3)].getPos();
            Vec3Df vj2 = vertices[ t.getVertex((j+2)%3)].getPos();
            
            //ATTENTION ACOS DOIT PRENDRE EN RADIAN !!
            float angle = acos( Vec3Df::dotProduct(vj1-vj2, vj - vj2) / (Vec3Df::distance(vj1, vj2) * Vec3Df::distance(vj, vj2) ) ) * 180 / M_PI;
            float angle2 = acos (Vec3Df::dotProduct( vj2 - vj1, vj - vj1) / (Vec3Df::distance(vj1, vj2) * Vec3Df::distance(vj, vj1) ) ) * 180 / M_PI;
            
            //test angle
//            float angle3 = acos( Vec3Df::dotProduct(vj1 - vj, vj2 - vj) / (Vec3Df::distance(vj1, vj) * Vec3Df::distance(vj, vj2) ) ) * 180 / M_PI;
//            cout << angle + angle2 + angle3 << endl;
            
            
            W.coeffRef(t.getVertex(j), t.getVertex( (j+1)%3)) += 1/2 * cotan(angle);
            V.coeffRef(t.getVertex(j), t.getVertex(j) ) += 1/2 * (cotan(angle) + cotan(angle2));
            
        }
    }
    
    //La matrice Laplacienne est L = D^-1.A
    // pour l'instant je n'ai pas calcule D-1 qui doit être l'aire de la cellule de Voronoi...
    
    Eigen::SparseMatrix<float> L(vertices.size() , vertices.size() );
    L.setZero();
    
    L = V - W;
    
    //on calcule la matrice H diagonale (cf article 2007 Baran and Popovic)
    //et on définit pour chaque vertex, le bone le plus proche !
    Eigen::SparseMatrix<float> H(vertices.size(), vertices.size());
    H.setZero();

    for (unsigned int i = 0; i< vertices.size(); i++){
        
        //on recheche le bone le plus proche du vertex i
        float dist_min = MAXFLOAT;
        int nb_bones = 0;
        
        for (unsigned int j = 0 ; j<bones.size() ; j++){
            for (unsigned int k = 0 ; k<2; k++){
                
                // il faut que je vérifie que le segment est inclu à l'intérieur du mesh !!
                
                float distance = Vec3Df::distance(vertices_bones[ bones[j]->getVertex(k)].getPos(), vertices[i].getPos());
                
                if (distance < dist_min) {
                    dist_min = distance;
                    nb_bones = 1;
                    vertices[i].setBone(j);
                }
                if (distance == dist_min){
                    nb_bones++;
                }
            }
        }
        
        H.insert(i,i) = nb_bones * 1/dist_min;
    }
    
    
    Eigen::SparseMatrix<float> A(vertices.size(), vertices.size());
    //on veut résoudre A wi = b;
    // cette résolution doit se faire pour chaque bone !! i = ith bone !!
    std:vector < Eigen::VectorXf > p;
    
    for (unsigned int i = 0 ; i< bones.size(); i++){
        
        //on calcule pi qui est le vecteur pour le bone i
        Eigen::VectorXf pi(vertices.size());
        
        for (unsigned int j = 0; j< vertices.size(); j++){
            if (vertices[j].getBone() == i){
                pi(j) = 1;
            }else{
                pi(j) = 0;
            }
        }
        
        p.push_back(pi);
        
    }
    
    for (unsigned int i = 0; i< bones.size() ; i++){
        
        Eigen::VectorXf wi(vertices.size()), b(vertices.size());
        A = -L + H;
        b = H * p[i];
            
        Eigen::SimplicialLDLT< Eigen::SparseMatrix<float> > solver;
        solver.compute(A);
        
        if (solver.info() != Eigen::Success) {
            //decomposition failed
            cout << " il y a une erreur dans la résolution du système " << endl;
            cout << "type d'erreur : " << solver.info() << endl;
            return;
        }
        
        wi = solver.solve(b);
        
        w.push_back(wi);
    }
    
    //test des wi - il faut que la somme pour un vertex des wi soit égal à 1 !
    for (unsigned int j = 0; j< vertices.size(); j++){
        
        float sum_test = 0;
        
        for (unsigned int i = 0; i< bones.size() ; i++){
            sum_test += w[i][j];
        }
        
        if (sum_test > 1){
            cout << sum_test << "sum" << endl;
            cout << "error pour wi ! " << endl;
            return;
        }
        
        if (sum_test < 0.99 ) {
            cout << sum_test << "sum" << endl;
            cout << "error pour wi ! " << endl;
            return;
        }
    }
    

}
