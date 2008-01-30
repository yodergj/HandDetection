#include <stdlib.h>
#include <string.h>
#include "Matrix.h"

Matrix::Matrix()
{
  mData = NULL;
  mRows = 0;
  mColumns = 0;
}

Matrix::~Matrix()
{
  if ( mData )
    free(mData);
}

bool Matrix::SetSize(int rows, int columns, bool clear)
{
  double *tmp;

  if ( (rows < 0) || (columns < 0) )
    return false;

  if ( (rows > 0) && (columns > 0) )
  {
    tmp = (double *)realloc(mData, rows * columns * sizeof(double));
    if ( !tmp )
      return false;

    mData = tmp;
  }
  mRows = rows;
  mColumns = columns;

  if ( clear )
    Clear();

  return true;
}

int Matrix::GetRows()
{
  return mRows;
}

int Matrix::GetColumns()
{
  return mColumns;
}

double Matrix::GetValue(int row, int column)
{
  if ( (row >= mRows) || (row < 0) ||
       (column >= mColumns) || (column < 0) )
    return 0;

  return mData[row * mColumns + column];
}

bool Matrix::SetValue(int row, int column, double value)
{
  if ( (row >= mRows) || (row < 0) ||
       (column >= mColumns) || (column < 0) )
    return false;

  mData[row * mColumns + column] = value;

  return true;
}

bool Matrix::GetDeterminant(double &determinant)
{
  int i;
  int row, column;
  int sign = 1;
  double subDeterminant;

  if ( (mRows != mColumns) || (mRows == 0) )
  {
    fprintf(stderr, "GetDeterminant: Matrix is not square (%d rows, %d columns\n", mRows, mColumns);
    return false;
  }

  if ( mRows == 1 )
    determinant = mData[0];
  else if ( mRows == 2 )
    determinant = mData[0] * mData[3] - mData[1] * mData[2];
  else
  {
    Matrix subMatrix;
    subMatrix.SetSize(mRows - 1, mRows - 1, false);
    determinant = 0;
    for (i = 0; i < mRows; i++)
    {
      for (row = 1; row < mRows; row++)
      {
        for (column = 0; column < mRows; column++)
        {
          if ( column < i )
            subMatrix.SetValue(row - 1, column, mData[row * mColumns + column]);
          else if ( column > i )
            subMatrix.SetValue(row - 1, column - 1, mData[row * mColumns + column]);
        }
      }
      subMatrix.GetDeterminant(subDeterminant);
      determinant += sign * mData[i] * subDeterminant;
      sign *= -1;
    }
  }

  return true;
}

void Matrix::Clear()
{
  if ( !mRows || !mColumns )
    return;
  memset(mData, 0, mRows * mColumns * sizeof(double));
}

Matrix& Matrix::Transpose()
{
  int i, j;
  static Matrix result;

  result.SetSize(mColumns, mRows, false);
  for (i = 0; i < mRows; i++)
    for (j = 0; j < mColumns; j++)
      result.mData[i * mColumns + j] =  mData[j * mColumns + i];
  return result;
}

Matrix& Matrix::Inverse()
{
  double determinant;
  static Matrix result;

  /* TODO : Reimplement for General Case */
  if ( (mRows != 2) || (mColumns != 2) )
    result.SetSize(0, 0);
  else
  {
    GetDeterminant(determinant);
    if ( determinant > 0 )
    {
      result.SetSize(2, 2, false);
      result.SetValue(0, 0, GetValue(1, 1));
      result.SetValue(0, 1, -1 * GetValue(0, 1));
      result.SetValue(1, 0, -1 * GetValue(1, 0));
      result.SetValue(1, 1, GetValue(0, 0));
      result = result * (1 / determinant);
    }
    else
      result.SetSize(0, 0);
  }
  return result;
}

Matrix& Matrix::operator-(Matrix& m)
{
  int i, j;
  static Matrix result;

  if ( (mRows != m.mRows) || (mColumns != m.mColumns) )
    result.SetSize(0, 0);
  else
  {
    result.SetSize(mRows, mColumns, false);
    for (i = 0; i < mRows; i++)
      for (j = 0; j < mColumns; j++)
        result.mData[i * mColumns + j] =  mData[i * mColumns + j] - m.mData[i * mColumns + j];
  }
  return result;
}

Matrix& Matrix::operator*(Matrix& m)
{
  int i;
  int row, column;
  static Matrix result;

  if ( mColumns != m.mRows )
  {
    result.SetSize(0, 0);
    return result;
  }

  result.SetSize(mRows, m.mColumns, false);
  for (row = 0; row < mRows; row++)
    for (column = 0; column < m.mColumns; column++)
    {
      result.mData[row * m.mColumns + column] = 0;
      for (i = 0; i < mColumns; i++)
        result.mData[row * m.mColumns + column] += mData[row * mColumns + i] * m.mData[i * m.mColumns + column];
    }
  return result;
}

Matrix& Matrix::operator*(double d)
{
  int i, j;
  static Matrix result;

  result.SetSize(mRows, mColumns, false);
  for (i = 0; i < mRows; i++)
    for (j = 0; j < mColumns; j++)
      result.mData[i * mColumns + j] = d * mData[i * mColumns + j];
  return result;
}

Matrix& Matrix::operator=(Matrix& m)
{
  SetSize(m.mRows, m.mColumns, false);
  memcpy(mData, m.mData, mRows * mColumns * sizeof(double));
  return *this;
}

Matrix& Matrix::operator*=(double d)
{
  int i, j;

  for (i = 0; i < mRows; i++)
    for (j = 0; j < mColumns; j++)
      mData[i * mColumns + j] *= d;
  return *this;
}

Matrix& Matrix::operator+=(Matrix& m)
{
  int i, j;

  for (i = 0; i < mRows; i++)
    for (j = 0; j < mColumns; j++)
      mData[i * mColumns + j] += m.mData[i * mColumns + j];
  return *this;
}

bool Matrix::operator==(Matrix& m)
{
  if ( (mRows != m.mRows) || (mColumns != m.mColumns) )
    return false;

  return !memcmp(mData, m.mData, mRows * mColumns * sizeof(double));
}

bool Matrix::operator!=(Matrix& m)
{
  return !(*this == m);
}

bool Matrix::operator<(Matrix& m)
{
  int row, column;

  if ( mRows != m.mRows )
    return mRows < m.mRows;

  if ( mColumns != m.mColumns )
    return mColumns < m.mColumns;

  for (row = 0; row < mRows; row++)
    for (column = 0; column < mColumns; column++)
      if ( mData[row * mColumns + column] != m.mData[row * mColumns + column] )
        return mData[row * mColumns + column] < m.mData[row * mColumns + column];

  return false;
}

#define LABEL "Matrix"
#define LABEL_LEN 6
bool Matrix::Save(FILE* file)
{
  int i, j;

  if ( !file )
    return false;

  fprintf(file, "%s %d %d\n", LABEL, mRows, mColumns);
  for (i = 0; i < mRows; i++)
  {
    for (j = 0; j < mColumns; j++)
      fprintf(file, "%.9f ", mData[i * mColumns + j]);
    fprintf(file, "\n");
  }

  return true;
}

#define MAX_STR_LEN 16
bool Matrix::Load(FILE* file)
{
  int i, j;
  char buf[MAX_STR_LEN];
  int rows, columns;

  if ( !file )
  {
    fprintf(stderr, "Matrix::Load - NULL file\n");
    return false;
  }

  fgets(buf, LABEL_LEN + 1, file);
  if ( strncmp(LABEL, buf, LABEL_LEN) )
  {
    fprintf(stderr, "Matrix::Load - Header string didn't match\n");
    return false;
  }

  if ( fscanf(file, "%d %d", &rows, &columns) != 2 )
  {
    fprintf(stderr, "Matrix::Load - Failed getting rows and columns\n");
    return false;
  }

  if ( !SetSize(rows, columns, false) )
  {
    fprintf(stderr, "Matrix::Load - Failed SetSize(%d, %d)\n", rows, columns);
    return false;
  }

  for (i = 0; i < mRows; i++)
  {
    for (j = 0; j < mColumns; j++)
    {
      if ( !fscanf(file, "%lf ", &mData[i * mColumns + j]) )
      {
        fprintf(stderr, "Matrix::Load - Failed getting data at (%d, %d)\n", i, j);
        return false;
      }
    }
  }

  return true;
}
