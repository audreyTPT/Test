// ---------------------------------------------------------
// Mesh Class
// Author : Tamy Boubekeur (boubek@gmail.com).
// Copyright (C) 2008 Tamy Boubekeur.
// All rights reserved.
// ---------------------------------------------------------

#ifndef MESH_H
#define MESH_H

#include <vector>
#include <string>
#include <Eigen/Dense>
#include <Eigen/Sparse>

#include "Vertex.h"
#include "Triangle.h"
#include "Edge.h"
#include "Bone.h"
#include "Handle.h"

class Mesh {
public:
    inline Mesh () {}
    inline Mesh (const std::vector<Vertex> & v) 
    : vertices (v) {}
    inline Mesh (const std::vector<Vertex> & v,
                 const std::vector<Triangle> & t) 
    : vertices (v), triangles (t)  { }
    inline Mesh (const Mesh & mesh)
        : vertices (mesh.vertices), 
    triangles (mesh.triangles), vertices_bones(mesh.vertices_bones), bones(mesh.bones) { }
    
    inline virtual ~Mesh () {}
    inline std::vector<Vertex> & getVertices () { return vertices; }
    inline const std::vector<Vertex> & getVertices () const { return vertices; }
    inline std::vector<Triangle> & getTriangles () { return triangles; }
    inline const std::vector<Triangle> & getTriangles () const { return triangles; }
    inline std::vector<Armature *> & getBones() { return bones; }
    inline const std::vector<Armature *> & getBones() const { return bones; }
    inline std::vector<Vertex> & getBonesVertices() { return vertices_bones; }
    inline const std::vector<Vertex> & getBonesVertices() const { return vertices_bones; }
    inline void setBoneVertices(unsigned int i, Vertex vert) { vertices_bones[i] = vert; }
    inline void setMeshVertices(unsigned int i, Vertex vert) { vertices[i] = vert; }
    inline void initWeights() { computeWeights(weights); }
    
    void clear ();
    void clearGeometry ();
    void clearTopology ();
    void unmarkAllVertices ();
    void recomputeSmoothVertexNormals (unsigned int weight);
    void computeTriangleNormals (std::vector<Vec3Df> & triangleNormals);  
    void collectOneRing (std::vector<std::vector<unsigned int> > & oneRing) const;
    void collectOrderedOneRing (std::vector<std::vector<unsigned int> > & oneRing) const;
    void computeDualEdgeMap (EdgeMapIndex & dualVMap1, EdgeMapIndex & dualVMap2);
    void markBorderEdges (EdgeMapIndex & edgeMap);
    
    void renderGL (bool boneVisu, bool area, bool flat, int idx_bones = -1) const;
    void makeCube (const Vec3Df & v0, const Vec3Df & v1, std::vector<Vec3Df> & vert, std::vector<Triangle> & tri) const;
    void drawSphere(unsigned int resU, unsigned int resV, Vec3Df pos) const;
    void drawBoundingBox(int idx_bone) const ;
    void centerToCandScaleToF(Vec3Df c, float f);
    
    void modifyMesh(const int & idx_bone, const Vec3Df & x_displacement, const Vec3Df & y_displacement);
    void modifyBone(const int & idx_bone, const Vec3Df & x_displacement, const Vec3Df & y_displacement, bool end_displacement = 0);
    void computeWeights(std::vector < Eigen::VectorXf> & w);
    void addHandle(Vertex vert, bool influenceArea);
    void suppr(int idx_bone);
    
    void loadOFF (const std::string & filename);
    void loadOBJ (const std::string & filename);
    void rotateAroundX(float angle);
    void rotateAroundY(float angle);
    void rotateAroundZ(float angle);
  
    class Exception {
    private: 
        std::string msg;
    public:
        Exception (const std::string & msg) : msg ("[Mesh Exception]" + msg) {}
        virtual ~Exception () {}
        inline const std::string & getMessage () const { return msg; }
    };

private:
    std::vector<Vertex> vertices;
    std::vector<Triangle> triangles;
    std::vector<Vertex> vertices_bones;
    std::vector<Armature * > bones; // car c'est une classe abstraite
    std::vector <Eigen::VectorXf> weights;
    
};

#endif // MESH_H

// Some Emacs-Hints -- please don't remove:
//
//  Local Variables:
//  mode:C++
//  tab-width:4
//  End:
