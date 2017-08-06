/* =============================================================================
 * scenemapper.cxx
 * Masado Ishii
 * v0.1 2016-12-26
 * v0.2 2017-08-04 - New scene content.
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
    // Constants.
    const float d45 = atan(1);  // PI/4.
    const float d360 = 8*d45;   // 2*PI.

    /* ----------------------------------------------------
     * Meshes (model space).
     * ----------------------------------------------------
     */

    shapes = glGenLists(4);

    //
    // unitSquare (display list): White square with vertices at (+-1, +-1, 0).
    //
    GLuint unitSquare = shapes+0;
    glNewList(unitSquare, GL_COMPILE);
    glBegin(GL_QUADS);
        glColor3f(1.0f, 1.0f, 1.0f);
        glVertex3f(1, 1, 0);
        glVertex3f(-1, 1, 0);
        glVertex3f(-1, -1, 0);
        glVertex3f(1, -1, 0);
    glEnd();
    glEndList();

    //
    // windowFrame (display list): Gray frame around unitSquare, width = 0.1.
    //
    float w = 0.1f;
        /* Corner coordinates in quadrants 1, 2, 3, 4, 1. */
    float wfInnerX[5] = {1.0f, -1.0f, -1.0f, 1.0f, 1.0f};
    float wfInnerY[5] = {1.0f, 1.0f, -1.0f, -1.0f, 1.0f};
    float wfOuterX[5] = {1.0f +w, -1.0f -w, -1.0f -w, 1.0f +w, 1.0f +w};
    float wfOuterY[5] = {1.0f +w, 1.0f +w, -1.0f -w, -1.0f -w, 1.0f +w};
    //
    GLuint windowFrame = shapes+1;
    glNewList(windowFrame, GL_COMPILE);
    glBegin(GL_QUADS);
        glColor3f(0.7f, 0.7f, 0.7f);
        for (int q= 0; q< 4; q++)
        {
            /*     C --------------- B
             *       \             /
             *      D ------------- A
             */
            glVertex3f(wfInnerX[q], wfInnerY[q], 0.0f);
            glVertex3f(wfOuterX[q], wfOuterY[q], 0.0f);
            glVertex3f(wfOuterX[q+1], wfOuterY[q+1], 0.0f);
            glVertex3f(wfInnerX[q+1], wfInnerY[q+1], 0.0f);
        }
    glEnd();
    glEndList();

    //
    // octahedron (display list): Yellow and blue octahedron with vertices at +-i, +-j, +-k.
    //
    float octahedronRim[4*3] =    // CCW if looking down +X.
    {
        0, -1, 0,
        0, 0, -1,
        0, 1, 0,
        0, 0, 1
    };
    float octahedronFar[3] = {1, 0, 0};
    float octahedronNear[3] = {-1, 0, 0};
    //
    GLuint octahedron = shapes+2;
    glNewList(octahedron, GL_COMPILE);
    glBegin(GL_TRIANGLES);
        // +X (Far): Yellow
        glColor3f(0.8f, 0.8f, 0.0f);
        float *rimVertPrev = octahedronRim + 3*3;
        float *rimVert = octahedronRim;
        while (rimVert < octahedronRim + 4*3)
        {
            glVertex3fv(rimVertPrev);     // Order important if single-sided lighting.
            glVertex3fv(rimVert);         //
            glVertex3fv(octahedronFar);   //
            rimVertPrev = rimVert;
            rimVert += 3;
        }
        // -X (Near): Blue
        glColor3f(0.0f, 0.0f, 0.5f);
        rimVertPrev = octahedronRim + 3*3;
        rimVert = octahedronRim;
        while (rimVert < octahedronRim + 4*3)
        {
            glVertex3fv(octahedronNear);  // Order important if single-sided lighting.
            glVertex3fv(rimVert);         //
            glVertex3fv(rimVertPrev);     //
            rimVertPrev = rimVert;
            rimVert += 3;
        }
    glEnd();
    glEndList();
    
    //
    // cone (display list): Red right-cone with unit-circle base in XY, height=2*r in Z.
    //
    float cone_radius = 1;
    float cone_height = 2;
    int cone_num_subdiv = 8;
    //
    float cone_subdiv_angle = d360 / cone_num_subdiv;
    float cone_angle_acc = 0.0f;
    GLuint cone = shapes+3;
    glNewList(cone, GL_COMPILE);
    glBegin(GL_TRIANGLE_FAN);
        glColor3f(0.6f, 0.1f, 0.1f);
        glVertex3f(0.0f, 0.0f, cone_height);              // Apex.
        glVertex3f(cone_radius, 0.0f, 0.0f);              // First vertex.
        for (int i = 1; i < cone_num_subdiv; i++)         // All but last vertex.
        {
            cone_angle_acc += cone_subdiv_angle;
            glVertex3f(cone_radius*cos(cone_angle_acc),
                    cone_radius*sin(cone_angle_acc), 0.0f);
        }
        glVertex3f(cone_radius, 0.0f, 0.0f);              // Last vertex should be first vertex.
    glEnd();
    glEndList();

    // Wrap all display lists into 'meshes' and register all meshes.
    DisplayListMesh *mesh_square= new DisplayListMesh(unitSquare);
    DisplayListMesh *mesh_windowFrame = new DisplayListMesh(windowFrame);
    DisplayListMesh *mesh_octahedron = new DisplayListMesh(octahedron);
    DisplayListMesh *mesh_cone = new DisplayListMesh(cone);
    meshes.push_back(mesh_square);
    meshes.push_back(mesh_windowFrame);
    meshes.push_back(mesh_octahedron);
    meshes.push_back(mesh_cone);


    /* ----------------------------------------------------
     * Scene (world space).
     * ----------------------------------------------------
     */

    // Scene contains a ground plane, an octahedron, a cone, and two framed portals.

    // To make glm matrix expressions succinct.
    using namespace glm_mishii_matrix_transforms;

    // Ground.
    MeshObject *mobj_ground = new MeshObject(mesh_square, scale(mat4(), vec3(20.0f, 20.0f, 1.0f)));

    // Octahedron.
    MeshObject *mobj_octahedron = new MeshObject(mesh_octahedron,
            translate(mat4(), vec3(-3.0f, 6.0f, 2.0f))
            * scale(mat4(), vec3(2.0f, 2.0f, 2.0f)));

    // Cone.
    MeshObject *mobj_cone = new MeshObject(mesh_cone,
            translate(mat4(), vec3(3.0f, -6.0f, 0.0f))
            * scale(mat4(), vec3(2.0f, 2.0f, 2.0f)));

    //
    // Portals & frames.
    //
    mat4 Transform1 = translate(mat4(), vec3(-9.0f, 6.0f, 4.0f))
            * rotate(mat4(), 2*d45, vec3(0.0f, 1.0f, 0.0f))
            * scale(mat4(), vec3(4.0f, 4.0f, 1.0f));
    mat4 Transform2 = translate(mat4(), vec3(9.0f, -6.0f, 4.0f))
            * rotate(mat4(), -2*d45, vec3(0.0f, 1.0f, 0.0f))
            * scale(mat4(), vec3(4.0f, 4.0f, 1.0f));

    // Portals.
    PortalObject *mobj_portal1 = new PortalObject(mesh_square, &meshObjects,
            NULL, Transform1);
    PortalObject *mobj_portal2 = new PortalObject(mesh_square, &meshObjects,
            NULL, Transform2);
    assert( mobj_portal1->SetDestPortal(mobj_portal2) );
    assert( mobj_portal2->SetDestPortal(mobj_portal1) );

    // Frames around portals.
    MeshObject* mobj_frame1 = new MeshObject(mesh_windowFrame, Transform1);
    MeshObject* mobj_frame2 = new MeshObject(mesh_windowFrame, Transform2);

    // Register all objects in the scene.
    meshObjects.push_back(mobj_ground);
    meshObjects.push_back(mobj_octahedron);
    meshObjects.push_back(mobj_cone);
    meshObjects.push_back(mobj_portal1);
    meshObjects.push_back(mobj_portal2);
    meshObjects.push_back(mobj_frame1);
    meshObjects.push_back(mobj_frame2);

    // Feed the animator.
    animationTarget = mobj_octahedron;

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
    static float timeIncrement = 0.01;

    using glm::mat4;
    using glm::sin;
    using glm::cos;
    using glm::abs;
    if (animationTarget != NULL)
    {
        float angle = 1.05*timeIncrement;
        float s = sin(angle);
        float c = cos(angle);
        mat4 *modelMat = &(animationTarget->modelMat);

        // Rotate about world Z direction at the target's origin.
        // Matrices are accessed column-major, and column i contains
        //   the world space coordinates of model axis i.
        // Leave column 3 alone, it is the target's origin.
        for (int i = 0; i <= 2; i++)
        {
            float a = (*modelMat)[i][0];
            float b = (*modelMat)[i][1];
            (*modelMat)[i][0] = a*c - b*s;
            (*modelMat)[i][1] = a*s + b*c;
        }

        //Old animation...
        //animationTarget->modelMat[3][2] = 3.0 + 2.0*animTime;
    }

    animTime += timeIncrement;
    if (abs(animTime) > 0.995)
        timeIncrement = -timeIncrement;
}
