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

void Mesh::renderGL (bool flat) const {
    
    glColor3f(1.0, 1.0, 1.0);
    glLoadName(7);
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
    glLineWidth(2);
    glLoadName(3);
    
    /*glBegin(GL_TRIANGLES);
    
    for (unsigned int i =0; i< bones.size() ; i++){
        
        const Bone & b = bones [i];
        vector<Vec3Df> vert;
        vector<Triangle> tri;
        
        const Vec3Df v0 = vertices_bones[b.getVertex(0)].getPos();
        const Vec3Df v1 = vertices_bones[b.getVertex(1)].getPos();
        
        makeCube(v0, v1, vert, tri);
        //appeler une fonction makeCube qui à partir d'un 2 vec3Df créer un cube centrée sur cette ligne
        
        for (unsigned int j=0; j< tri.size(); j++){
            const Triangle & t = tri[j];
            Vec3Df v[3];
            for (unsigned int k = 0; k < 3; k++)
                v[k] = vert[t.getVertex(k)];
            for (unsigned int k = 0; k < 3; k++)
                glVertexVec3Df(v[k]);
        }
        
    }
    glEnd();*/
    
    glBegin (GL_LINES);
    for (unsigned int i=0; i< bones.size(); i++){
        
        const Bone & b = bones[i];
        Vertex v[2];
        for (unsigned int j=0; j<2; j++)
            v[j] = vertices_bones[b.getVertex(j)];
        
        for (unsigned int j=0; j<2; j++){
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
                unsigned int index1 = stof(line[1])-1-vertices.size();
                unsigned int index2 = stof(line[2])-1-vertices.size();
                Bone bone = Bone(index1, index2);
                bone.buildBox(vertices_bones[index1], vertices_bones[index2]);
                bones.push_back(bone);
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
    
    /*cout << "je vais essayer de tester eigen" << endl;
    Eigen::MatrixXd m(2,2);
    m(0,0) = 3;
    m(1,0) = 2.5;
    m(0,1) = -1;
    m(1,1) = m(1,0) + m(0,1);
    std::cout << m << std::endl;*/
    
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

void Mesh::modifyMesh(const int & idx_bone, const Vec3Df & x_displacement, const Vec3Df & y_displacement){
    
    // modification de la position du bone
    /*Vertex vert0 = vertices_bones[bones[idx_bone].getVertex(0)];
    Vertex vert1 = vertices_bones[bones[idx_bone].getVertex(1)];
    
    Vertex new0 = Vertex(vert0.getPos() + x_displacement + y_displacement);
    Vertex new1 = Vertex( vert1.getPos() + x_displacement + y_displacement);

    setBoneVertices(bones[idx_bone].getVertex(0), new0);
    setBoneVertices(bones[idx_bone].getVertex(1), new1);*/
    
    // modification du mesh
    
    //calcul du poids des différents bones pour chaque vertex du mesh
    
    std::vector <Eigen::VectorXf> w;
    computeWeights(w);
    
    //modification de la position des différents vertices du mesh selon LBS.
    // pas de sommes des contributions des différents bones car on ne modifie qu'un bone à la fois pour l'instant
    
    for (unsigned int i = 0; i< vertices.size() ; i++){
        
        if (w[idx_bone][i] != 0){
            vertices[i].getPos();
            Vertex vert = Vertex( w[idx_bone][i] * vertices[i].getPos() + x_displacement + y_displacement);
            setMeshVertices(i, vert);
        }
    }
    
    recomputeSmoothVertexNormals(0);
    
}

void Mesh::modifyBone(const int & idx_bone, const Vec3Df & x_displacement, const Vec3Df & y_displacement){
    
    // modification de la position du bone
    Vertex vert0 = vertices_bones[bones[idx_bone].getVertex(0)];
    Vertex vert1 = vertices_bones[bones[idx_bone].getVertex(1)];
    
    Vertex new0 = Vertex(vert0.getPos() + x_displacement + y_displacement);
    Vertex new1 = Vertex( vert1.getPos() + x_displacement + y_displacement);
    
    setBoneVertices(bones[idx_bone].getVertex(0), new0);
    setBoneVertices(bones[idx_bone].getVertex(1), new1);

}

void Mesh::computeWeights(std::vector < Eigen::VectorXf> & w){
    
    //on calcule la matrice Laplacienne (cf article Discrete Laplace-Beltrami Operators for Shape Analysis and Segmentation pour savoir comment faire)
    
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
            
            //avant sans les sparsematrix...
            //W( t.getVertex(j), t.getVertex( (j+1)%3 ) ) += 1/2 * cotan(angle);
            //V( t.getVertex(j), t.getVertex(j) ) += 1/2* (cotan(angle) + cotan(angle2));
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
                
                float distance = Vec3Df::distance(vertices_bones[ bones[j].getVertex(k)].getPos(), vertices[i].getPos());
                
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
        
        /*for (unsigned int j = 0; j< vertices_bones.size(); j++){
            
            //on vérifie si le segment est inclu dans l'intérieur du mesh
            //à réfléchir si OK de transférer la bounding bo d'un objet avec la bounding box d'un mesh !
            Vec3Df segment = vertices_bones[j].getPos() - vertices[i].getPos();
            
            float distance = Vec3Df::distance(vertices_bones[j].getPos(), vertices[i].getPos());
            if (distance < dist_min){
                dist_min = distance;
                nb_bones = 1;
            }
            //si la distance est atteinte à plusieurs endroits, il faut les comptabiliser.
            if (distance == dist_min){
                nb_bones++;
            }
            
        }*/
        
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

    //std::vector < Eigen::VectorXf> w;
    
    for (unsigned int i = 0; i< bones.size() ; i++){
        
        Eigen::VectorXf wi(vertices.size()), b(vertices.size());
        A = -L + H;
        b = H * p[i];
        
        /*Eigen::SparseLU< Eigen::SparseMatrix<float> > solver;
        //solver.compute(A);
        solver.analyzePattern(A);
        solver.factorize(A);*/
        
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
