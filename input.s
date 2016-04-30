val CONSTANT = 10;

var mutable = CONSTANT;

def main()
{
  test();
  printPlus10(CONSTANT);
  printPlus10(double(15));
  compare(10, double(10));
  println(mutable);
  mutable = mutable + 5;
  println(mutable);
}

def test()
{
  val test = 10;
  val test2 = test;
  var abc;
  abc = test2 + test;
  println(abc);
}

def compare(first, second)
{
  return first < second;
}

def double(input)
{
  return input * 2;
}

def printPlus10(single)
{
  val test = 10 + single;
  println(test);
}
