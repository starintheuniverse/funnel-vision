/* =============================================================================
 * scenemapper.cxx
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

#include "vtkActor.h"         // Used in vtk441MapperMishii::RenderPiece
#include "vtkRenderer.h"      //
#include "vtkRenderWindow.h"  //

#include "vtkObjectFactory.h"  // For vtkStandardNewMacro( )

#include "mesh.h"        // For populating the scene.
#include "meshobject.h"  //


#include "../include/scenemapper.h"


/* --------------------------------------------------------------------
 * vtk441Mapper member functions.
 * --------------------------------------------------------------------
 */

/*
 * AdvanceAnimation()
 */
void vtk441Mapper::AdvanceAnimation()
{
   animTime += 0.001;
   if (animTime >= 2.0)
       animTime = 0.0;

   //DEBUG
   //std::cerr << animTime << std::endl;
}


/*
 * RemoveVTKOpenGLStateSideEffects()
 */
void vtk441Mapper::RemoveVTKOpenGLStateSideEffects()
{
    float Info[4] = { 0, 0, 0, 1 };
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, Info);
    float ambient[4] = { 1,1, 1, 1.0 };
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, ambient);
    float diffuse[4] = { 1, 1, 1, 1.0 };
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, diffuse);
    float specular[4] = { 1, 1, 1, 1.0 };
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, specular);
}


/*
 * SetupLight()
 */
void vtk441Mapper::SetupLight(void)
{
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    GLfloat diffuse0[4] = { 0.8, 0.8, 0.8, 1 };
    GLfloat ambient0[4] = { 0.2, 0.2, 0.2, 1 };
    GLfloat specular0[4] = { 0.0, 0.0, 0.0, 1 };
    GLfloat pos0[4] = { 1, 2, 3, 0 };
    glLightfv(GL_LIGHT0, GL_POSITION, pos0);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse0);
    glLightfv(GL_LIGHT0, GL_AMBIENT, ambient0);
    glLightfv(GL_LIGHT0, GL_SPECULAR, specular0);
    glDisable(GL_LIGHT1);
    glDisable(GL_LIGHT2);
    glDisable(GL_LIGHT3);
    glDisable(GL_LIGHT5);
    glDisable(GL_LIGHT6);
    glDisable(GL_LIGHT7);
}


/* --------------------------------------------------------------------
 * vtk441MapperMishii member functions.
 * --------------------------------------------------------------------
 */

/*
 * vtkStandardNewMacro( ) - Defines New()
 */
vtkStandardNewMacro(vtk441MapperMishii);

/*
 * Destructor.
 */
vtk441MapperMishii::~vtk441MapperMishii()
{
    for (std::list<MeshObject *>::iterator iter = meshObjects.begin();
            iter != meshObjects.end();
            ++iter)
        delete *iter;
    for (std::list<DisplayListMesh *>::iterator iter = meshes.begin();
            iter != meshes.end();
            ++iter)
        delete *iter;
}

/*
 * InitializeScene()
 */
void vtk441MapperMishii::InitializeScene()
{

    shapes = glGenLists(2);

    // unitSquare (display list): Square with vertices at (+-1, +-1, 0).
    GLuint unitSquare = shapes+0;
    glNewList(unitSquare, GL_COMPILE);
    glBegin(GL_QUADS);
        glVertex3f(1, 1, 0);
        glVertex3f(-1, 1, 0);
        glVertex3f(-1, -1, 0);
        glVertex3f(1, -1, 0);
    glEnd();
    glEndList();

    // Colors for cube faces.
    float colors[] = {0.6f, 0.0f, 0.6f,
                      0.0f, 1.0f, 1.0f,
                      1.0f, 0.0f, 0.0f,
                      0.0f, 0.0f, 1.0f,
                      0.0f, 0.45f, 0.0f,
                      0.95f, 0.95f, 0.0f};

    // unitCube (display list): Cube with vertices at (+-1, +-1, +-1).
    GLuint unitCube = shapes+1;
    glNewList(unitCube, GL_COMPILE);
    glPushMatrix();
      // Facing +X, -Z, -X, +Z.
      for (int i = 0; i < 4; i++)
      {
          glRotatef(90, 0, 1, 0);
          glPushMatrix();
            glTranslatef(0, 0, 1);
            glColor3fv(colors + 3*i);
            glCallList(unitSquare);
          glPopMatrix();
      }
      // Facing -Y.
      glRotatef(90, 1, 0, 0);
      glPushMatrix();
        glTranslatef(0, 0, 1);
        glColor3fv(colors + 3*4);
        glCallList(unitSquare);
      glPopMatrix();
      // Facing +Y.
      glRotatef(180, 1, 0, 0);
      glTranslatef(0, 0, 1);
      glColor3fv(colors + 3*5);
      glCallList(unitSquare);
    glPopMatrix();
    glEndList();

    // Reidentify the above shapes as "display list meshes".
    DisplayListMesh *squareMesh = new DisplayListMesh(unitSquare);
    DisplayListMesh *cubeMesh = new DisplayListMesh(unitCube);
    meshes.push_back(squareMesh);
    meshes.push_back(cubeMesh);

    // To make glm matrix expressions succinct.
    using namespace glm_mishii_matrix_transforms;

    // A scene with a ground plane, a floating cube, and a portal.
    MeshObject *squareObj = new MeshObject(squareMesh, scale(mat4(), vec3(20.0f, 20.0f, 1.0f)));
    MeshObject *cubeObj = new MeshObject(cubeMesh,
            translate(mat4(), vec3(-2.0f, -3.0f, 5.0f))
            * scale(mat4(), vec3(2.0f, 2.0f, 2.0f)));

    // Construct portals.
    float d45 = atan(1);
    PortalObject *portal1 = new PortalObject(squareMesh, &meshObjects, NULL,
            translate(mat4(), vec3(5.0f, 2.0f, 3.0f))
            * rotate(mat4(), d45, vec3(1.0f, 0.0f, 0.0f))
            * scale(mat4(), vec3(3.0f, 3.0f, 1.0f)));
    PortalObject *portal2 = new PortalObject(squareMesh, &meshObjects, NULL,
            translate(mat4(), vec3(-5.5f, -3.0f, 5.0f))
            * rotate(mat4(), 2*d45, vec3(0.0f, 1.0f, 0.0f))
            * rotate(mat4(), 2*d45, vec3(0.0f, 0.0f, 1.0f))
            * scale(mat4(), vec3(3.0f, 3.0f, 1.0f)));
    assert( portal1->SetDestPortal(portal2) );
    assert( portal2->SetDestPortal(portal1) );

    // A list of objects which represents the scene.
    meshObjects.push_back(squareObj);
    meshObjects.push_back(cubeObj);
    meshObjects.push_back(portal1);
    meshObjects.push_back(portal2);
    animationTarget = cubeObj;

    // This function has done its job.
    initialized = true;
}

/*
 * RenderPiece()
 */
void vtk441MapperMishii::RenderPiece(vtkRenderer *ren, vtkActor *act)
{
    RemoveVTKOpenGLStateSideEffects();
    SetupLight();

    glEnable(GL_COLOR_MATERIAL);
    glEnable(GL_CULL_FACE);  // This is not the correct way to implement single-sided portals.
                             // Single-sided portals should be configured using glStencilOpSeparate().
    glEnable(GL_STENCIL_TEST);  // Needed for portal boundaries.
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);  // Needed to empty portal viewport background.

    // Initialize the stencil buffer. This is the outermost level of portal recursion.
    // Also initialize the color buffer to black.
    glClearStencil(255);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    //glClear(GL_STENCIL_BUFFER_BIT);
    glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Initialize the stencil test.
    glStencilFunc(GL_GEQUAL, 255, 0xFF);        // Outermost ref value.
    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);  // Default update action.
    glStencilMask(0x0);                         // By default, read-only.

    if (!initialized)
        InitializeScene();

    MeshObject::DrawList(meshObjects);
}

/*
 * AdvanceAnimation()
 */
void vtk441MapperMishii::AdvanceAnimation()
{
    animTime += 0.01;
    if (animTime >= 3.0)
        animTime = 0.0;
    if (animationTarget != NULL)
        animationTarget->modelMat[3][2] = 3.0 + 2.0*animTime;
}
