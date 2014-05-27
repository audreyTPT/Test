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
#include <Eigen/Dense>

using namespace std;


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

void Mesh::renderGL (bool flat) const {
    
    glColor3f(1.0, 1.0, 1.0);
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
    
    //dessiner le skelette
    glColor3f(1.0, 0.0, 0.0);
    glLineWidth(15);
    glBegin (GL_LINES);
    for (unsigned int i=0; i< bones.size(); i++){
        
        const Bone & b = bones[i];
        Vertex v[2];
        for (unsigned int j=0; j<2; j++)
            v[j] = vertices_bones[b.getVertex(j)];
        
        for (unsigned int j=0; j<2; j++){
            //glDrawPoint (v[j]);
            glVertexVec3Df (v[j].getPos ());
        }
        
    }
    glEnd();
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
                Vec3Df pos( stof(line[1]) , stof(line[2]), stof(line[3]) );
                vertices.push_back (Vertex (pos, Vec3Df (1.0, 0.0, 0.0)));
           }else{
                Vec3Df pos( stof(line[1]) , stof(line[2]), stof(line[3]) );
                vertices_bones.push_back( Vertex(pos, Vec3Df (1.0, 0.0, 0.0)) );
            }
            
            std::getline(input, word);
            
        }else if( line[0] == "f"){
            //je lis une ligne de face
            unsigned int polygonSize = line.size();
            
            if (polygonSize == 4){
                //c'est un triangle
                //on enlève 1 car dans un .obj, l'index des vertices commencent à 1 et non 0 !
                triangles.push_back (Triangle (stof(line[1])-1, stof(line[2])-1, stof(line[3])-1));
            }
            std::getline(input, word);
            
        }else if (line[0] == "l"){
            // c'est un bone
            
            if (line.size() == 3){
                // c'est une ligne
                //on enlève 1 car dans un .obj, l'index des vertices commencent à 1 et non 0
                //on enlève la taille des vertices précédents !
                bones.push_back(Bone (stof(line[1])-1-vertices.size(), stof(line[2])-1-vertices.size()));
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
    
    cout << "je vais essayer de tester eigen" << endl;
    Eigen::MatrixXd m(2,2);
    m(0,0) = 3;
    m(1,0) = 2.5;
    m(0,1) = -1;
    m(1,1) = m(1,0) + m(0,1);
    std::cout << m << std::endl;
    
    
}
