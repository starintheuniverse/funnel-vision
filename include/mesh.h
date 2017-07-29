
/* =============================================================================
 * mesh.h
 * Masado Ishii
 * v0.1 2016-12-26
 *
 * Description: Implementations of drawables.
 *
 * Attributions:
 * =============================================================================
 */

// Note: This file uses the GL api, but nothing from VTK.

#ifndef _MESH_H
#define _MESH_H

#include <GL/gl.h>

/* ------------------------------------------------------------------
 * Mesh class.
 * ------------------------------------------------------------------
 */
class Mesh
{
  public:
    virtual void Draw() = 0;
};


/* ----------------
 * PolygonMesh TBA.
 * ----------------
 */


/* ------------------------------------------------------------------
 * DisplayListMesh class.
 *
 * Wrapper around a OpenGL display list reference.
 * ------------------------------------------------------------------
 */
class DisplayListMesh : public Mesh
{
  protected:
    GLuint displayList;
  public:
    DisplayListMesh(GLuint displayList) : displayList(displayList) {}
    virtual ~DisplayListMesh() {}
    void Draw() { glCallList(displayList); }
};


#endif /*_MESH_H */
