/* =============================================================================
 * utility.cxx
 * Masado Ishii
 * v0.1 2016-12-26
 *
 * Description: Miscellaneous conveniences, such as debug helper routines and 
 *   namespace translations.
 *
 * Attributions:
 * =============================================================================
 */

#include <ostream>
#include <iomanip>
#include "../include/utility.h"

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


