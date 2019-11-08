


int ipow(int base, int exp)
{
  int result = 1;
  int scale = base;
  while (exp) {
    if (exp & 1) {
      result *= scale;
    }
    scale *= scale;
    exp /= 2;
  }
  return result;
}
