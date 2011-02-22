#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "Matrix.h"

#define IDENTITY_THRESH .001

#define NUM_ROWS_STR "NumRows"
#define NUM_COLS_STR "NumColumns"
#define NUMBER_STR "Number"
#define VALUE_STR "Value"

Matrix::Matrix()
{
  mData = NULL;
  mDataAlloc = 0;
  mRows = 0;
  mColumns = 0;
  mRowEchelonData = NULL;
  mRowEchelonDataAlloc = 0;
  mNumRowEchelonSwaps = 0;
}

Matrix::Matrix(const Matrix& refMatrix)
{
  mData = NULL;
  mDataAlloc = 0;
  mRows = 0;
  mColumns = 0;
  mRowEchelonData = NULL;
  mRowEchelonDataAlloc = 0;
  mNumRowEchelonSwaps = 0;
  *this = refMatrix;
}

Matrix::~Matrix()
{
  if ( mData )
    free(mData);
  if ( mRowEchelonData )
    free(mRowEchelonData);
}

bool Matrix::SetSize(int rows, int columns, bool clear)
{
  double *tmp;
  int cells;

  if ( (rows < 0) || (columns < 0) )
  {
    fprintf(stderr, "Matrix::SetSize - Invalid parameter\n");
    return false;
  }

  cells = rows * columns;
  if ( cells )
  {
    if ( cells > mDataAlloc )
    {
      tmp = (double *)realloc(mData, cells * sizeof(double));
      if ( !tmp )
      {
        fprintf(stderr, "Matrix::SetSize - Failed allocating mData\n");
        return false;
      }

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

void Matrix::Fill(double value)
{
  int i;

  for (i = 0; i < mCells; i++)
    mData[i] = value;
}

int Matrix::GetRows() const
{
  return mRows;
}

int Matrix::GetColumns() const
{
  return mColumns;
}

double Matrix::GetValue(int row, int column) const
{
  if ( (row >= mRows) || (row < 0) ||
       (column >= mColumns) || (column < 0) )
  {
    fprintf(stderr, "Matrix::GetValue - Invalid parameter\n");
    return 0;
  }

  return mData[row * mColumns + column];
}

bool Matrix::SetValue(int row, int column, double value)
{
  if ( (row >= mRows) || (row < 0) ||
       (column >= mColumns) || (column < 0) )
  {
    fprintf(stderr, "Matrix::SetValue - Invalid parameter\n");
    return false;
  }

  mData[row * mColumns + column] = value;

  return true;
}

bool Matrix::Set(double* values)
{
  if ( !values )
  {
    fprintf(stderr, "Matrix::Set - Invalid parameter\n");
    return false;
  }

  memcpy(mData, values, mCells * sizeof(double));

  return true;
}

bool Matrix::GetDeterminant(double &determinant)
{
  int row;
  double product;

  if ( (mRows != mColumns) || (mRows == 0) )
  {
    fprintf(stderr, "Matrix::GetDeterminant - Matrix is not square (%d rows, %d columns\n", mRows, mColumns);
    return false;
  }

  if ( mRows == 1 )
    determinant = mData[0];
  else if ( mRows == 2 )
    determinant = mData[0] * mData[3] - mData[1] * mData[2];
  else if ( mRows == 3 )
  {
    determinant = (mData[0] * mData[4] * mData[8] +
                   mData[1] * mData[5] * mData[6] +
                   mData[2] * mData[3] * mData[7]) -
                  (mData[6] * mData[4] * mData[2] +
                   mData[7] * mData[5] * mData[0] +
                   mData[8] * mData[3] * mData[1]);
  }
  else
  {
    if ( CalculateRowEchelonForm() )
    {
      product = mRowEchelonData[0];
      for (row = 1; row < mRows; row++)
        product *= mRowEchelonData[row * mColumns + row];
      determinant = product;
      if ( mNumRowEchelonSwaps % 2 )
        determinant *= -1;
    }
    else
      determinant = 0;
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
  {
    fprintf(stderr, "Matrix::SetAsInverse - Invalid parameter\n");
    return false;
  }

  if ( (m.mRows == 2) && (m.mColumns == 2) )
  {
    m.GetDeterminant(determinant);
    if ( determinant > 0 )
    {
      SetSize(2, 2, false);
      mData[0] = m.mData[3] / determinant;
      mData[1] = -1 * m.mData[1] / determinant;
      mData[2] = -1 * m.mData[2] / determinant;
      mData[3] = m.mData[0] / determinant;
    }
    else
    {
      fprintf(stderr, "Matrix::SetAsInverse - Determinant is non-positive\n");
      return false;
    }
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
  {
    fprintf(stderr, "Matrix::SetAsInverse - Row reduce failed\n");
    return false;
  }

  return true;
}

bool Matrix::CalculateRowEchelonForm()
{
  double *tmp;
  double pivotValue, swapVal, ratio;
  int row, refRow, column, pos1, pos2;

  if ( !mCells )
    return false;

  if ( mCells > mRowEchelonDataAlloc )
  {
    tmp = (double *)realloc(mRowEchelonData, mCells * sizeof(double));
    if ( !tmp )
    {
      fprintf(stderr, "Matrix::CalculateRowEchelonForm - Failed allocating mRowEchelonData\n");
      return false;
    }

    mRowEchelonData = tmp;
    mRowEchelonDataAlloc = mCells;
  }

  memcpy(mRowEchelonData, mData, mCells * sizeof(double));
  mNumRowEchelonSwaps = 0;

  for (row = 0; row < mRows; row++)
  {
    pivotValue = mRowEchelonData[row * mColumns + row];

    /* Need to record the number of row swaps since each time effectively multiplies
       the determinant by -1 */
    if ( pivotValue == 0 )
    {
      for (refRow = row + 1; (refRow < mRows) && (pivotValue == 0); refRow++)
      {
        if ( mRowEchelonData[refRow * mColumns + row] != 0 )
        {
          mNumRowEchelonSwaps++;
          pos1 = row * mColumns;
          pos2 = refRow * mColumns;
          for (column = 0; column < mColumns; column++, pos1++, pos2++)
          {
            swapVal = mRowEchelonData[pos1];
            mRowEchelonData[pos1] = mRowEchelonData[pos2];
            mRowEchelonData[pos2] = swapVal;
          }
          pivotValue = mRowEchelonData[row * mColumns + row];
        }
      }
      if ( pivotValue == 0 )
        return false;
    }

    for (refRow = row + 1; refRow < mRows; refRow++)
    {
      ratio = -mRowEchelonData[refRow * mColumns + row] / pivotValue;
      for (column = 0; column < mColumns; column++)
        mRowEchelonData[refRow * mColumns + column] += mRowEchelonData[row * mColumns + column] * ratio;
    }
  }

  return true;
}

bool Matrix::RowReduce()
{
  int currentRow, refRow, row, column;
  double pivotValue, ratio;
  bool identityFound = true;

  if ( mRows > mColumns )
  {
    fprintf(stderr, "Matrix::RowReduce - Matrix is too narrow\n");
    return false;
  }

  /* Form diagonal on left of matrix */
  for (currentRow = 0; currentRow < mRows; currentRow++)
  {
    pivotValue = mData[currentRow * mColumns + currentRow];
    if ( pivotValue == 0 )
    {
      fprintf(stderr, "Matrix::RowReduce - Pivot is zero\n");
      return false;
    }
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
    {
      fprintf(stderr, "Matrix::RowReduce - Pivot is zero\n");
      return false;
    }
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

Matrix& Matrix::operator-(const Matrix& m)
{
  static Matrix result;

  if ( !result.SetFromDifference(*this, m) )
    result.SetSize(0, 0);
  return result;
}

bool Matrix::SetFromDifference(const Matrix& a, const Matrix& b)
{
  int i;

  if ( (a.mRows != b.mRows) || (a.mColumns != b.mColumns) )
  {
    fprintf(stderr, "Matrix::SetFromDifference - Invalid paremeter\n");
    return false;
  }

  SetSize(a.mRows, a.mColumns, false);
  if ( mCells == 2 )
  {
    mData[0] = a.mData[0] - b.mData[0];
    mData[1] = a.mData[1] - b.mData[1];
  }
  else if ( mCells == 3 )
  {
    mData[0] = a.mData[0] - b.mData[0];
    mData[1] = a.mData[1] - b.mData[1];
    mData[2] = a.mData[2] - b.mData[2];
  }
  else
  {
    for (i = 0; i < mCells; i++)
      mData[i] =  a.mData[i] - b.mData[i];
  }

  return true;
}

Matrix& Matrix::operator*(const Matrix& m)
{
  static Matrix result;

  if ( !result.SetFromProduct(*this, m) )
    result.SetSize(0, 0);

  return result;
}

bool Matrix::SetFromProduct(const Matrix& a, const Matrix& b)
{
  int i;
  int row, column;
  int dest, aRowStart, bRowStart, bPos;

  if ( a.mColumns != b.mRows )
  {
    fprintf(stderr, "Matrix::SetFromProduct - Invalid paremeter\n");
    return false;
  }

  /* FIXME The unrolled checks are missing the cases for b.mRows > 3.  Check if we really want these unrolled loops, and if so, make them work correctly (don't trust the existing code). */
#if 0
  if ( a.mRows == 1 )
  {
    SetSize(a.mRows, b.mColumns, false);

    /* Manually unroll the loops for our common cases */
    if ( mColumns == 1 )
    {
      if ( b.mRows == 1 )
        mData[0] = a.mData[0] * b.mData[0];
      else if ( b.mRows == 2 )
        mData[0] = a.mData[0] * b.mData[0] + a.mData[1] * b.mData[1];
      else if ( b.mRows == 3 )
        mData[0] = a.mData[0] * b.mData[0] + a.mData[1] * b.mData[1] + a.mData[2] * b.mData[2];
    }
    else if ( mColumns == 2 )
    {
      if ( b.mRows == 1 )
      {
        mData[0] = a.mData[0] * b.mData[0];
        mData[1] = a.mData[0] * b.mData[1];
      }
      else if ( b.mRows == 2 )
      {
        mData[0] = a.mData[0] * b.mData[0] + a.mData[1] * b.mData[2];
        mData[1] = a.mData[0] * b.mData[1] + a.mData[1] * b.mData[3];
      }
      else if ( b.mRows == 3 )
      {
        mData[0] = a.mData[0] * b.mData[0] + a.mData[1] * b.mData[2] + a.mData[2] * b.mData[4];
        mData[1] = a.mData[0] * b.mData[1] + a.mData[1] * b.mData[3] + a.mData[2] * b.mData[5];
      }
    }
    else if ( mColumns == 3 )
    {
      if ( b.mRows == 1 )
      {
        mData[0] = a.mData[0] * b.mData[0];
        mData[1] = a.mData[0] * b.mData[1];
        mData[2] = a.mData[0] * b.mData[2];
      }
      else if ( b.mRows == 2 )
      {
        mData[0] = a.mData[0] * b.mData[0] + a.mData[1] * b.mData[3];
        mData[1] = a.mData[0] * b.mData[1] + a.mData[1] * b.mData[4];
        mData[2] = a.mData[0] * b.mData[2] + a.mData[1] * b.mData[5];
      }
      else if ( b.mRows == 3 )
      {
        mData[0] = a.mData[0] * b.mData[0] + a.mData[1] * b.mData[3] + a.mData[2] * b.mData[6];
        mData[1] = a.mData[0] * b.mData[1] + a.mData[1] * b.mData[4] + a.mData[2] * b.mData[7];
        mData[2] = a.mData[0] * b.mData[2] + a.mData[1] * b.mData[5] + a.mData[2] * b.mData[8];
      }
    }
    else
    {
      for (column = 0; column < mColumns; column++)
      {
        mData[column] = a.mData[0] * b.mData[column];
        bPos = column + b.mColumns;
        for (i = 1; i < b.mRows; i++, bPos += b.mColumns)
          mData[column] += a.mData[i] * b.mData[bPos];
      }
    }
  }
  else
#endif
  {
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
  }
  return true;
}

bool Matrix::Scale(Matrix& a)
{
  int i;

  if ( (a.mRows != mRows) || (a.mColumns != mColumns) )
  {
    fprintf(stderr, "Matrix::Scale - Invalid paremeter\n");
    return false;
  }

  for (i = 0; i < mCells; i++)
    mData[i] *= a.mData[i];
  return true;
}

bool Matrix::SetFromCellProducts(const Matrix& a, const Matrix& b)
{
  int i;

  if ( (a.mRows != b.mRows) || (a.mColumns != b.mColumns) )
  {
    fprintf(stderr, "Matrix::SetFromCellProducts - Invalid paremeter\n");
    return false;
  }

  SetSize(a.mRows, a.mColumns);
  for (i = 0; i < mCells; i++)
    mData[i] = a.mData[i] * b.mData[i];
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

Matrix& Matrix::operator=(const Matrix& m)
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

Matrix& Matrix::operator+=(const Matrix& m)
{
  int i;

  for (i = 0; i < mCells; i++)
    mData[i] += m.mData[i];

  return *this;
}

Matrix& Matrix::operator+=(double* values)
{
  int i;

  for (i = 0; i < mCells; i++)
    mData[i] += values[i];

  return *this;
}

Matrix& Matrix::operator-=(const Matrix& m)
{
  int i;

  for (i = 0; i < mCells; i++)
    mData[i] -= m.mData[i];

  return *this;
}

Matrix& Matrix::operator-=(double* values)
{
  int i;

  for (i = 0; i < mCells; i++)
    mData[i] -= values[i];

  return *this;
}

bool Matrix::operator==(const Matrix& m) const
{
  if ( (mRows != m.mRows) || (mColumns != m.mColumns) )
    return false;

  return !memcmp(mData, m.mData, mRows * mColumns * sizeof(double));
}

bool Matrix::operator!=(const Matrix& m) const
{
  return !(*this == m);
}

bool Matrix::operator<(const Matrix& m) const
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

bool Matrix::Print(FILE* file)
{
  int i, j;

  if ( !file )
  {
    fprintf(stderr, "Matrix::Save - Invalid paremeter\n");
    return false;
  }

  fprintf(file, "Matrix %d %d\n", mRows, mColumns);
  for (i = 0; i < mRows; i++)
  {
    for (j = 0; j < mColumns; j++)
      fprintf(file, "%.9f ", mData[i * mColumns + j]);
    fprintf(file, "\n");
  }

  return true;
}

bool Matrix::Save(const char* filename)
{
  FILE* file;
  xmlDocPtr document;
  xmlNodePtr rootNode;
  bool retCode = true;

  if ( !filename || !*filename )
  {
    fprintf(stderr, "Matrix::Save - Invalid parameter\n");
    return false;
  }

  file = fopen(filename, "w");
  if ( !file )
  {
    fprintf(stderr, "Matrix::Save - Failed opening %s\n", filename);
    return false;
  }

  document = xmlNewDoc(NULL);
  rootNode = xmlNewDocNode(document, NULL, (const xmlChar *)MATRIX_STR, NULL);
  xmlDocSetRootElement(document, rootNode);

  retCode = Save(rootNode);

  xmlDocFormatDump(file, document, 1);
  fclose(file);
  xmlFreeDoc(document);

  return retCode;
}

bool Matrix::Save(xmlNodePtr matrixNode)
{
  int i, j;
  xmlNodePtr numberNode;

  SetIntValue(matrixNode, NUM_ROWS_STR, mRows);
  SetIntValue(matrixNode, NUM_COLS_STR, mColumns);

  for (i = 0; i < mRows; i++)
  {
    for (j = 0; j < mColumns; j++)
    {
      numberNode = xmlNewNode(NULL, (const xmlChar*)NUMBER_STR);
      xmlAddChild(matrixNode, numberNode);
      SetDoubleValue(numberNode, VALUE_STR, mData[i * mColumns + j]);
    }
  }

  return true;
}

bool Matrix::Load(const char* filename)
{
  bool retCode = true;
  xmlDocPtr document;
  xmlNodePtr node;

  if ( !filename || !*filename )
  {
    fprintf(stderr, "Matrix::Load - Bad filename\n");
    return false;
  }

  document = xmlParseFile(filename);

  if ( !document )
  {
    fprintf(stderr, "Matrix::Load - Failed parsing %s\n", filename);
    return false;
  }

  node = xmlDocGetRootElement(document);
  if ( !node )
  {
    xmlFreeDoc(document);
    fprintf(stderr, "Matrix::Load - No root node in %s\n", filename);
    return false;
  }

  retCode = Load(node);

  xmlFreeDoc(document);

  return retCode;
}

bool Matrix::Load(xmlNodePtr matrixNode)
{
  int rows, cols, cellsFilled;
  xmlNodePtr node;
  Matrix matrix;
  string matrixID;
  bool retCode = true;

  rows = GetIntValue(matrixNode, NUM_ROWS_STR, 0);
  cols = GetIntValue(matrixNode, NUM_COLS_STR, 0);
  if ( (rows < 1) || (cols < 1) )
  {
    fprintf(stderr, "Matrix::Load - Invalid property\n");
    return false;
  }

  if ( !SetSize(rows, cols, false) )
  {
    fprintf(stderr, "Matrix::Load - Failed setting size %d %d\n", rows, cols);
    return false;
  }

  cellsFilled = 0;
  node = matrixNode->children;
  while ( node && retCode )
  {
    if ( !strcmp((char *)node->name, NUMBER_STR) )
    {
      if ( cellsFilled == mCells )
      {
        fprintf(stderr, "Matrix::Load - Found extra value\n");
        retCode = false;
      }
      else
      {
        mData[cellsFilled] = GetDoubleValue(node, VALUE_STR, 0);
        cellsFilled++;
      }
    }
    else if ( strcmp((char *)node->name, "text") )
    {
      fprintf(stderr, "Matrix::Load - Found unknown node %s\n", (char*)node->name);
      retCode = false;
    }
    node = node->next;
  }

  return retCode;
}
