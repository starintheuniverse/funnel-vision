
/* =============================================================================
 * funnel.cxx
 * Masado Ishii
 * CIS 441 "Intro Comp. Graphics" (H.Childs)
 * 2016-12-09
 *
 * Description:
 *   My final project, "FunnelVision," to simulate portal-like visual effects
 *   using the OpenGL and VTK frameworks.
 *
 * Attributions:
 *   > Interactor and mapper derived from examples by Hank Childs.
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


class vtk441Mapper;


class Triangle
{
  public:
      double         X[3];
      double         Y[3];
      double         Z[3];
      double         fieldValue[3]; // always between 0 and 1
      double         normals[3][3];
};


class vtk441Mapper : public vtkOpenGLPolyDataMapper
{
  protected:
   GLuint displayList;
   bool   initialized;
   float  size;

  public:
   vtk441Mapper()
   {
     initialized = false;
     size = 1;
   }
    
   void   IncrementSize()
   {
       size += 0.01;
       if (size > 2.0)
           size = 1.0;
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

class vtk441MapperPart1 : public vtk441Mapper
{
 public:
   static vtk441MapperPart1 *New();
   
   virtual void RenderPiece(vtkRenderer *ren, vtkActor *act)
   {

      /* This is a separate scene from part 2, which is static. */

      RemoveVTKOpenGLStateSideEffects();
      SetupLight();
      glBegin(GL_TRIANGLES);
      glVertex3f(-10*size, -10*size, -10*size);
      glVertex3f(10*size, -10*size, 10*size);
      glVertex3f(10*size, 10*size, 10*size);
      glEnd();
   }
};

vtkStandardNewMacro(vtk441MapperPart1);



/* ------------------
 * mishii helper code
 * ------------------
 */

/*
 * mishii_PrintMatrix()
 *
 * Expects a matrix stored as in a linear array of floats.
 */

void mishii_PrintMatrix(std::ostream &out, float *mat, int num_rows, int num_cols)
{
    out << std::fixed << std::setprecision(5);
    for (int row = 0; row < num_rows; row++)
    {
        for (int col = 0; col < num_cols; col++)
            out << std::setw(8) << *(mat++) << " ";
        out << std::endl;
    }
}

/* 000 */



class vtk441MapperPart2 : public vtk441Mapper
{
 public:
   static vtk441MapperPart2 *New();
   
   GLuint displayList;
   bool   initialized;

   vtk441MapperPart2()
   {
     initialized = false;
   }
   virtual void RenderPiece(vtkRenderer *ren, vtkActor *act)
   {
       RemoveVTKOpenGLStateSideEffects();
       SetupLight();


        GLuint shapes = glGenLists(2);

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
                glCallList(unitSquare);
              glPopMatrix();
          }
          // Facing -Y.
          glRotatef(90, 1, 0, 0);
          glPushMatrix();
            glTranslatef(0, 0, 1);
            glCallList(unitSquare);
          glPopMatrix();
          // Facing +Y.
          glRotatef(180, 1, 0, 0);
          glTranslatef(0, 0, 1);
          glCallList(unitSquare);
        glPopMatrix();
        glEndList();

        // A scene with a ground plane and a floating cube.
        glPushMatrix();
          glScalef(20, 20, 1);
          glCallList(unitSquare);
        glPopMatrix();
        glPushMatrix();
          glTranslatef(-2, -3, 5);
          glScalef(2, 2, 2);
          glCallList(unitCube);
        glPopMatrix();


        // Source portal: Use silhouette to refine the stencil buffer.
        //...Some code

        // Render portal view: Transform view coordinates.
        float current_modelview[16];
        glGetFloatv(GL_MODELVIEW_MATRIX, current_modelview);
        std::cout << "------------------------------------------" << endl;
        mishii_PrintMatrix(std::cout, current_modelview, 4, 4);
        std::cout << "------------------------------------------" << endl;

   }
};

vtkStandardNewMacro(vtk441MapperPart2);

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
      // THIS IS WHAT GETS CALLED EVERY TIMER
      //cout << "Got a timer!!" << this->TimerCount << endl;

      // NOW DO WHAT EVER ACTIONS YOU WANT TO DO...
      if (vtkCommand::TimerEvent == eventId)
        {
        ++this->TimerCount;
        }

      // Make a call to the mapper to make it alter how it renders...
      if (mapper != NULL)
            mapper->IncrementSize();

      // Modify the camera...
      if (cam != NULL)
      {
         cam->SetFocalPoint(0,0,0);
         float rads = angle/360.0*2*3.14;
         cam->SetPosition(70*cos(rads),0,70*sin(rads));
         angle++;
         if (angle > 360)
            angle = 0;
         cam->SetViewUp(0,1,0);
         cam->SetClippingRange(20, 120);
         cam->SetDistance(70);
      }

      // Force a render...
      if (renWin != NULL)
         renWin->Render();
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
  vtkSmartPointer<vtk441MapperPart1> win1Mapper =
    vtkSmartPointer<vtk441MapperPart1>::New();
  win1Mapper->SetInputConnection(sphere->GetOutputPort());

  vtkSmartPointer<vtkActor> win1Actor =
    vtkSmartPointer<vtkActor>::New();
  win1Actor->SetMapper(win1Mapper);

  vtkSmartPointer<vtkRenderer> ren1 =
    vtkSmartPointer<vtkRenderer>::New();

  vtkSmartPointer<vtk441MapperPart2> win2Mapper =
    vtkSmartPointer<vtk441MapperPart2>::New();
  win2Mapper->SetInputConnection(sphere->GetOutputPort());

  vtkSmartPointer<vtkActor> win2Actor =
    vtkSmartPointer<vtkActor>::New();
  win2Actor->SetMapper(win2Mapper);

  vtkSmartPointer<vtkRenderer> ren2 =
    vtkSmartPointer<vtkRenderer>::New();

  vtkSmartPointer<vtkRenderWindow> renWin =
    vtkSmartPointer<vtkRenderWindow>::New();
  renWin->AddRenderer(ren1);
  ren1->SetViewport(0, 0, 0.5, 1);
  renWin->AddRenderer(ren2);
  ren2->SetViewport(0.5, 0, 1.0, 1);

  vtkSmartPointer<vtkRenderWindowInteractor> iren =
    vtkSmartPointer<vtkRenderWindowInteractor>::New();
  iren->SetRenderWindow(renWin);

  // Add the actors to the renderer, set the background and size.
  //
  bool doWindow1 = false;
  if (doWindow1)
     ren1->AddActor(win1Actor);
  ren1->SetBackground(0.0, 0.0, 0.0);
  bool doWindow2 = true;
  if (doWindow2)
      ren2->AddActor(win2Actor);
  ren2->SetBackground(0.0, 0.0, 0.0);
  renWin->SetSize(1200, 600);

  // Set up the lighting.
  //
  vtkRenderer *rens[2] = { ren1, ren2 };
  for (int i = 0 ; i < 2 ; i++)
  {
     rens[i]->GetActiveCamera()->SetFocalPoint(0,0,0);
     rens[i]->GetActiveCamera()->SetPosition(0,0,70);
     rens[i]->GetActiveCamera()->SetViewUp(0,1,0);
     rens[i]->GetActiveCamera()->SetClippingRange(20, 120);
     rens[i]->GetActiveCamera()->SetDistance(70);
  }
  
  // This starts the event loop and invokes an initial render.
  //
  ((vtkInteractorStyle *)iren->GetInteractorStyle())->SetAutoAdjustCameraClippingRange(0);
  iren->Initialize();

  // Sign up to receive TimerEvent
  vtkSmartPointer<vtkTimerCallback> cb = 
    vtkSmartPointer<vtkTimerCallback>::New();
  iren->AddObserver(vtkCommand::TimerEvent, cb);
  cb->SetMapper(win1Mapper);
  cb->SetRenderWindow(renWin);
  cb->SetCamera(ren1->GetActiveCamera());
 
  vtkSmartPointer<vtkCallbackCommand> keypressCallback = 
    vtkSmartPointer<vtkCallbackCommand>::New();
  keypressCallback->SetCallback ( KeypressCallbackFunction );
  iren->AddObserver ( vtkCommand::KeyPressEvent, keypressCallback );

  /* Disabled... */
  //int timerId = iren->CreateRepeatingTimer(10);  // repeats every 10 microseconds <--> 0.01 seconds
  //std::cout << "timerId: " << timerId << std::endl;  
 
  iren->Start();

  return EXIT_SUCCESS;
}

void KeypressCallbackFunction ( vtkObject* caller, long unsigned int vtkNotUsed(eventId), void* vtkNotUsed(clientData), void* vtkNotUsed(callData) )
{
  std::cout << "Keypress callback" << std::endl;
 
  vtkRenderWindowInteractor *iren = 
    static_cast<vtkRenderWindowInteractor*>(caller);
 
  std::cout << "Pressed: " << iren->GetKeySym() << std::endl;


    /* Disabled... */
    /* Test: Show the camera transform matrices access. */
    //vtkRenderer *ren = iren->FindPokedRenderer(1, 0); //Some dummy coordinates that hopefully land me near the right window.
    //ren->PrintSelf(std::cout, vtkIndent(2));
    //vtkCamera *camera = ren->GetActiveCamera();
    //camera->PrintSelf(std::cout, vtkIndent(2));
}




