
/* =============================================================================
 * funnel.cxx
 * Masado Ishii
 * CIS 441 "Intro Comp. Graphics" (H.Childs)
 * 2016-12-26
 *
 * Description:
 *   My final project, "FunnelVision," to simulate portal-like visual effects
 *   using the OpenGL and VTK frameworks.
 *
 * Attributions:
 *   > Interactor and mapper derived from examples by Hank Childs.
 *   > glm usage derived from the glm docs at <glm.g-truc.net>.
 *   > Other source code used as provided by Hank Childs, presumably derivative
 *     of Kitware coders K.Martin, W.Schroeder, and B.Lorensen.
 * =============================================================================
 */


/*=========================================================================
  Kitware Copyright Notice:

  Program:   Visualization Toolkit
  Module:    SpecularSpheres.cxx

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

#include "vtkSmartPointer.h"
#include "vtkSphereSource.h"
#include "vtkPolyDataMapper.h"
#include "vtkCallbackCommand.h"
#include "vtkCommand.h"
#include "vtkActor.h"
#include "vtkInteractorStyle.h"
#include "vtkObjectFactory.h"
#include "vtkRenderer.h"
#include "vtkRenderWindow.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkProperty.h"
#include "vtkCamera.h"
#include "vtkLight.h"
#include "vtkOpenGLPolyDataMapper.h"
#include "vtkJPEGReader.h"
#include "vtkImageData.h"

#include <vtkPolyData.h>
#include <vtkPointData.h>
#include <vtkPolyDataReader.h>
#include <vtkPoints.h>
#include <vtkUnsignedCharArray.h>
#include <vtkFloatArray.h>
#include <vtkDoubleArray.h>
#include <vtkCellArray.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <cassert>
#include <list>


class vtk441Mapper;


class vtk441Mapper : public vtkOpenGLPolyDataMapper
{
  protected:
   GLuint displayList;
   bool   initialized;
   float  animTime;

  public:
   vtk441Mapper()
   {
     initialized = false;
     animTime = 0.0;
   }
    
   virtual void AdvanceAnimation()
   {
       animTime += 0.001;
       if (animTime >= 2.0)
           animTime = 0.0;

       //DEBUG
       //std::cerr << animTime << std::endl;
   }

   void
   RemoveVTKOpenGLStateSideEffects()
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


   void SetupLight(void)
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
};


/* ------------------------------------------------------------------
 * Utility routine: mishii_PrintMatrix().
 *
 * Expects a matrix stored column-major as in a linear array of floats.
 * ------------------------------------------------------------------
 */
void mishii_PrintMatrix(std::ostream &out, const float *mat, int num_rows, int num_cols)
{
    out << std::fixed << std::setprecision(5);
    for (int row = 0; row < num_rows; row++)
    {
        for (int col = 0; col < num_cols; col++)
            out << std::setw(8) << mat[4*col + row] << " ";
        out << std::endl;
    }
}

/* ------------------------------------------------------------------
 * Convenience namepace: glm_mishii_matrix_transforms.
 * ------------------------------------------------------------------
 */
namespace glm_mishii_matrix_transforms
{
    using glm::mat4;
    using glm::vec3;
    using glm::scale;
    using glm::scale;
    using glm::translate;
}


/* ------------------------------------------------------------------
 * Class forward declarations.
 * ------------------------------------------------------------------
 */
class Mesh;
class MeshObject;
class PortalObject;
class DisplayListMesh;
typedef std::list<MeshObject *> MeshObjList;


/* ------------------------------------------------------------------
 * Mesh class.
 * ------------------------------------------------------------------
 */
class Mesh
{
  public:
    virtual void Draw() = 0;
};


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
    virtual void Draw() const
    //void Draw() const   // DEBUG hack: switch these to enable/disable portals.
    {
        glPushMatrix();
          glMultMatrixf(glm::value_ptr(modelMat));
          mesh->Draw();
        glPopMatrix();
    }

    static void DrawList(MeshObjList &l)
    {
        for (MeshObjList::iterator iter = l.begin(); iter != l.end(); ++iter)
            (*iter)->Draw();
    }
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
             // Initialization of currentPortalRecursionDepth follows class definition.
    static PortalObject *oldDestPortal;
             // Initialization of oldDestPortal follows class definition.

  public:
    MeshObjList *parentScene;
    PortalObject *destPortal;

    PortalObject(Mesh *mesh, MeshObjList *scene, PortalObject *portal = NULL,
            glm::mat4 modelMat = glm::mat4(1.0))
            : MeshObject(mesh, modelMat), parentScene(scene), destPortal(portal) {}

    bool SetDestPortal(PortalObject *portal)
    {
        if (portal != NULL && portal->mesh == mesh)
        {
            destPortal = portal;
            return true;
        }
        else
        {
            std::cerr << "PortalObject::SetDestPortal(): Meshes are different."
                    << std::endl;
            return false;
        }
    }

    virtual void Draw() const
    {
        // At a recursion depth of 0, portal rendering is disabled.
        // If this is the old_dest_portal, do not render anything.

        if (PortalObject::oldDestPortal != NULL && this == PortalObject::oldDestPortal)
            return;

        if (PortalObject::currentPortalRecursionDepth
                < PortalObject::MAX_PORTAL_RECURSION_DEPTH)
        {
            //DEBUG
            std::cerr << "DEBUG PortalObject::Draw(): recursion depth = "
                    << PortalObject::currentPortalRecursionDepth
                    << " .. Drawing as PortalObject." << std::endl;

            // Get the view from destPortal.
            float matrixBuffer[16];
            glm::mat4 C1, C2;
            glm::mat4 aboutFace = glm::scale(glm::mat4(), glm::vec3(-1.0f, 1.0f, -1.0f));
            glGetFloatv(GL_MODELVIEW_MATRIX, matrixBuffer);  // The current view.
            C1 = glm::make_mat4(matrixBuffer);               // Type casting.
            C2 = C1 * modelMat * aboutFace * glm::inverse(this->destPortal->modelMat);
                    // The new modelview moves the "camera" to behind the destPortal.

            // Any portals in the (re-)drawn scene will know it is the next recursion level.
            // Preserve the current portal recursion depth.
            int portalRecursionDepth = PortalObject::currentPortalRecursionDepth++;

            // Preserve the oldDestPortal pointer.
            PortalObject *oldDestPortalTrace = PortalObject::oldDestPortal;
            PortalObject::oldDestPortal = this->destPortal;

            // Re-render the scene normally from the destPortal view.
            glPushMatrix();
              glLoadMatrixf(glm::value_ptr(C2));
              MeshObject::DrawList(*(this->destPortal->parentScene));
            glPopMatrix();

            // Now return to the previous tracker value. This is safer than simply decrementing.
            PortalObject::oldDestPortal = oldDestPortalTrace;
            PortalObject::currentPortalRecursionDepth = portalRecursionDepth;
        }
        else
        {
            //DEBUG
            std::cerr << "DEBUG PortalObject::Draw(): recursion depth = "
                    << PortalObject::currentPortalRecursionDepth
                    << " .. Drawing as MeshObject." << std::endl;

            MeshObject::Draw();
        }
    }
};

int PortalObject::currentPortalRecursionDepth = 0;
PortalObject * PortalObject::oldDestPortal = NULL;



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

    vtk441MapperMishii()
    {
      initialized = false;
      animationTarget = NULL;
    }

   ~vtk441MapperMishii()
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

  protected:
    void InitializeScene()
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

  public:
    virtual void RenderPiece(vtkRenderer *ren, vtkActor *act)
    {
        RemoveVTKOpenGLStateSideEffects();
        SetupLight();

        glEnable(GL_COLOR_MATERIAL);
        //glEnable(GL_CULL_FACE);  // Single-sided portals are set using glStencilOpSeparate().
        glEnable(GL_STENCIL_TEST);

        // Initialize the stencil buffer.
        glClearStencil(255);  // Maybe the correct way.
        glClear(GL_STENCIL_BUFFER_BIT);
        glStencilFunc(GL_GEQUAL, 255, 0xFF);
        glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);

        if (!initialized)
            InitializeScene();

        MeshObject::DrawList(meshObjects);
   }

   virtual void AdvanceAnimation()
   {
       animTime += 0.01;
       if (animTime >= 3.0)
           animTime = 0.0;
       if (animationTarget != NULL)
           animationTarget->modelMat[3][2] = 3.0 + 2.0*animTime; }
};

vtkStandardNewMacro(vtk441MapperMishii);

// -------------------------------------------------------------------




class vtkTimerCallback : public vtkCommand
{
  public:
    static vtkTimerCallback *New()
    {
      vtkTimerCallback *cb = new vtkTimerCallback;
      cb->TimerCount = 0;
      cb->mapper = NULL;
      cb->renWin = NULL;
      cb->cam    = NULL;
      cb->angle  = 0;
      return cb;
    }

    void   SetMapper(vtk441Mapper *m) { mapper = m; };
    void   SetRenderWindow(vtkRenderWindow *rw) { renWin = rw; };
    void   SetCamera(vtkCamera *c) { cam = c; };
 
    virtual void Execute(vtkObject *vtkNotUsed(caller), unsigned long eventId,
                         void *vtkNotUsed(callData))
    {
/*
      // THIS IS WHAT GETS CALLED EVERY TIMER
      //cout << "Got a timer!!" << this->TimerCount << endl;

      // NOW DO WHAT EVER ACTIONS YOU WANT TO DO...
      if (vtkCommand::TimerEvent == eventId)
        {
        ++this->TimerCount;
        }

      // Make a call to the mapper to make it alter how it renders...
      if (mapper != NULL)
            mapper->AdvanceAnimation();

      // Force a render...
      if (renWin != NULL)
         renWin->Render();
*/
    }
 
  private:
    int TimerCount;
    vtk441Mapper *mapper;
    vtkRenderWindow *renWin;
    vtkCamera *cam;
    float angle;
};


void KeypressCallbackFunction (
  vtkObject* caller,
  long unsigned int eventId,
  void* clientData,
  void* callData );

int main()
{
  // Dummy input so VTK pipeline mojo is happy.
  //
  vtkSmartPointer<vtkSphereSource> sphere =
    vtkSmartPointer<vtkSphereSource>::New();
  sphere->SetThetaResolution(100);
  sphere->SetPhiResolution(50);

  // The mapper is responsible for pushing the geometry into the graphics
  // library. It may also do color mapping, if scalars or other attributes
  // are defined. 
  //

  vtkSmartPointer<vtk441MapperMishii> winMapper =
    vtkSmartPointer<vtk441MapperMishii>::New();
  winMapper->SetInputConnection(sphere->GetOutputPort());

  vtkSmartPointer<vtkActor> winActor =
    vtkSmartPointer<vtkActor>::New();
  winActor->SetMapper(winMapper);

  vtkSmartPointer<vtkRenderer> ren =
    vtkSmartPointer<vtkRenderer>::New();

  vtkSmartPointer<vtkRenderWindow> renWin =
    vtkSmartPointer<vtkRenderWindow>::New();
  renWin->AddRenderer(ren);
  ren->SetViewport(0.0, 0.0, 1.0, 1);
  renWin->StencilCapableOn();    // Important for proper portals.

  vtkSmartPointer<vtkRenderWindowInteractor> iren =
    vtkSmartPointer<vtkRenderWindowInteractor>::New();
  iren->SetRenderWindow(renWin);

  // Add the actor(s) to the renderer, set the background and size.
  //
  ren->AddActor(winActor);
  ren->SetBackground(0.0, 0.0, 0.0);
  renWin->SetSize(1200, 600);

  // Set up the lighting.
  //
     ren->GetActiveCamera()->SetFocalPoint(0,0,0);
     ren->GetActiveCamera()->SetPosition(0,0,70);
     ren->GetActiveCamera()->SetViewUp(0,1,0);
     ren->GetActiveCamera()->SetClippingRange(20, 120);
     ren->GetActiveCamera()->SetDistance(70);
  
  // This starts the event loop and invokes an initial render.
  //
  ((vtkInteractorStyle *)iren->GetInteractorStyle())->SetAutoAdjustCameraClippingRange(0);
  iren->Initialize();

  // Sign up to receive TimerEvent
  vtkSmartPointer<vtkTimerCallback> cb = 
    vtkSmartPointer<vtkTimerCallback>::New();
  iren->AddObserver(vtkCommand::TimerEvent, cb);
  cb->SetMapper(winMapper);
  cb->SetRenderWindow(renWin);
  cb->SetCamera(ren->GetActiveCamera());
 
  vtkSmartPointer<vtkCallbackCommand> keypressCallback = 
    vtkSmartPointer<vtkCallbackCommand>::New();
  keypressCallback->SetCallback ( KeypressCallbackFunction );
  iren->AddObserver ( vtkCommand::KeyPressEvent, keypressCallback );

  int timerId = iren->CreateRepeatingTimer(10);  // repeats every 10 milliseconds <--> 0.01 seconds
  std::cout << "timerId: " << timerId << std::endl;  
 
  iren->Start();

  return EXIT_SUCCESS;
}

void KeypressCallbackFunction ( vtkObject* caller, long unsigned int vtkNotUsed(eventId), void* vtkNotUsed(clientData), void* vtkNotUsed(callData) )
{
  std::cout << "Keypress callback" << std::endl;
 
  vtkRenderWindowInteractor *iren = 
    static_cast<vtkRenderWindowInteractor*>(caller);
 
  std::cout << "Pressed: " << iren->GetKeySym() << std::endl;

    /* Disabled because doesn't work... */
    /* Test: Show the camera transform matrices access. */
    //float current_modelview[16];
    //glGetFloatv(GL_MODELVIEW_MATRIX, current_modelview);
    //std::cout << "------------------------------------------" << endl;
    //mishii_PrintMatrix(std::cout, current_modelview, 4, 4);
    //std::cout << "------------------------------------------" << endl;

    /* Disabled because doesn't work... */
    /* Test: Show the camera transform matrices access. */
    //vtkRenderer *ren = iren->FindPokedRenderer(1, 0); //Some dummy coordinates that hopefully land me near the right window.
    //ren->PrintSelf(std::cout, vtkIndent(2));
    //vtkCamera *camera = ren->GetActiveCamera();
    //camera->PrintSelf(std::cout, vtkIndent(2));
}




