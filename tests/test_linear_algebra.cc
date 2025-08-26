#include <gtest/gtest.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <swiftnav/linear_algebra.h>

#include "check_utils.h"

#define LINALG_TOL 1e-9
#define LINALG_NUM 22
#define MATRIX_MIN -1e3
#define MATRIX_MAX 1e3
#define MSIZE_MAX 64

/* TODO: matrix_multiply, matrix_add_sc, matrix_copy, all vector functions */

namespace {

TEST(TestLinearAlgebra, MatrixInverse2x2) {
  u32 i, j, t;
  double A[4];
  double B[4];
  double I[4];

  Random rand{};
  /* 2x2 inverses */
  for (t = 0; t < LINALG_NUM; t++) {
    do {
      for (i = 0; i < 2; i++) {
        for (j = 0; j < 2; j++) {
          A[2 * i + j] = rand.frand(MATRIX_MIN, MATRIX_MAX);
        }
      }
    } while (matrix_inverse(2, A, B) < 0);
    matrix_multiply(2, 2, 2, A, B, I);
    EXPECT_LT(fabs(I[0] - 1), LINALG_TOL)
        << "Matrix differs from identity: " << I[0];
    EXPECT_LT(fabs(I[3] - 1), LINALG_TOL)
        << "Matrix differs from identity: " << I[3];
  }
  for (i = 0; i < 2; i++) {
    for (j = 0; j < 2; j++) {
      if (j == 0) {
        A[2 * i + j] = 22;
      } else {
        A[2 * i + j] = 1;
      }
    }
  }
  s32 mi = matrix_inverse(2, A, B);
  EXPECT_LT(mi, 0) << "Singular matrix not detected.";
}

TEST(TestLinearAlgebra, MatrixInverse3x3) {
  u32 i, j, t;
  double A[9];
  double B[9];
  double I[9];

  Random rand{};
  /* 3x3 inverses */
  for (t = 0; t < LINALG_NUM; t++) {
    do {
      for (i = 0; i < 3; i++) {
        for (j = 0; j < 3; j++) {
          A[3 * i + j] = rand.frand(MATRIX_MIN, MATRIX_MAX);
        }
      }
    } while (matrix_inverse(3, A, B) < 0);
    matrix_multiply(3, 3, 3, A, B, I);
    EXPECT_NEAR(I[0], 1, LINALG_TOL)
        << "Matrix differs from identity: " << I[0];
    EXPECT_NEAR(I[4], 1, LINALG_TOL)
        << "Matrix differs from identity: " << I[4];
    EXPECT_NEAR(I[8], 1, LINALG_TOL)
        << "Matrix differs from identity: " << I[8];
  }
  for (i = 0; i < 3; i++) {
    for (j = 0; j < 3; j++) {
      if (j == 0) {
        A[3 * i + j] = 33;
      } else {
        A[3 * i + j] = 1;
      }
    }
  }
  s32 mi = matrix_inverse(3, A, B);
  EXPECT_LT(mi, 0) << "Singular matrix not detected.";
}

TEST(TestLinearAlgebra, MatrixInverse4x4) {
  u32 i, j, t;
  double A[16];
  double B[16];
  double I[16];

  Random rand{};
  /* 4x4 inverses */
  for (t = 0; t < LINALG_NUM; t++) {
    do {
      for (i = 0; i < 4; i++) {
        for (j = 0; j < 4; j++) {
          A[4 * i + j] = rand.frand(MATRIX_MIN, MATRIX_MAX);
        }
      }
    } while (matrix_inverse(4, A, B) < 0);
    matrix_multiply(4, 4, 4, A, B, I);
    EXPECT_NEAR(I[0], 1, LINALG_TOL)
        << "Matrix differs from identity: " << I[0];
    EXPECT_NEAR(I[5], 1, LINALG_TOL)
        << "Matrix differs from identity: " << I[5];
    EXPECT_NEAR(I[10], 1, LINALG_TOL)
        << "Matrix differs from identity: " << I[10];
    EXPECT_NEAR(I[15], 1, LINALG_TOL)
        << "Matrix differs from identity: " << I[15];
  }
  for (i = 0; i < 4; i++) {
    for (j = 0; j < 4; j++) {
      if (j == 0) {
        A[4 * i + j] = 44;
      } else {
        A[4 * i + j] = 1;
      }
    }
  }
  s32 mi = matrix_inverse(4, A, B);
  EXPECT_LT(mi, 0) << "Singular matrix not detected.";
}

TEST(TestLinearAlgebra, MatrixInverse5x5) {
  u32 i, j, t;
  double A[25];
  double B[25];
  double I[25];

  Random rand{};
  /* 5x5 inverses */
  for (t = 0; t < LINALG_NUM; t++) {
    do {
      for (i = 0; i < 5; i++) {
        for (j = 0; j < 5; j++) {
          A[5 * i + j] = rand.frand(MATRIX_MIN, MATRIX_MAX);
        }
      }
    } while (matrix_inverse(5, A, B) < 0);
    matrix_multiply(5, 5, 5, A, B, I);
    EXPECT_NEAR(I[0], 1, LINALG_TOL)
        << "Matrix differs from identity: " << I[0];
    EXPECT_NEAR(I[6], 1, LINALG_TOL)
        << "Matrix differs from identity: " << I[6];
    EXPECT_NEAR(I[12], 1, LINALG_TOL)
        << "Matrix differs from identity: " << I[12];
    EXPECT_NEAR(I[18], 1, LINALG_TOL)
        << "Matrix differs from identity: " << I[18];
    EXPECT_NEAR(I[24], 1, LINALG_TOL)
        << "Matrix differs from identity: " << I[24];
  }
  for (i = 0; i < 5; i++) {
    for (j = 0; j < 5; j++) {
      if (j == 0) {
        A[5 * i + j] = 55;
      } else {
        A[5 * i + j] = 1;
      }
    }
  }
  s32 mi = matrix_inverse(5, A, B);
  EXPECT_LT(mi, 0) << "Singular matrix not detected.";
}

TEST(TestLinearAlgebra, MatrixEye) {
  double M[10][10];

  matrix_eye(10, &M[0][0]);

  for (u32 i = 0; i < 10; i++) {
    for (u32 j = 0; j < 10; j++) {
      if (i == j) {
        EXPECT_EQ(M[i][j], 1) << "Identity diagonal element != 1";
      } else {
        EXPECT_EQ(M[i][j], 0) << "Identity off-diagonal element != 0";
      }
    }
  }
}

TEST(TestLinearAlgebra, MatrixTriu) {
  double M[4][4] = {
      {1, 2, 3, 4}, {5, 6, 7, 8}, {9, 10, 11, 12}, {13, 14, 15, 16}};

  double M_[4][4] = {{1, 2, 3, 4}, {0, 6, 7, 8}, {0, 0, 11, 12}, {0, 0, 0, 16}};

  matrix_triu(4, &M[0][0]);

  for (u32 i = 0; i < 4; i++) {
    for (u32 j = 0; j < 4; j++) {
      EXPECT_EQ(M[i][j], M_[i][j]) << "triu result != test matrix";
    }
  }
}

TEST(TestLinearAlgebra, MatrixUdu1) {
  double M[4][4] = {{100, 145, 121, 16},
                    {145, 221, 183, 24},
                    {121, 183, 199, 28},
                    {16, 24, 28, 4}};

  double U[4][4] = {{0}};
  double D[4] = {0};

  matrix_udu(4, &M[0][0], &U[0][0], D);

  double U_[4][4] = {{1, 2, 3, 4}, {0, 1, 5, 6}, {0, 0, 1, 7}, {0, 0, 0, 1}};

  double D_[4] = {1, 2, 3, 4};

  for (u32 i = 0; i < 4; i++) {
    for (u32 j = 0; j < 4; j++) {
      EXPECT_EQ(U[i][j], U_[i][j]) << "U result != test matrix";
    }
  }
  for (u32 i = 0; i < 4; i++) {
    EXPECT_EQ(D[i], D_[i]) << "D result != test D";
  }
}

TEST(TestLinearAlgebra, MatrixUdu2) {
  Random rand{};
  u32 n = rand.sizerand(MSIZE_MAX);
  double M[n][n];
  double M_orig[n][n];

  for (u32 i = 0; i < n; i++) {
    for (u32 j = 0; j <= i; j++) {
      M[i][j] = M[j][i] = rand.frand(MATRIX_MIN, MATRIX_MAX);
    }
  }

  /* Square the random matrix to ensure it is positive semi-definite. */
  matrix_multiply(n, n, n, &M[0][0], &M[0][0], &M_orig[0][0]);
  memcpy(M, M_orig, n * n * sizeof(double));

  double U[n][n];
  memset(U, 0, n * n * sizeof(double));
  double D[n];
  memset(D, 0, n * sizeof(double));

  matrix_udu(n, &M[0][0], &U[0][0], D);

  /* Check U is unit upper triangular. */
  for (u32 i = 0; i < n; i++) {
    for (u32 j = 0; j < n; j++) {
      if (i == j) {
        EXPECT_NEAR(U[i][j], 1, LINALG_TOL)
            << "U diagonal element != 1 (was " << M[i][j] << ")";
      }
      if (i > j) {
        EXPECT_LT(fabs(U[i][j]), LINALG_TOL) << "U lower triangle element != 0";
      }
    }
  }

  /* Check reconstructed matrix is correct. */
  double M_[n][n];
  memset(M_, 0, n * n * sizeof(double));
  matrix_reconstruct_udu(n, &U[0][0], D, &M_[0][0]);

  for (u32 i = 0; i < n; i++) {
    for (u32 j = 0; j < n; j++) {
      EXPECT_NEAR(M_orig[i][j], M_[i][j], LINALG_TOL * MATRIX_MAX)
          << "reconstructed result != original matrix, delta[" << i << "][" << j
          << "] = " << fabs(M_orig[i][j] - M_[i][j]);
    }
  }
}

TEST(TestLinearAlgebra, MatrixUdu3) {
  double M[3][3] = {
      {36, 49, 9},
      {49, 77, 15},
      {9, 15, 3},
  };

  double U[3][3] = {{0}};
  double D[3] = {0};

  matrix_udu(3, &M[0][0], &U[0][0], D);

  /* Check using formula for 3x3 matrix on Gibbs p. 393 */
  EXPECT_EQ(D[2], M[2][2]) << "D[2] incorrect";
  EXPECT_EQ(U[2][1], M[2][1] / D[2]) << "U[2][1] incorrect";
  EXPECT_EQ(U[2][0], M[2][0] / D[2]) << "U[2][0] incorrect";
  EXPECT_EQ(D[1], (M[1][1] - U[2][1] * U[2][1] * D[2])) << "D[1] incorrect";
  EXPECT_EQ(U[1][0], ((M[1][0] - U[2][0] * U[2][1] * D[2]) / D[1]))
      << "U[1][0] incorrect";
  EXPECT_EQ(D[0],
            (M[0][0] - U[1][0] * U[1][0] * D[1] - U[2][0] * U[2][0] * D[2]))
      << "D[0] incorrect";
}

TEST(TestLinearAlgebra, MatrixReconstructUdu) {
  double U[4][4] = {{1, 2, 3, 4}, {0, 1, 5, 6}, {0, 0, 1, 7}, {0, 0, 0, 1}};

  double D[4] = {1, 2, 3, 4};

  double M[4][4] = {{0}};

  double M_[4][4] = {{100, 145, 121, 16},
                     {145, 221, 183, 24},
                     {121, 183, 199, 28},
                     {16, 24, 28, 4}};

  matrix_reconstruct_udu(4, &U[0][0], D, &M[0][0]);

  for (u32 i = 0; i < 4; i++) {
    for (u32 j = 0; j < 4; j++) {
      EXPECT_EQ(M[i][j], M_[i][j]) << "reconstructed result != test matrix";
    }
  }
}

TEST(TestLinearAlgebra, MatrixAddSc) {
  u32 i, j, t;

  Random rand{};
  for (t = 0; t < LINALG_NUM; t++) {
    u32 n = rand.sizerand(MSIZE_MAX);
    u32 m = rand.sizerand(MSIZE_MAX);
    double A[n * m];
    double B[m * n];
    for (i = 0; i < n; i++) {
      for (j = 0; j < m; j++) {
        A[m * i + j] = rand.frand(MATRIX_MIN, MATRIX_MAX);
      }
    }
    matrix_add_sc(n, m, A, A, -1, B);
    for (i = 0; i < n; i++) {
      for (j = 0; j < m; j++) {
        EXPECT_LT(fabs(B[m * i + j]), LINALG_TOL)
            << "Matrix differs from zero: " << B[m * i + j];
      }
    }
  }
}

TEST(TestLinearAlgebra, MatrixCopy) {
  u32 i, j, t;
  double tmp;

  Random rand{};
  for (t = 0; t < LINALG_NUM; t++) {
    u32 n = rand.sizerand(MSIZE_MAX);
    u32 m = rand.sizerand(MSIZE_MAX);
    double A[n * m];
    double B[m * n];
    for (i = 0; i < n; i++) {
      for (j = 0; j < m; j++) {
        A[m * i + j] = rand.frand(MATRIX_MIN, MATRIX_MAX);
      }
    }
    matrix_copy(n, m, A, B);
    for (i = 0; i < n; i++) {
      for (j = 0; j < m; j++) {
        tmp = fabs(B[m * i + j] - A[m * i + j]);
        EXPECT_LT(tmp, LINALG_TOL) << "Matrix differs from zero: %lf" << tmp;
      }
    }
  }
}

TEST(TestLinearAlgebra, MatrixTranspose) {
  u32 i, j, t;

  Random rand{};
  for (t = 0; t < LINALG_NUM; t++) {
    u32 n = rand.sizerand(MSIZE_MAX);
    u32 m = rand.sizerand(MSIZE_MAX);
    double A[n * m];
    double B[m * n];
    double C[n * m];
    for (i = 0; i < n; i++) {
      for (j = 0; j < m; j++) {
        A[m * i + j] = rand.frand(MATRIX_MIN, MATRIX_MAX);
      }
    }
    matrix_transpose(n, m, A, B);
    matrix_transpose(m, n, B, C);
    for (i = 0; i < n; i++) {
      for (j = 0; j < m; j++) {
        EXPECT_NEAR(A[m * i + j], C[m * i + j], LINALG_TOL)
            << "Matrix element differs from original: " << A[m * i + j] << ", "
            << C[m * i + j];
      }
    }
  }
}

TEST(TestLinearAlgebra, VectorDot) {
  u32 i, t;

  Random rand{};
  for (t = 0; t < LINALG_NUM; t++) {
    u32 n = rand.sizerand(MSIZE_MAX);
    u32 mid;
    if (n % 2 == 0) {
      mid = n / 2;
    } else {
      mid = (n - 1) / 2;
    }

    double A[n], B[n];
    for (i = 0; i < n; i++) {
      A[i] = rand.frand(MATRIX_MIN, MATRIX_MAX) / 1e20;
      if (i < mid) {
        B[n - i - 1] = -A[i];
      } else {
        B[n - i - 1] = A[i];
      }
    }
    double dot = vector_dot(n, A, B);
    if (n % 2 == 0) {
      EXPECT_LT(fabs(dot), LINALG_TOL)
          << "Dot product differs from zero: " << vector_dot(n, A, B);
    } else {
      EXPECT_NEAR(dot, A[mid] * B[mid], LINALG_TOL)
          << "Dot product differs from square of middle element "
             "%lf: %lf (%lf)"
          << A[mid] * B[mid] << dot << dot - A[mid] * B[mid];
    }
  }
}

TEST(TestLinearAlgebra, VectorMean) {
  u32 i, t;

  Random rand{};
  for (t = 0; t < LINALG_NUM; t++) {
    u32 n = rand.sizerand(MSIZE_MAX);
    double A[n];
    double test = rand.frand(MATRIX_MIN, MATRIX_MAX) / 1e22;
    for (i = 0; i < n; i++) {
      A[i] = test + i;
    }
    double mean = vector_mean(n, A);
    double expect = test + (n - 1.0) / 2.0;
    EXPECT_NEAR(mean, expect, LINALG_TOL)
        << "Mean differs from expected %lf: %lf (%lf)" << expect << mean
        << fabs(mean - expect);
  }
}

TEST(TestLinearAlgebra, VectorNorm) {
  u32 i, t;

  Random rand{};
  for (t = 0; t < LINALG_NUM; t++) {
    u32 n = rand.sizerand(MSIZE_MAX);
    double test = rand.frand(MATRIX_MIN, MATRIX_MAX) / 1e22;
    double A[n];
    for (i = 0; i < n; i++) {
      A[i] = test;
    }
    EXPECT_NEAR(vector_norm(n, A) * vector_norm(n, A),
                n * test * test,
                LINALG_TOL * vector_norm(n, A))
        << "Norm differs from expected %lf: %lf (%lf)" << n * test * test
        << vector_norm(n, A) * vector_norm(n, A)
        << fabs(vector_norm(n, A) * vector_norm(n, A) - n * test * test);
  }
}

TEST(TestLinearAlgebra, VectorNormalize) {
  u32 i, t;

  Random rand{};
  for (t = 0; t < LINALG_NUM; t++) {
    u32 n = rand.sizerand(MSIZE_MAX);
    double A[n];
    for (i = 0; i < n; i++) {
      A[i] = rand.frand(MATRIX_MIN, MATRIX_MAX);
    }
    vector_normalize(n, A);
    double vnorm = vector_norm(n, A);
    EXPECT_NEAR(vnorm, 1, LINALG_TOL)
        << "Norm differs from 1: %lf" << vector_norm(n, A);
  }
}

TEST(TestLinearAlgebra, VectorAddSc) {
  u32 i, t;

  Random rand{};
  for (t = 0; t < LINALG_NUM; t++) {
    u32 n = rand.sizerand(MSIZE_MAX);
    double A[n], B[n];
    for (i = 0; i < n; i++) {
      A[i] = rand.frand(MATRIX_MIN, MATRIX_MAX);
    }
    vector_add_sc(n, A, A, -1, B);
    for (i = 0; i < n; i++) {
      EXPECT_LT(fabs(B[i]), LINALG_TOL)
          << "Vector element differs from 0: " << B[i];
    }
  }
}

TEST(TestLinearAlgebra, VectorAdd) {
  u32 i, t;

  Random rand{};
  for (t = 0; t < LINALG_NUM; t++) {
    u32 n = rand.sizerand(MSIZE_MAX);
    double A[n], B[n], C[n];
    for (i = 0; i < n; i++) {
      A[i] = rand.frand(MATRIX_MIN, MATRIX_MAX);
      B[i] = -A[i];
    }
    vector_add(n, A, B, C);
    for (i = 0; i < n; i++) {
      EXPECT_LT(fabs(C[i]), LINALG_TOL)
          << "Vector element differs from 0: " << C[i];
    }
  }
}

TEST(TestLinearAlgebra, VectorSubtract) {
  u32 i, t;

  Random rand{};
  for (t = 0; t < LINALG_NUM; t++) {
    u32 n = rand.sizerand(MSIZE_MAX);
    double A[n], B[n], C[n];
    for (i = 0; i < n; i++) {
      A[i] = rand.frand(MATRIX_MIN, MATRIX_MAX);
      B[i] = A[i];
    }
    vector_subtract(n, A, B, C);
    for (i = 0; i < n; i++) {
      EXPECT_LT(fabs(C[i]), LINALG_TOL)
          << "Vector element differs from 0: " << C[i];
    }
  }
}

TEST(TestLinearAlgebra, VectorCross) {
  u32 i, t;

  Random rand{};
  for (t = 0; t < LINALG_NUM; t++) {
    double A[3], B[3], C[3], D[3];
    for (i = 0; i < 3; i++) {
      A[i] = rand.frand(MATRIX_MIN, MATRIX_MAX);
      B[i] = A[i];
    }
    vector_cross(A, B, C);
    for (i = 0; i < 3; i++) {
      EXPECT_LT(fabs(C[i]), LINALG_TOL)
          << "Vector element differs from 0: " << C[i];
    }
    for (i = 0; i < 3; i++) {
      A[i] = rand.frand(MATRIX_MIN, MATRIX_MAX);
      B[i] = rand.frand(MATRIX_MIN, MATRIX_MAX);
    }
    vector_cross(A, B, C);
    for (i = 0; i < 3; i++) {
      B[i] *= -1;
    }
    vector_cross(B, A, D);
    for (i = 0; i < 3; i++) {
      EXPECT_NEAR(C[i], D[i], LINALG_TOL);
    }
  }
}

TEST(TestLinearAlgebra, VectorThree) {
  u32 i, t;

  Random rand{};
  for (t = 0; t < LINALG_NUM; t++) {
    double A[3], B[3], C[3], tmp[3];
    double D, E, F, norm;
    for (i = 0; i < 3; i++) {
      A[i] = rand.frand(MATRIX_MIN, MATRIX_MAX) / 1e20;
      B[i] = rand.frand(MATRIX_MIN, MATRIX_MAX) / 1e20;
      C[i] = rand.frand(MATRIX_MIN, MATRIX_MAX) / 1e20;
    }
    /* Check triple product identity */
    vector_cross(A, B, tmp);
    D = vector_dot(3, C, tmp);
    vector_cross(B, C, tmp);
    E = vector_dot(3, A, tmp);
    vector_cross(C, A, tmp);
    F = vector_dot(3, B, tmp);

    norm = (vector_norm(3, A) + vector_norm(3, B) + vector_norm(3, C)) / 3;
    EXPECT_NEAR(E, D, LINALG_TOL * norm);
    EXPECT_NEAR(E, F, LINALG_TOL * norm);
    EXPECT_NEAR(F, D, LINALG_TOL * norm);
  }
}

/*
TEST(TestLinearAlgebra, QrsolveConsistency) {
  u32 i, j, t;
  double norm;

  Random rand{};
  for (t = 0; t < LINALG_NUM; t++) {
    u32 n = rand.sizerand(MSIZE_MAX);
    double x_gauss[n], x_qr[n];
    double A[n*n], Ainv[n*n], b[n];
    do {
      for (i = 0; i < n; i++) {
        b[i] = rand.frand(MATRIX_MIN, MATRIX_MAX);;
        for (j = 0; j < n; j++)
          A[n*i + j] = rand.frand(MATRIX_MIN, MATRIX_MAX);;
      }
    } while (matrix_inverse(n, A, Ainv) < 0);
    matrix_multiply(n, n, 1, Ainv, b, x_gauss);
    qrsolve(A, n, n, b, x_qr);

    norm = (vector_norm(n, x_qr) + vector_norm(n, x_gauss)) / 2;
    for (i = 0; i < n; i++)
      EXPECT_NEAR(x_qr[i] , x_gauss[i], LINALG_TOL * norm,
                  "QR solve failure; difference was %lf for element %u",
                  x_qr[i] - x_gauss[i], i);
  }
}


TEST(TestLinearAlgebra, QrsolveRect) {
  s32 i;
  const double A[8] = {-0.0178505395610981, 1.4638781031761146,
                       -0.8242742209580581, -0.6843477128009663,
                       0.9155272861151404, -0.1651159277864960,
                       -0.9929037180867774, -0.1491537478964264};
  double Q[16], R[8];

  double buf[10] SWIFT_ATTR_UNUSED = {22, 22, 22, 22, 22,
                                            22, 22, 22, 22, 22};

  i = qrdecomp(A, 4, 2, Q, R);

  printf("i returned %d\n", i);

  MAT_PRINTF(A, 4, 2);
  MAT_PRINTF(Q, 4, 4);
  MAT_PRINTF(R, 4, 2);
}

*/

TEST(TestLinearAlgebra, Submatrix) {
  const double A[3 * 3] = {0, 1, 2, 3, 4, 5, 6, 7, 8};

  double A2[2 * 2];

  u32 row_map[2] = {1, 2};
  u32 col_map[2] = {0, 1};

  const double answer[2 * 2] = {3, 4, 6, 7};

  submatrix(2, 2, 3, A, row_map, col_map, A2);

  for (u8 i = 0; i < 2 * 2; i++) {
    EXPECT_EQ(answer[i], A2[i]);
  }
}

TEST(TestLinearAlgebra, VectorDistance) {
  const double A1[1 * 4] = {0, 1, 2, 3};

  const double B1[1 * 4] = {0, 2, 1, -3};

  const double C1[1 * 4] = {0, 1, 1, 6};

  for (u8 i = 0; i < 4; i++) {
    double dist;
    dist = vector_distance(1, &A1[i], &B1[i]);
    EXPECT_NEAR(dist, C1[i], LINALG_TOL);
  }

  const double A2[2 * 5] = {0, 0, 1, 1, 2, 1, 0, 0, 0, 0};

  const double B2[2 * 5] = {0, 0, 1, 1, 1, 1, 1, 1, -1, -1};

  const double C2[1 * 5] = {0, 0, 1, M_SQRT2, M_SQRT2};

  for (u8 i = 0; i < 5; i++) {
    double dist;
    dist = vector_distance(2, &A2[i * 2], &B2[i * 2]);
    EXPECT_NEAR(dist, C2[i], LINALG_TOL);
  }

  const double A3[3 * 5] = {0, 0, 0, 1, 1, 1, 2, 1, 0, 0, 0, 0, 0, 0, 0};

  const double B3[3 * 5] = {0, 0, 0, 1, 1, 1, 1, 1, 0, 1, 1, 1, -1, -1, 1};

  const double C3[3 * 5] = {
      0,
      0,
      1,
      1.73205080756887729352744634150587236694280525381038062805580,
      1.73205080756887729352744634150587236694280525381038062805580};

  for (u8 i = 0; i < 5; i++) {
    double dist;
    dist = vector_distance(3, &A3[i * 3], &B3[i * 3]);
    EXPECT_NEAR(dist, C3[i], LINALG_TOL);
  }
}

}  // namespace
