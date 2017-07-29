/* =============================================================================
 * meshobject.cxx
 * Masado Ishii
 * v0.1 2016-12-26
 *
 * Description: Base types for drawable objects having spatial transform.
 *
 * Attributions:
 * =============================================================================
 */

#include "../include/meshobject.h"

#include <iostream> //DEBUG


/* --------------------------------------------------------------------
 * MeshObject member functions.
 * --------------------------------------------------------------------
 */

void MeshObject::Draw() const
{
    glPushMatrix();
      glMultMatrixf(glm::value_ptr(modelMat));
      mesh->Draw();
    glPopMatrix();
}

void MeshObject::DrawList(MeshObjList &l)
{
    for (MeshObjList::iterator iter = l.begin(); iter != l.end(); ++iter)
        (*iter)->Draw();
}


/* --------------------------------------------------------------------
 * PortalObject members.
 * --------------------------------------------------------------------
 */

int PortalObject::currentPortalRecursionDepth = 0;
PortalObject * PortalObject::oldDestPortal = NULL;

bool PortalObject::SetDestPortal(PortalObject *portal)
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

void PortalObject::Draw() const
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

        // Get the view transformation relative to destPortal.
        float matrixBuffer[16];
        glm::mat4 C1, C2;
        glm::mat4 aboutFace = glm::scale(glm::mat4(), glm::vec3(-1.0f, 1.0f, -1.0f));
        glGetFloatv(GL_MODELVIEW_MATRIX, matrixBuffer);  // The current view.
        C1 = glm::make_mat4(matrixBuffer);               // Type casting.
        C2 = C1 * modelMat * aboutFace * glm::inverse(this->destPortal->modelMat);
                // The new modelview moves the "camera" to behind the destPortal.

        // DEBUG: This should ensure the correct preliminary test, which affects the stencil update.
        //glStencilFunc(GL_GEQUAL, 255 - PortalObject::currentPortalRecursionDepth, 0xFF);

        // Initialize the next recursive portal "viewport".
        glStencilMask(0xFF);                     // Enable writing to the stencil buffer.
        glStencilOp(GL_KEEP, GL_KEEP, GL_DECR);  // This region is marked as a deeper recursive level.
        glDepthMask(GL_FALSE);                   // The portal surface is not physical... yet.
        glBlendFunc(GL_ZERO, GL_ZERO);           // Paints a literal silhouette into the color buffer.
        MeshObject::Draw();                      // Do the painting.
        // Back to defaults.
        glBlendFunc(GL_ONE, GL_ZERO);
        glDepthMask(GL_TRUE);
        glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
        glStencilMask(0x0);

        // Recursion book-keeping:
        // Update the statically-scoped portal recursion depth for the scene about to be drawn.
        // Preserve the current portal recursion depth.
        int portalRecursionDepth = PortalObject::currentPortalRecursionDepth++;
        // Preserve the oldDestPortal pointer.
        PortalObject *oldDestPortalTrace = PortalObject::oldDestPortal;
        PortalObject::oldDestPortal = this->destPortal;

        // Set the stencil test ref value to constrain scene rendering to the poral bounds.
        glStencilFunc(GL_GEQUAL, 255 - PortalObject::currentPortalRecursionDepth, 0xFF);

        /* Missing the depth-buffer trick... */

        // Re-render the scene normally from the destPortal view.
        glPushMatrix();
          glLoadMatrixf(glm::value_ptr(C2));
          MeshObject::DrawList(*(this->destPortal->parentScene));
        glPopMatrix();

        // Now return to the previous tracker value. This is safer than simply decrementing.
        PortalObject::oldDestPortal = oldDestPortalTrace;
        PortalObject::currentPortalRecursionDepth = portalRecursionDepth;

        // Restore the stencil test ref value for the scene outside this portal.
        glStencilFunc(GL_GEQUAL, 255 - PortalObject::currentPortalRecursionDepth, 0xFF);

        // "Cap" the portal viewport in the stencil and depth buffers as an ordinary surface.
        glStencilMask(0xFF);                                  // Enable writing to the stencil buffer.
        glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);  // Disable writing to the color buffer.
        MeshObject::Draw();                                   // Do the painting.
        // Back to defaults.
        glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
        glStencilMask(0x0);
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

