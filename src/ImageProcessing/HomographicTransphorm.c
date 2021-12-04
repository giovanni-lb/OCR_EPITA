#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
#pragma GCC diagnostic ignored "-Wtype-limits"
#include <SDL.h>
#include <SDL_image.h>
#include <err.h>
// TODO : remove the STDIO.H
#include <stdio.h>
#pragma GCC diagnostic pop

#include "Pixels.h"
#include "OrderPoints.h"
#include "../Verbose.h"
#include "../NeuralNetwork/MatUtils.h"


//Function : MatrixTransphorm
//Description : Transpose a matrix
//Arguments :   - matrix : the matrix to transpose
//              - transpose : the transposed matrix
//              - rows : the number of rows of the matrix
//              - cols : the number of columns of the matrix
//Return Value : Void

void MatrixTransphorm(double *matrix, double *transpose, int rows, int cols)
{
    int i, j;
    for (i = 0; i < rows; i++)
    {
        for (j = 0; j < cols; j++)
        {
            transpose[j * rows + i] = matrix[i * cols + j];
        }
    }
}




// Function : Fill_matrix
// Description : Computes the value for the homographic transphormation matrix
// input : rderedPoints *points the points

// return : **float the matrix
double *Fill_matrix(orderedPoints points)
{
    orderedPoints *ordered = malloc(sizeof(orderedPoints));
    if (ordered == NULL){
        err(1, "Fill_matrix : Malloc failed");
    }
    ordered->ul = (Point){0,0};
    ordered->ll = (Point){0, 480};
    ordered->ur = (Point){640, 0};
    ordered->lr = (Point){640, 480};
    double *mat = calloc(8*8,sizeof(double));
    for(int i = 0; i < 8; i+=2){
        Point sp;
        Point ap;
        switch (i)
        {
        case 0:
            sp = points.ul;
            ap = ordered->ul;
            break;
        case 2:
            sp = points.ur;
            ap = ordered->ur;
            break;
        case 4:
            sp = points.ll;
            ap = ordered->ll;
            break;
        case 6:
            sp = points.lr;
            ap = ordered->lr;
            break;
        default:
                errx(1, "Undefined point");
            break;
        }
        mat[i * 8 + 0] = ap.x;
        mat[i * 8 + 1] = ap.y;
        mat[i * 8 + 2] = 1;
        mat[i * 8 + 3] = 0;
        mat[i * 8 + 4] = 0;
        mat[i * 8 + 5] = 0;
        mat[i * 8 + 6] = -sp.x * ap.x;
        mat[i * 8 + 7] = -sp.x * ap.y;
        mat[(i+1) * 8 + 0] = 0;
        mat[(i+1) * 8 + 1] = 0;
        mat[(i+1) * 8 + 2] = 0;
        mat[(i+1) * 8 + 3] = ap.x;
        mat[(i+1) * 8 + 4] = ap.y;
        mat[(i+1) * 8 + 5] = 1;
        mat[(i+1) * 8 + 6] = -sp.y * ap.x;
        mat[(i+1) * 8 + 7] = -sp.y * ap.y;
    }
    double *res = calloc(8,sizeof(double));
    double b[] = {
        points.ul.x,
        points.ul.y,
        points.ur.x,
        points.ur.y,
        points.ll.x,
        points.ll.y,
        points.lr.x,
        points.lr.y
    };
    free(ordered);
    // Create a Matrix to store the transpose of the matrix mat
    double *mat_transpose = calloc(8*8,sizeof(double));
    MatrixTransphorm(mat,mat_transpose, 8,8);
    // Create a Matrix to store the multiplication of the matrix mat_transpose and mat
    double *res_mult = calloc(8*8,sizeof(double));
    matricesMult(mat_transpose, mat, 8,8,8, res_mult);
    // Free mat we don't need it anymore
    free(mat);

    // Copy the result of the multiplication in the matrix res_inv to prepare for the inverse
    double *res_inv = calloc(8*16,sizeof(double));

    for(int i = 0; i < 8; i++){
        res[i] = res_mult[i*8+i];
        for(int j = 0; j < 8; j++){
            res_inv[i*16+j] = res_mult[i*8+j];
        }
    }
    
    inverse_matrix(res_inv,8,16);
    // Copy the left part of the inverse in the matrix res_mult
    for(int i = 0; i < 8; i++){
        for(int j = 8; j < 16; j++){
            res_mult[i*8+(j-8)] = res_inv[i*16+j];
        }
    }
    free(res_inv);
    
    double *res_mult2 = calloc(8*8,sizeof(double));
    if (res_mult2 == NULL)
        error_s("Not enough memory !");
    matricesMult(res_mult, mat_transpose, 8,8,8, res_mult2);
    free(res_mult);
    free(mat_transpose);
    matricesMult(res_mult2, b, 8,8,1, res);
    free(res_mult2);
    return res;
}
// Functo