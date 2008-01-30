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
    bool operator==(Matrix& m);
    bool operator!=(Matrix& m);
    bool operator<(Matrix& m); // For sorting purposes only
  private:
    int mRows;
    int mColumns;
    double *mData;
};

#endif
