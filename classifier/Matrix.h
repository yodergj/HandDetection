#ifndef MATRIX_H
#define MATRIX_H

#include "XMLUtils.h"
#include <stdio.h>

#define MATRIX_STR "Matrix"

class Matrix
{
  public:
    Matrix();
    Matrix(const Matrix& refMatrix);
    ~Matrix();
    bool SetSize(int rows, int columns, bool clear=true);
    void Fill(double value);
    int GetRows();
    int GetColumns();
    double GetValue(int row, int column) const;
    bool SetValue(int row, int column, double value);
    bool Set(double* values);
    bool Scale(Matrix& a);
    bool SetFromProduct(const Matrix& a, const Matrix& b);
    bool SetFromCellProducts(const Matrix& a, const Matrix& b);
    void SetAsTranspose(Matrix& m);
    bool SetAsInverse(Matrix& m);
    bool SetFromDifference(const Matrix& a, const Matrix& b);
    bool GetDeterminant(double &determinant);
    void Clear();
    bool Print(FILE* file);
    bool Save(const char* filename);
    bool Save(xmlNodePtr matrixNode);
    bool Load(const char* filename);
    bool Load(xmlNodePtr matrixNode);
    Matrix& Transpose();
    Matrix& Inverse();
    Matrix& operator-(const Matrix& m);
    Matrix& operator*(const Matrix& m);
    Matrix& operator*(double d);
    Matrix& operator=(const Matrix& m);
    Matrix& operator*=(double d);
    Matrix& operator+=(const Matrix& m);
    Matrix& operator+=(double* values);
    Matrix& operator-=(const Matrix& m);
    Matrix& operator-=(double* values);
    bool operator==(const Matrix& m) const;
    bool operator!=(const Matrix& m) const;
    bool operator<(const Matrix& m) const; // For sorting purposes only
  private:
    bool RowReduce();
    bool CalculateRowEchelonForm();

    int mRows;
    int mColumns;
    int mCells;
    double *mData;
    int mDataAlloc;
    double *mRowEchelonData;
    int mRowEchelonDataAlloc;
    int mNumRowEchelonSwaps;
};

#endif
