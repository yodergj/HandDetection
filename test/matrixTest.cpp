#include "Matrix.h"

int main(int argc, char* argv[])
{
  Matrix a, b, c, d, e, f, g, h, i, j, k, l;
  double determinant;

  a.SetSize(2, 1);
  a.SetValue(0, 0, 1);
  a.SetValue(1, 0, 2);
  a.Save(stdout);

  b.SetSize(1, 2);
  b.SetValue(0, 0, 3);
  b.SetValue(0, 1, 4);
  b.Save(stdout);

  c = a * b;
  c.Save(stdout);

  d = c * 3.0;
  d.Save(stdout);

  e.SetSize(3, 3);
  e.SetValue(0, 0, 1);
  e.SetValue(0, 1, 2);
  e.SetValue(0, 2, 3);
  e.SetValue(1, 0, 4);
  e.SetValue(1, 1, 5);
  e.SetValue(1, 2, 6);
  e.SetValue(2, 0, 7);
  e.SetValue(2, 1, 8);
  e.SetValue(2, 2, 9);
  e.GetDeterminant(determinant);
  e.Save(stdout);
  printf("Determinant = %f\n", determinant);

  f.SetSize(3, 3);
  f.SetValue(0, 0, 9);
  f.SetValue(0, 1, 8);
  f.SetValue(0, 2, 7);
  f.SetValue(1, 0, 6);
  f.SetValue(1, 1, 5);
  f.SetValue(1, 2, 4);
  f.SetValue(2, 0, 3);
  f.SetValue(2, 1, 2);
  f.SetValue(2, 2, 1);
  f.Save(stdout);

  g = f - e;
  g.Save(stdout);

  h = g.Transpose();
  h.Save(stdout);
  
  i = a.Transpose();
  i.Save(stdout);

  printf("Orig\n");
  j.SetSize(2, 2);
  j.SetValue(0, 0, 5);
  j.SetValue(0, 1, 2);
  j.SetValue(1, 0, 1);
  j.SetValue(1, 1, 3);
  j.Save(stdout);

  printf("Inverse\n");
  k = j.Inverse();
  k.Save(stdout);

  l = j * k;
  l.Save(stdout);

  j *= 2;
  j.Save(stdout);

  return 0;
}
