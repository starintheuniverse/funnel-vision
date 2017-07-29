/* =============================================================================
 * utility.h
 * Masado Ishii
 * v0.1 2016-12-26
 *
 * Description: Miscellaneous conveniences, such as debug helper routines and 
 *   namespace translations.
 *
 * Attributions:
 * =============================================================================
 */

#ifndef _UTILITY_H
#define _UTILITY_H

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <ostream>

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
 * Utility routine: mishii_PrintMatrix().
 *
 * Expects a matrix stored column-major as in a linear array of floats.
 * ------------------------------------------------------------------
 */
void mishii_PrintMatrix(std::ostream &out, const float *mat, int num_rows, int num_cols);


#endif /* _UTILITY_H */
