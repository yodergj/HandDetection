#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "Matrix.h"

#define IDENTITY_THRESH .001

Matrix::Matrix()
{
  mData = NULL;
  mDataAlloc = 0;
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
  int cells;

  if ( (rows < 0) || (columns < 0) )
    return false;

  cells = rows * columns;
  if ( cells )
  {
    if ( cells > mDataAlloc )
    {
      tmp = (double *)realloc(mData, cells * sizeof(double));
      if ( !tmp )
        return false;

      mData = tmp;
      mDataAlloc = cells;
    }

    if ( clear )
      memset(mData, 0, cells * sizeof(double));
  }
  mRows = rows;
  mColumns = columns;
  mCells = cells;

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
  static Matrix result;

  result.SetAsTranspose(*this);

  return result;
}

void Matrix::SetAsTranspose(Matrix& m)
{
  int i, j;

  SetSize(m.mColumns, m.mRows, false);
  if ( (mRows == 1) || (mColumns == 1) )
    memcpy(mData, m.mData, mRows * mColumns * sizeof(double));
  else
  {
    for (i = 0; i < mRows; i++)
      for (j = 0; j < mColumns; j++)
        mData[i * mColumns + j] =  m.mData[j * mColumns + i];
  }
}

Matrix& Matrix::Inverse()
{
  static Matrix result;

  if ( !result.SetAsInverse(*this) )
    result.SetSize(0, 0);

  return result;
}

bool Matrix::SetAsInverse(Matrix& m)
{
  double determinant;
  Matrix workMatrix;
  int row, column;

  if ( m.mRows != m.mColumns )
    return false;

  if ( (m.mRows == 2) && (m.mColumns == 2) )
  {
    m.GetDeterminant(determinant);
    if ( determinant > 0 )
    {
#if 0
      result.SetSize(2, 2, false);
      result.SetValue(0, 0, m.GetValue(1, 1));
      result.SetValue(0, 1, -1 * m.GetValue(0, 1));
      result.SetValue(1, 0, -1 * m.GetValue(1, 0));
      result.SetValue(1, 1, m.GetValue(0, 0));
      result = result * (1 / determinant);
#else
      SetSize(2, 2, false);
      mData[0] = m.mData[3] / determinant;
      mData[1] = -1 * m.mData[1] / determinant;
      mData[2] = -1 * m.mData[2] / determinant;
      mData[3] = m.mData[0] / determinant;
#endif
    }
    else
      return false;
    return true;
  }

  workMatrix.SetSize(m.mRows, m.mColumns * 2, false);
  for (row = 0; row < m.mRows; row++)
  {
    for (column = 0; column < m.mColumns; column++)
    {
      workMatrix.mData[row * m.mColumns * 2 + column] = m.mData[row * m.mColumns + column];
      if ( row == column )
        workMatrix.mData[row * m.mColumns * 2 + m.mColumns + column] = 1;
      else
        workMatrix.mData[row * m.mColumns * 2 + m.mColumns + column] = 0;
    }
  }

  if ( workMatrix.RowReduce() )
  {
    SetSize(m.mRows, m.mColumns, false);
    for (row = 0; row < mRows; row++)
      for (column = 0; column < mColumns; column++)
        mData[row * mColumns + column] = workMatrix.mData[row * mColumns * 2 + mColumns + column];
  }
  else
    return false;

  return true;
}

bool Matrix::RowReduce()
{
  int currentRow, refRow, row, column;
  double pivotValue, ratio;
  bool identityFound = true;

  if ( mRows > mColumns )
  {
    fprintf(stderr, "RowReduce - Matrix is too narrow\n");
    return false;
  }

  /* Form diagonal on left of matrix */
  for (currentRow = 0; currentRow < mRows; currentRow++)
  {
    pivotValue = mData[currentRow * mColumns + currentRow];
    if ( pivotValue == 0 )
      return false;
    for (refRow = 0; refRow < mRows; refRow++)
    {
      if ( refRow == currentRow )
        continue;
      ratio = -mData[refRow * mColumns + currentRow] / pivotValue;
      for (column = 0; column < mColumns; column++)
        mData[refRow * mColumns + column] += mData[currentRow * mColumns + column] * ratio;
    }
  }

  /* Reduce the diagonal to an identity */
  for (currentRow = 0; currentRow < mRows; currentRow++)
  {
    pivotValue = mData[currentRow * mColumns + currentRow];
    if ( pivotValue == 0 )
      return false;
    for (column = 0; column < mColumns; column++)
      mData[currentRow * mColumns + column] /= pivotValue;
  }

  /* Check if we've sufficiently approximated an identity matrix on the left side */
  for (row = 0; (row < mRows) && identityFound; row++)
  {
    for (column = 0; (column < mRows) && identityFound; column++)
    {
      if ( row == column )
      {
        if ( fabs(1 - mData[row * mColumns + column]) > IDENTITY_THRESH )
          identityFound = false;
      }
      else
      {
        if ( fabs(mData[row * mColumns + column]) > IDENTITY_THRESH )
          identityFound = false;
      }
    }
  }

  return identityFound;
}

Matrix& Matrix::operator-(Matrix& m)
{
  static Matrix result;

  if ( !result.SetFromDifference(*this, m) )
    result.SetSize(0, 0);
  return result;
}

bool Matrix::SetFromDifference(Matrix& a, Matrix& b)
{
  int i;

  if ( (a.mRows != b.mRows) || (a.mColumns != b.mColumns) )
    return false;

  SetSize(a.mRows, a.mColumns, false);
  for (i = 0; i < mCells; i++)
    mData[i] =  a.mData[i] - b.mData[i];

  return true;
}

Matrix& Matrix::operator*(Matrix& m)
{
  int i;
  int row, column;
  static Matrix result;
  int dest, aRowStart, bRowStart;

  if ( mColumns != m.mRows )
  {
    result.SetSize(0, 0);
    return result;
  }

  result.SetSize(mRows, m.mColumns);
  dest = 0;
  aRowStart = 0;
  for (row = 0; row < result.mRows; row++, aRowStart += mColumns)
    for (column = 0; column < result.mColumns; column++, dest++)
    {
      bRowStart = 0;
      for (i = 0; i < mColumns; i++, bRowStart += m.mColumns)
        result.mData[dest] += mData[aRowStart + i] * m.mData[bRowStart + column];
    }
  return result;
}

bool Matrix::SetFromProduct(Matrix& a, Matrix& b)
{
  int i;
  int row, column;
  int dest, aRowStart, bRowStart;

  if ( a.mColumns != b.mRows )
    return false;

  SetSize(a.mRows, b.mColumns);
  dest = 0;
  aRowStart = 0;
  for (row = 0; row < mRows; row++, aRowStart += a.mColumns)
    for (column = 0; column < mColumns; column++, dest++)
    {
      bRowStart = 0;
      for (i = 0; i < a.mColumns; i++, bRowStart += b.mColumns)
        mData[dest] += a.mData[aRowStart + i] * b.mData[bRowStart + column];
    }
  return true;
}

Matrix& Matrix::operator*(double d)
{
  int i;
  static Matrix result;

  result.SetSize(mRows, mColumns, false);
  for (i = 0; i < mCells; i++)
    result.mData[i] = d * mData[i];
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
  int i;

  for (i = 0; i < mCells; i++)
    mData[i] *= d;
  return *this;
}

Matrix& Matrix::operator+=(Matrix& m)
{
  int i;

  for (i = 0; i < mCells; i++)
    mData[i] += m.mData[i];

  return *this;
}

Matrix& Matrix::operator-=(Matrix& m)
{
  int i;

  for (i = 0; i < mCells; i++)
    mData[i] -= m.mData[i];

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
