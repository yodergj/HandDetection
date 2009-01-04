#ifndef MATRIX_H
#define MATRIX_H

#include <stdio.h>

class Matrix
{
  public:
    Matrix();
    ~Matrix();
    bool SetSize(int rows, int columns, bool clear=true);
    int GetRows();
    int GetColumns();
    double GetValue(int row, int column);
    bool SetValue(int row, int column, double value);
    bool Set(double* values);
    bool SetFromProduct(Matrix& a, Matrix& b);
    void SetAsTranspose(Matrix& m);
    bool SetAsInverse(Matrix& m);
    bool SetFromDifference(Matrix& a, Matrix& b);
    bool GetDeterminant(double &determinant);
    void Clear();
    bool Save(FILE* file);
    bool Load(FILE* file);
    Matrix& Transpose();
    Matrix& Inverse();
    Matrix& operator-(Matrix& m);
    Matrix& operator*(Matrix& m);
    Matrix& operator*(double d);
    Matrix& operator=(Matrix& m);
    Matrix& operator*=(double d);
    Matrix& operator+=(Matrix& m);
    Matrix& operator+=(double* values);
    Matrix& operator-=(Matrix& m);
    Matrix& operator-=(double* values);
    bool operator==(Matrix& m);
    bool operator!=(Matrix& m);
    bool operator<(Matrix& m); // For sorting purposes only
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
