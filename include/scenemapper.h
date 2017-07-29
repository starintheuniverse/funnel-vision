/* =============================================================================
 * scenemapper.h
 * Masado Ishii
 * v0.1 2016-12-26
 *
 * Description: Initialization of the GL environment; definitions of scene 
 *   and animations.
 *
 * Attributions:
 *   > Modeled on code by Hank Childs, presumably derivative
 *     of Kitware coders K.Martin, W.Schroeder, and B.Lorensen.
 * =============================================================================
 */

#ifndef _SCENEMAPPER_H
#define _SCENEMAPPER_H

#include "vtkOpenGLPolyDataMapper.h"  // Inherit mapper from this.

#include "mesh.h"        // For populating the scene.
#include "meshobject.h"  //


/* ------------------------------------------------------------------
 * vtk441Mapper class.
 *
 * Part of the hook into the VTK harness. Set up GL states.
 * ------------------------------------------------------------------
 */
class vtk441Mapper : public vtkOpenGLPolyDataMapper
{
  protected:
   GLuint displayList;
   bool   initialized;
   float  animTime;

  public:
   vtk441Mapper() : initialized(false), animTime(0.0) {}
    
   virtual void AdvanceAnimation();

   void RemoveVTKOpenGLStateSideEffects();
   void SetupLight(void);
};



/* ------------------------------------------------------------------
 * vtk441MapperMishii class.
 * ------------------------------------------------------------------
 */
class vtk441MapperMishii : public vtk441Mapper
{
  protected:
    GLuint shapes; // Display lists.
    bool   initialized;

    std::list<DisplayListMesh *> meshes;
    std::list<MeshObject *> meshObjects;

    MeshObject *animationTarget;

  public:
    static vtk441MapperMishii *New();

    vtk441MapperMishii() : initialized(false), animationTarget(NULL) {}
   ~vtk441MapperMishii();

  protected:
    void InitializeScene();

  public:
    virtual void RenderPiece(vtkRenderer *ren, vtkActor *act);
    virtual void AdvanceAnimation();
};

#endif /* _SCENEMAPPER_H */
