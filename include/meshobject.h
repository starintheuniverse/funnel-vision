/* =============================================================================
 * meshobject.h
 * Masado Ishii
 * v0.1 2016-12-26
 *
 * Description: Base types for drawable objects having spatial transform.
 *
 * Attributions:
 * =============================================================================
 */

#ifndef _MESHOBJECT_H
#define _MESHOBJECT_H

#include <list>

#include "mesh.h"
#include "utility.h"


/* ------------------------------------------------------------------
 * MeshObjList type.
 * ------------------------------------------------------------------
 */
class MeshObject;
typedef std::list<MeshObject *> MeshObjList;


/* ------------------------------------------------------------------
 * MeshObject class.
 * ------------------------------------------------------------------
 */
class MeshObject
{
  public:
    glm::mat4 modelMat;
    Mesh *mesh;

    MeshObject(Mesh *mesh, glm::mat4 modelMat = glm::mat4(1.0))
            : mesh(mesh), modelMat(modelMat) {}
    virtual ~MeshObject() {};
    virtual void Draw() const;
    //void Draw() const;   // DEBUG hack: switch these to enable/disable portals.
    
    static void DrawList(MeshObjList &l);
};


/* ------------------------------------------------------------------
 * PortalObject class.
 *
 * Rays go into the +Z side and come out of the +Z side.
 * ------------------------------------------------------------------
 */
class PortalObject : public MeshObject
{
  protected:
    // An internal mechanism to count and limit the portal recursion depth.
    static const int MAX_PORTAL_RECURSION_DEPTH = 2;
    static int currentPortalRecursionDepth;  // Must be class-level so that child implementations of Draw() can access.
             // Initialization of currentPortalRecursionDepth is in class implementation.
    static PortalObject *oldDestPortal;
             // Initialization of oldDestPortal is in class implementation.

  public:
    MeshObjList *parentScene;
    PortalObject *destPortal;

    /* Constructor */
    PortalObject(Mesh *mesh, MeshObjList *scene, PortalObject *portal = NULL,
            glm::mat4 modelMat = glm::mat4(1.0))
            : MeshObject(mesh, modelMat), parentScene(scene), destPortal(portal) {}

    /* Establishes one direction of the portal link.
     * Currently no way to check that two portals are pointed at each other.
     * DOES check both portals use the same mesh (so that shape is consistent).
     * DOES check non-NULL portal.
     * Returns true if portal link was set, false otherwise.
     */
    bool SetDestPortal(PortalObject *portal);
    
    /* Overrides MeshObject::Draw, rendering scene from perspective of the
     *   destination portal.
     * Will render through additional portals on the other side if visible,
     *   up to MAX_PORTAL_RECURSION_DEPTH.
     */
    virtual void Draw() const;
};


#endif /* _MESHOBJECT_H */
