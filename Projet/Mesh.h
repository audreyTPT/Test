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

#include "Vertex.h"
#include "Triangle.h"
#include "Edge.h"
#include "Bone.h"

class Mesh {
public:
    inline Mesh () {} 
    inline Mesh (const std::vector<Vertex> & v) 
        : vertices (v) {}
    inline Mesh (const std::vector<Vertex> & v, 
                 const std::vector<Triangle> & t) 
        : vertices (v), triangles (t)  {}
    inline Mesh (const Mesh & mesh) 
        : vertices (mesh.vertices), 
          triangles (mesh.triangles), vertices_bones(mesh.vertices_bones), bones(mesh.bones) {}
        
    inline virtual ~Mesh () {}
    std::vector<Vertex> & getVertices () { return vertices; }
    const std::vector<Vertex> & getVertices () const { return vertices; }
    std::vector<Triangle> & getTriangles () { return triangles; }
    const std::vector<Triangle> & getTriangles () const { return triangles; }
    std::vector<Bone> & getBones() { return bones; }
    const std::vector<Bone> & getBones() const { return bones; }
    std::vector<Vertex> & getBonesVertices() { return vertices_bones; }
    const std::vector<Vertex> & getBonesVertices() const { return vertices_bones; }
    void setVertices(unsigned int i, Vertex vert) { vertices_bones[i] = vert; }
    
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
    
    void renderGL (bool flat) const;
    void makeCube (const Vec3Df & v0, const Vec3Df & v1, std::vector<Vec3Df> & vert, std::vector<Triangle> & tri) const;
    void modifyMesh(const Bone & bone, const Vec3Df & x_displacement, const Vec3Df & y_displacement);
    
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
    std::vector<Bone> bones;
};

#endif // MESH_H

// Some Emacs-Hints -- please don't remove:
//
//  Local Variables:
//  mode:C++
//  tab-width:4
//  End:
